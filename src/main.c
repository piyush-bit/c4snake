#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "config.h"
#include "logic/snake.h"
#include "platform/input.h"
#include "platform/terminal.h"
#include "view/render.h"

typedef struct{
    int x;
    int y;
} food;

struct timespec ts = {0, 0.5 * 100 * 1000 * 1000};

int main() {
    enableRawMode();
    pthread_t t;
    pthread_create(&t, NULL, loop, NULL);
    char screen[SCREEN_HEIGHT][SCREEN_WIDTH];
    char pov[POV_HEIGHT][POV_WIDTH];
    memset(screen, 0, sizeof(screen));
    memset(pov, -1, sizeof(pov));
    for(int i = 0; i < SCREEN_HEIGHT; i++){
        screen[i][0] = 1;
        screen[i][SCREEN_WIDTH - 1] = 1;
    }
    for(int i = 0; i < SCREEN_WIDTH; i++){
        screen[0][i] = 1;
        screen[SCREEN_HEIGHT - 1][i] = 1;
    }
    struct snake s = {10, 10, NULL};
    s.next = &(struct snake){10, 9, NULL};
    s.next->next = NULL;

    snake_state ss = {&s,s.next,'w'};

    snake* temp = ss.head;
    while(temp){
        screen[temp->y][temp->x] = 1;
        temp = temp->next;
    }
    food food;
    while(1){
        food.x = rand() % SCREEN_WIDTH;
        food.y = rand() % SCREEN_HEIGHT;
        if(screen[food.y][food.x] == 0){
            break;
        }
    }
    screen[food.y][food.x] = 2;
    int score = 0;
    while(1){
        nanosleep(&ts, NULL);
        snake* end = ss.head;
        screen[end->y][end->x] = 0;
        compute_snake(&ss, dir);
        snake* new_head = ss.end;
        screen[new_head->y][new_head->x] = 1;
        if(new_head->x < 0 || new_head->x >= SCREEN_WIDTH || new_head->y < 0 || new_head->y >= SCREEN_HEIGHT){
            break;
        }
        get_pov(screen, pov, new_head->x, new_head->y);
        if(new_head->x == food.x && new_head->y == food.y){
            score++;
            screen[food.y][food.x] = 1;
            screen[end->y][end->x] = 1;
            snake* new_end = (snake*)malloc(sizeof(snake));
            new_end->x = end->x;
            new_end->y = end->y;
            new_end->next = ss.head;
            ss.head = new_end;
            while (screen[food.y][food.x] == 1) {
                food.x = rand() % SCREEN_WIDTH;
                food.y = rand() % SCREEN_HEIGHT;
            }
            screen[food.y][food.x] = 2;
        }
        
        render(POV_HEIGHT, POV_WIDTH, pov);
    }
    printf("Game Over :%d\n", score);
    // pthread_join(t, NULL);
  return 0;
}