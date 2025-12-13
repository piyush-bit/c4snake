#include "../utils/d_array.h"
#include "snake.h"
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Vision radius (squared comparisons used internally)
#ifndef VISION_RADIUS
#define VISION_RADIUS 100
#endif

static bool buffers_initialized = false;
static unsigned char *g_visited = NULL; // Used to track visited nodes
static int *g_prev = NULL;              // Used to reconstruct paths
static int *g_queue = NULL;             // Queue for BFS
static int g_buffer_size = 0;

// Initialize buffers if they haven't been already or if size changed.
static void ensure_buffers(int total_cells) {
  if (!buffers_initialized || g_buffer_size < total_cells) {
    if (buffers_initialized) {
      free(g_visited);
      free(g_prev);
      free(g_queue);
    }
    g_visited = (unsigned char *)malloc(total_cells * sizeof(unsigned char));
    g_prev = (int *)malloc(total_cells * sizeof(int));
    g_queue = (int *)malloc(total_cells * sizeof(int));
    g_buffer_size = total_cells;
    buffers_initialized = true;
  }
}

// --- Helper Functions ---

static inline bool is_valid(int x, int y, int w, int h) {
  return x >= 0 && x < w && y >= 0 && y < h;
}

static inline int to_index(int x, int y, int w) { return y * w + x; }

static inline void from_index(int index, int *x, int *y, int w) {
  *x = index % w;
  *y = index / w;
}

// Euclidean within-radius check (avoid sqrt by comparing squares)
static inline bool within_radius(int cx, int cy, int x, int y, int radius) {
  int dx = x - cx;
  int dy = y - cy;
  return (dx * dx + dy * dy) <= (radius * radius);
}

// Get delta (dx, dy) from a direction character
static void get_direction_delta(char dir, int *dx, int *dy) {
  *dx = 0;
  *dy = 0;
  switch (dir) {
  case 'w':
    *dy = -1;
    break;
  case 's':
    *dy = 1;
    break;
  case 'a':
    *dx = -1;
    break;
  case 'd':
    *dx = 1;
    break;
  }
}

// --- Algorithms ---

/**
 * BFS to find the distance to the nearest food, but only searching cells within
 * vision_radius centered at (vision_cx, vision_cy). The BFS start node is
 * (start_x, start_y) (typically the candidate cell).
 *
 * If out_first_step != NULL, it is set to the first move from the start to the food.
 *
 * Returns: distance in steps, or -1 if no food visible within the radius.
 */
static int find_nearest_food_with_vision(DArray *screen, int start_x, int start_y,
                                        char *out_first_step, int vision_cx,
                                        int vision_cy, int vision_radius) {
  int w = screen->col;
  int h = screen->row;
  int total_cells = w * h;
  ensure_buffers(total_cells);

  // Reset visited (we only need visited; g_prev entries are set when nodes visited)
  memset(g_visited, 0, total_cells * sizeof(unsigned char));

  int head = 0;
  int tail = 0;
  int start_idx = to_index(start_x, start_y, w);

  // If the start cell itself is outside vision, we cannot search from here.
  if (!within_radius(vision_cx, vision_cy, start_x, start_y, vision_radius))
    return -1;

  g_queue[tail++] = start_idx;
  g_visited[start_idx] = 1;
  g_prev[start_idx] = -1;

  int found_idx = -1;

  // Directions: Up, Down, Left, Right
  const int dx[] = {0, 0, -1, 1};
  const int dy[] = {-1, 1, 0, 0};

  while (head < tail) {
    int current_idx = g_queue[head++];
    int cx, cy;
    from_index(current_idx, &cx, &cy, w);

    // Check for food (represented by 2)
    if (*d_array_get(screen, cx, cy) == 2) {
      found_idx = current_idx;
      break;
    }

    for (int i = 0; i < 4; ++i) {
      int nx = cx + dx[i];
      int ny = cy + dy[i];

      if (!is_valid(nx, ny, w, h))
        continue;

      // Do not explore cells outside vision circle (they are unknown/unseen, not walls)
      if (!within_radius(vision_cx, vision_cy, nx, ny, vision_radius))
        continue;

      int neighbor_idx = to_index(nx, ny, w);

      // Skip if visited or obstacle (1 is wall/snake)
      if (g_visited[neighbor_idx] || *d_array_get(screen, nx, ny) == 1)
        continue;

      g_visited[neighbor_idx] = 1;
      g_prev[neighbor_idx] = current_idx;
      g_queue[tail++] = neighbor_idx;
    }
  }

  if (found_idx == -1)
    return -1; // No food found within vision

  // Reconstruct path to find distance and first step
  int curr = found_idx;
  int distance = 0;
  int previous_node = -1;

  while (curr != start_idx) {
    previous_node = curr;
    curr = g_prev[curr];
    distance++;
    if (curr == -1) { // defensive check
      return -1;
    }
  }

  // Determine the first step direction
  if (out_first_step && previous_node != -1) {
    int px, py;
    from_index(previous_node, &px, &py, w);

    if (px == start_x && py == start_y - 1)
      *out_first_step = 'w';
    else if (px == start_x && py == start_y + 1)
      *out_first_step = 's';
    else if (px == start_x - 1 && py == start_y)
      *out_first_step = 'a';
    else if (px == start_x + 1 && py == start_y)
      *out_first_step = 'd';
    else
      *out_first_step = 0; // Should not happen for adjacent nodes
  }

  return distance;
}

/**
 * Flood fill to count the size of the reachable area **within vision** centered
 * at (vision_cx, vision_cy). The flood-fill start node is (start_x, start_y)
 * (typically a candidate cell).
 *
 * Returns number of reachable cells inside the vision circle.
 */
static int calculate_reachable_area_with_vision(DArray *screen, int start_x,
                                                int start_y, int vision_cx,
                                                int vision_cy, int vision_radius) {
  int w = screen->col;
  int h = screen->row;

  if (!is_valid(start_x, start_y, w, h) ||
      *d_array_get(screen, start_x, start_y) == 1)
    return 0;

  // If start is outside vision, no reachable area is visible from head.
  if (!within_radius(vision_cx, vision_cy, start_x, start_y, vision_radius))
    return 0;

  int total_cells = w * h;
  ensure_buffers(total_cells);
  memset(g_visited, 0, total_cells * sizeof(unsigned char));

  int head = 0;
  int tail = 0;
  int start_idx = to_index(start_x, start_y, w);

  g_queue[tail++] = start_idx;
  g_visited[start_idx] = 1;

  int count = 0;
  const int dx[] = {0, 0, -1, 1};
  const int dy[] = {-1, 1, 0, 0};

  while (head < tail) {
    int current_idx = g_queue[head++];
    count++;

    int cx, cy;
    from_index(current_idx, &cx, &cy, w);

    for (int i = 0; i < 4; i++) {
      int nx = cx + dx[i];
      int ny = cy + dy[i];

      if (!is_valid(nx, ny, w, h))
        continue;

      // Skip exploring cells outside the vision circle
      if (!within_radius(vision_cx, vision_cy, nx, ny, vision_radius))
        continue;

      int neighbor_idx = to_index(nx, ny, w);
      if (g_visited[neighbor_idx] || *d_array_get(screen, nx, ny) == 1)
        continue;

      g_visited[neighbor_idx] = 1;
      g_queue[tail++] = neighbor_idx;
    }
  }

  return count;
}

// --- Main Decision Logic ---

char decideMove(snake_state *ss, DArray *screen) {
  int head_x = ss->end->x; // 'end' is the logical head in this codebase
  int head_y = ss->end->y;
  char current_dir = ss->dir;

  // Vision center is the head. Use the configured radius.
  const int vision_radius = VISION_RADIUS;
  const int vision_cx = head_x;
  const int vision_cy = head_y;

  // Define relative directions
  char forward = current_dir;
  char left, right;

  if (current_dir == 'w') {
    left = 'a';
    right = 'd';
  } else if (current_dir == 's') {
    left = 'd';
    right = 'a';
  } else if (current_dir == 'a') {
    left = 's';
    right = 'w';
  } else {
    left = 'w';
    right = 's';
  }

  char candidates[] = {forward, left, right};

  // Evaluation metrics for each candidate
  bool is_safe[3];
  int reachable_area[3] = {0, 0, 0};
  int distance_to_food[3] = {-1, -1, -1};
  int candidate_x[3], candidate_y[3];
  int w = screen->col;
  int h = screen->row;

  // 1. Analyze each candidate move
  for (int i = 0; i < 3; i++) {
    int dx, dy;
    get_direction_delta(candidates[i], &dx, &dy);

    candidate_x[i] = head_x + dx;
    candidate_y[i] = head_y + dy;

    // Check immediate collision
    is_safe[i] = is_valid(candidate_x[i], candidate_y[i], w, h) &&
                 (*d_array_get(screen, candidate_x[i], candidate_y[i]) != 1);

    // Also ensure candidate cell is within vision (if it's outside, treat as not visible)
    if (is_safe[i] &&
        !within_radius(vision_cx, vision_cy, candidate_x[i], candidate_y[i],
                       vision_radius)) {
      // Candidate is safe wrt immediate map but outside vision -> treat as unknown
      // For safety, allow it, but reachable area will be zero because we cannot see beyond vision.
      // is_safe remains true (so the snake can still move there), but searches will return 0/-1.
    }

    if (is_safe[i]) {
      // Calculate metrics limited by vision centered on head
      reachable_area[i] = calculate_reachable_area_with_vision(
          screen, candidate_x[i], candidate_y[i], vision_cx, vision_cy,
          vision_radius);
      distance_to_food[i] = find_nearest_food_with_vision(
          screen, candidate_x[i], candidate_y[i], NULL, vision_cx, vision_cy,
          vision_radius);
    } else {
      reachable_area[i] = 0;
      distance_to_food[i] = -1;
    }
  }

  long best_score = LONG_MIN;
  int best_index = -1;

  for (int i = 0; i < 3; i++) {
    if (!is_safe[i])
      continue;

    // Basic score: prefer larger reachable area (safety), and smaller visible distance to food.
    long score = (long)reachable_area[i] * 100L;

    if (distance_to_food[i] != -1) {
      score -= distance_to_food[i]; // closer food is better
    } else {
      // If no food is visible in the radius, use a moderate penalty rather than an extreme one.
      // This avoids pathological behavior when vision is limited.
      const long NO_FOOD_PENALTY = (long)vision_radius * 50L;
      score -= NO_FOOD_PENALTY;
    }

    // Tie-breaking: prefer Forward > Left > Right implicitly by candidate ordering, but
    // ensure stable choice: if equal score, prefer the lower i (forward first).
    if (score > best_score) {
      best_score = score;
      best_index = i;
    }
  }

  // 3. Return best move
  if (best_index != -1) {
    return candidates[best_index];
  }

  // Fallback: If trapped, try any valid move
  for (int i = 0; i < 3; i++) {
    if (is_safe[i])
      return candidates[i];
  }

  // Last resort: go forward regardless
  return forward;
}