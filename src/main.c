#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <time.h>
#include "config.h"
#include "logic/snake.h"
#include "platform/input.h"
#include "platform/terminal.h"
#include "view/render.h"
#include "logic/bot.h"

typedef struct{
    int x;
    int y;
} food;

typedef struct{
    
} gameState;

struct timespec ts = {0, SPEED_MULTIPLIER * 100 * 1000 * 1000};

void clearSnakeLayer(snake_state* ss , char snake_layer[SCREEN_HEIGHT][SCREEN_WIDTH]){
    snake* temp = ss->head;
    while(temp){
        snake_layer[temp->y][temp->x] = 0;
        temp = temp->next;
    }
}

int updateSnakeLayer(snake_state* ss , char food_layer[SCREEN_HEIGHT][SCREEN_WIDTH], char wall_layer[SCREEN_HEIGHT][SCREEN_WIDTH], int dir){
    compute_snake(ss, dir);
    snake* new_head = ss->end;
    if(wall_layer[new_head->y][new_head->x] == 1){
        return -1;
    }
    (*getcoordinatesPointer(ss, new_head->x, new_head->y))++;
    if(food_layer[new_head->y][new_head->x] > 0){
        food_layer[new_head->y][new_head->x] ++;
        ss->length++;
        snake* new_end = (snake*)malloc(sizeof(snake));
        new_end->x = ss->deleted->x;
        new_end->y = ss->deleted->y;
        new_end->next = ss->head;
        ss->head = new_end;
        return 1;
    }
    (*getcoordinatesPointer(ss, ss->deleted->x, ss->deleted->y))--;
    return 0;
}

char screen[SCREEN_HEIGHT][SCREEN_WIDTH];
int main() {
    enableRawMode();
    pthread_t t;
    pthread_create(&t, NULL, loop, NULL);
    char pov[POV_HEIGHT][POV_WIDTH];
    char wall_layer[SCREEN_HEIGHT][SCREEN_WIDTH];
    char food_layer[SCREEN_HEIGHT][SCREEN_WIDTH];

    memset(pov, -1, sizeof(pov));
    memset(wall_layer, 0, sizeof(wall_layer));
    memset(food_layer, 0, sizeof(food_layer));
    /*setup the boundy */
    for(int i = 0; i < SCREEN_HEIGHT; i++){
        wall_layer[i][0] = 1;
        wall_layer[i][SCREEN_WIDTH - 1] = 1;
    }
    for(int i = 0; i < SCREEN_WIDTH; i++){
        wall_layer[0][i] = 1;
        wall_layer[SCREEN_HEIGHT - 1][i] = 1;
    }

    /*setup the food */
    food food;
    for(int i =0; i<20; i++){    
        while(1){
            food.x = rand() % SCREEN_WIDTH;
            food.y = rand() % SCREEN_HEIGHT;
            if(wall_layer[food.y][food.x] == 0 && food_layer[food.y][food.x] == 0){
                break;
            }
        }
        food_layer[food.y][food.x] = 1;
    }

    /*setup the snake */
    
    snake_state** states = (snake_state**)malloc(sizeof(snake_state*)*(BOTCOUNT+1));
    for (int i = 0; i <=BOTCOUNT; i++){
        int x = rand() % SCREEN_WIDTH;
        int y = rand() % SCREEN_HEIGHT;
        while(screen[y][x] != 0 && (x<=20 || x>=SCREEN_WIDTH-20 || y<=20 || y>=SCREEN_HEIGHT-20)){
            x = rand() % SCREEN_WIDTH;
            y = rand() % SCREEN_HEIGHT;
        }
        snake_state* s = initSnakeState(SCREEN_HEIGHT, SCREEN_WIDTH, x, y);
        screen[s->head->y][s->head->x] = 1;
        states[i] = s;
    }
    

    int score = 0;

    /*game loop */
    while(1){
        nanosleep(&ts, NULL);
        int foodEaten = 0;
        int res = updateSnakeLayer(states[0], food_layer, wall_layer, dir);
        if(res == -1){
            break;
        }
        if(res == 1){
            score++;
            foodEaten++;
        }
        for(int i = 1; i <=BOTCOUNT; i++){
            if(states[i]->isActive == 0){
                continue;
            }
            char dir = decideMove(states[i], screen);
            int res = updateSnakeLayer(states[i], food_layer, wall_layer, dir);
            if(res == -1){
                states[i]->isActive = 0;
            }
            if(res == 1){
                foodEaten++;
            }
        }
        //TODO: detect collision with other snakes
        snake* new_snake_head = states[0]->end;
        int iscolliding = 0;
        for(int i =1 ; i<=BOTCOUNT; i++){
            if(*(getcoordinatesPointer(states[i], new_snake_head->x, new_snake_head->y))>0){
                iscolliding = 1;
                break;
            }
        }

        if(iscolliding) {
            break;
        }

        for(int i = 1 ; i<=BOTCOUNT ; i++){
            int iscolliding = 0;
            snake* new_snake_head = states[i]->end;
            for(int j = 0 ; j <= BOTCOUNT ; j++){
                if(i != j){
                    if(*(getcoordinatesPointer(states[j], new_snake_head->x, new_snake_head->y))>0){
                        iscolliding = 1;
                        break;
                    }
                }
            }
            if(iscolliding){
                states[i]->isActive = 0;
            }
        }

        snake* new_head = states[0]->end;
        compose_layers(SCREEN_HEIGHT, SCREEN_WIDTH, screen, wall_layer, food_layer, states);
        get_pov(screen, pov, new_head->x, new_head->y);
        render(POV_HEIGHT, POV_WIDTH, pov);
        //add food 
        for(int j = 0 ; j < foodEaten ; j++){
            while(1){
            food.x = rand() % SCREEN_WIDTH;
            food.y = rand() % SCREEN_HEIGHT;
            if(screen[food.y][food.x] != 0){
                break;
            }
        }
        food_layer[food.y][food.x] = 1;
        }
    }
    printf("Game Over :%d\n", score);
    exit(0);
}