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

typedef struct{
    
} gameState;

struct timespec ts = {0, SPEED_MULTIPLIER * 100 * 1000 * 1000};

int updateSnakeLayer(snake_state* ss , char snake_layer[SCREEN_HEIGHT][SCREEN_WIDTH], char food_layer[SCREEN_HEIGHT][SCREEN_WIDTH], char wall_layer[SCREEN_HEIGHT][SCREEN_WIDTH], int dir){
    snake_layer[ss->deleted->y][ss->deleted->x]--;
    compute_snake(ss, dir);
    snake* new_head = ss->end;
    if(wall_layer[new_head->y][new_head->x] == 1){
        return -1;
    }
    snake_layer[new_head->y][new_head->x]++;
    if(food_layer[new_head->y][new_head->x] > 0){
        food_layer[new_head->y][new_head->x] = 0;
        snake_layer[ss->deleted->y][ss->deleted->x]++;
        snake* new_end = (snake*)malloc(sizeof(snake));
        new_end->x = ss->deleted->x;
        new_end->y = ss->deleted->y;
        new_end->next = ss->head;
        ss->head = new_end;
        food food;
        do{
            food.x = rand() % SCREEN_WIDTH;
            food.y = rand() % SCREEN_HEIGHT;
        }while (wall_layer[food.y][food.x] == 1 || snake_layer[food.y][food.x] > 0 || food_layer[food.y][food.x] == 1); 
        food_layer[food.y][food.x] = 1;
        return 1;
    }
    return 0;
}

int main() {
    enableRawMode();
    pthread_t t;
    pthread_create(&t, NULL, loop, NULL);
    char pov[POV_HEIGHT][POV_WIDTH];
    char wall_layer[SCREEN_HEIGHT][SCREEN_WIDTH];
    char food_layer[SCREEN_HEIGHT][SCREEN_WIDTH];
    char snake_layer[SCREEN_HEIGHT][SCREEN_WIDTH];

    memset(pov, -1, sizeof(pov));
    memset(wall_layer, 0, sizeof(wall_layer));
    memset(food_layer, 0, sizeof(food_layer));
    memset(snake_layer, 0, sizeof(snake_layer));
    /*setup the boundy */
    for(int i = 0; i < SCREEN_HEIGHT; i++){
        wall_layer[i][0] = 1;
        wall_layer[i][SCREEN_WIDTH - 1] = 1;
    }
    for(int i = 0; i < SCREEN_WIDTH; i++){
        wall_layer[0][i] = 1;
        wall_layer[SCREEN_HEIGHT - 1][i] = 1;
    }
    /*setup the snake */
    struct snake s = {10, 10, NULL};
    s.next = &(struct snake){10, 9, NULL};
    s.next->next = NULL;
    snake deleted = {-1,-1,NULL};

    snake_state ss = {&s,s.next,&deleted,'w'};

    snake* temp = ss.head;
    while(temp){
        snake_layer[temp->y][temp->x] = 1;
        temp = temp->next;
    }
    /*setup the food */
    food food;
    for(int i =0; i<50; i++){    
        while(1){
            food.x = rand() % SCREEN_WIDTH;
            food.y = rand() % SCREEN_HEIGHT;
            if(wall_layer[food.y][food.x] == 0 && snake_layer[food.y][food.x] == 0 && food_layer[food.y][food.x] == 0){
                break;
            }
        }
        food_layer[food.y][food.x] = 1;
    }

    int score = 0;
    /*game loop */
    while(1){
        nanosleep(&ts, NULL);
        int res = updateSnakeLayer(&ss, snake_layer, food_layer, wall_layer, dir);
        if(res == -1){
            break;
        }
        if(res == 1){
            score++;
        }
        snake* new_head = ss.end;
        char screen[SCREEN_HEIGHT][SCREEN_WIDTH];
        compose_layers(SCREEN_HEIGHT, SCREEN_WIDTH, screen, wall_layer, food_layer, snake_layer);
        get_pov(screen, pov, new_head->x, new_head->y);
        
        render(POV_HEIGHT, POV_WIDTH, pov);
    }
    printf("Game Over :%d\n", score);
    // pthread_join(t, NULL);
  return 0;
}