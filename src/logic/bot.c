#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../config.h"
#include "snake.h"

// helper to check bounds
static inline int in_bounds(int x, int y) {
    return x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT;
}

// map a direction char to dx,dy
static void dir_delta(char d, int *dx, int *dy) {
    *dx = 0; *dy = 0;
    if (d == 'w') *dy = -1;
    else if (d == 's') *dy = 1;
    else if (d == 'a') *dx = -1;
    else if (d == 'd') *dx = 1;
}

// flatten index
static inline int idx(int x, int y) { return y * SCREEN_WIDTH + x; }

// BFS to find distance to nearest food from (sx,sy).
// If a path exists, returns distance and optionally fills first_step (first direction char from (sx,sy) -> next cell).
// If no food found, returns -1.
static int bfs_nearest_food_first_step(char screen[SCREEN_HEIGHT][SCREEN_WIDTH],
                                       int sx, int sy,
                                       char *first_step /*out, nullable*/) {
    int total = SCREEN_WIDTH * SCREEN_HEIGHT;
    unsigned char *vis = malloc(total);
    int *prev = malloc(sizeof(int) * total);
    if (!vis || !prev) { free(vis); free(prev); return -1; }
    memset(vis, 0, total);
    for (int i = 0; i < total; ++i) prev[i] = -1;

    int *q = malloc(sizeof(int) * total);
    int qh = 0, qt = 0;
    q[qt++] = idx(sx, sy);
    vis[idx(sx, sy)] = 1;

    int found_idx = -1;
    while (qh < qt) {
        int cur = q[qh++];
        int cx = cur % SCREEN_WIDTH;
        int cy = cur / SCREEN_WIDTH;
        if (screen[cy][cx] == 2) { found_idx = cur; break; }
        // neighbors: w,s,a,d
        const int dxs[4] = {0, 0, -1, 1};
        const int dys[4] = {-1, 1, 0, 0};
        for (int k = 0; k < 4; ++k) {
            int nx = cx + dxs[k];
            int ny = cy + dys[k];
            if (!in_bounds(nx, ny)) continue;
            int nidx = idx(nx, ny);
            if (vis[nidx]) continue;
            if (screen[ny][nx] == 1) continue; // wall
            vis[nidx] = 1;
            prev[nidx] = cur;
            q[qt++] = nidx;
        }
    }

    int dist = -1;
    if (found_idx != -1) {
        // backtrack to find first step from start
        int cur = found_idx;
        int steps = 0;
        while (prev[cur] != -1 && prev[cur] != idx(sx, sy)) {
            cur = prev[cur];
            steps++;
        }
        // cur is now either start (if food is at start) or the immediate neighbor we must step into
        if (first_step) {
            int fx = cur % SCREEN_WIDTH;
            int fy = cur / SCREEN_WIDTH;
            if (fx == sx && fy == sy) {
                // food is on head
                *first_step = 0;
            } else if (fx == sx && fy == sy - 1) *first_step = 'w';
            else if (fx == sx && fy == sy + 1) *first_step = 's';
            else if (fx == sx - 1 && fy == sy) *first_step = 'a';
            else if (fx == sx + 1 && fy == sy) *first_step = 'd';
            else *first_step = 0;
        }
        // compute distance by following prev from found_idx to start
        int cur2 = found_idx;
        int distcount = 0;
        while (cur2 != idx(sx, sy)) { cur2 = prev[cur2]; distcount++; }
        dist = distcount;
    }

    free(vis); free(prev); free(q);
    return dist;
}

// compute size of reachable area (cells not equal to 1) from start (sx,sy)
static int reachable_area_size(char screen[SCREEN_HEIGHT][SCREEN_WIDTH], int sx, int sy) {
    if (!in_bounds(sx, sy)) return 0;
    if (screen[sy][sx] == 1) return 0;

    int total = SCREEN_WIDTH * SCREEN_HEIGHT;
    unsigned char *vis = malloc(total);
    if (!vis) return 0;
    memset(vis, 0, total);

    int *q = malloc(sizeof(int) * total);
    int qh = 0, qt = 0;
    q[qt++] = idx(sx, sy);
    vis[idx(sx, sy)] = 1;
    int count = 0;

    while (qh < qt) {
        int cur = q[qh++];
        int cx = cur % SCREEN_WIDTH;
        int cy = cur / SCREEN_WIDTH;
        count++;
        const int dxs[4] = {0, 0, -1, 1};
        const int dys[4] = {-1, 1, 0, 0};
        for (int k = 0; k < 4; ++k) {
            int nx = cx + dxs[k];
            int ny = cy + dys[k];
            if (!in_bounds(nx, ny)) continue;
            int nidx = idx(nx, ny);
            if (vis[nidx]) continue;
            if (screen[ny][nx] == 1) continue; // wall
            vis[nidx] = 1;
            q[qt++] = nidx;
        }
    }

    free(vis); free(q);
    return count;
}

char decideMove(snake_state* ss, char screen[SCREEN_HEIGHT][SCREEN_WIDTH]) {
    int x = ss->end->x;
    int y = ss->end->y;
    char current_dir = ss->dir;

    // compute forward/left/right relative directions
    char forward = current_dir;
    char left, right;
    if (current_dir == 'w') { left = 'a'; right = 'd'; }
    else if (current_dir == 's') { left = 'd'; right = 'a'; }
    else if (current_dir == 'a') { left = 's'; right = 'w'; }
    else { left = 'w'; right = 's'; }

    // candidate set: forward, left, right
    char candidates[3] = { forward, left, right };
    int cand_x[3], cand_y[3], cand_free[3];

    for (int i = 0; i < 3; ++i) {
        int dx = 0, dy = 0;
        dir_delta(candidates[i], &dx, &dy);
        cand_x[i] = x + dx;
        cand_y[i] = y + dy;
        cand_free[i] = in_bounds(cand_x[i], cand_y[i]) && (screen[cand_y[i]][cand_x[i]] != 1);
    }

    // For each free candidate compute reachable area and distance-to-food (if any)
    int areas[3] = {0,0,0};
    int dist_to_food[3] = {-1,-1,-1};
    for (int i = 0; i < 3; ++i) {
        if (!cand_free[i]) continue;
        areas[i] = reachable_area_size(screen, cand_x[i], cand_y[i]);
        // compute distance to nearest food from candidate cell
        char dummy_first = 0;
        dist_to_food[i] = bfs_nearest_food_first_step(screen, cand_x[i], cand_y[i], &dummy_first);
    }

    // scoring: prefer larger reachable area strongly; prefer closer food but not at cost of tiny area.
    // score = area * 100 - (dist_to_food>=0 ? dist_to_food : 10000)
    long best_score = LONG_MIN;
    int best_i = -1;
    for (int i = 0; i < 3; ++i) {
        if (!cand_free[i]) continue;
        long score = (long)areas[i] * 100L;
        if (dist_to_food[i] >= 0) score -= dist_to_food[i]; else score -= 10000;
        // small tie-breaker preferences: prefer forward, then left, then right for stable movement
        if (score == best_score) {
            if (best_i == -1) best_i = i;
            else {
                // order preference: forward(0), left(1), right(2)
                if (i < best_i) best_i = i;
            }
        } else if (score > best_score) {
            best_score = score;
            best_i = i;
        }
    }

    if (best_i != -1) {
        return candidates[best_i];
    }

    // If no candidate free (boxed in), attempt to return any non-wall in bounds (defensive)
    for (int i = 0; i < 3; ++i) if (cand_free[i]) return candidates[i];

    // otherwise just try forward (may be wall but we have no choice)
    return forward;
}
