#include "render.h"
#include <stddef.h>
#include <stdio.h>

const char *FILL100 = "█";
const char *FILL75 = "▓";
const char *FILL50 = "▒";
const char *FILL25 = "░";
const char *FILL0 =  " ";

const char *CLEAR = "\033[2J";
const char *RESET_CURSOR = "\033[H";

void compose_layers(size_t rows, size_t cols, char screen[rows][cols], char wall_layer[rows][cols], char food_layer[rows][cols], snake_state* states[]){
    for(size_t y = 0; y < rows; y++){
        for(size_t x = 0; x < cols; x++){
            if (wall_layer[y][x] > 0) {
                screen[y][x] = 1;
            } else if (food_layer[y][x] == 1) {
                screen[y][x] = 2;
            }else if (food_layer[y][x] > 1){
                food_layer[y][x] = 0;
            }else {
                if ( ((y / 4) + (x / 4)) % 2 == 0 ) {
                    screen[y][x] = 0;
                } else {
                    screen[y][x] = -1;
                }
            }
            for(int k = 0; k < BOTCOUNT+1; k++){
                if(states[k]->isActive == 0){
                    continue;
                }
                if(*getcoordinatesPointer(states[k], x, y)>0){
                    screen[y][x] = 1;
                }
            }
        }
    }
}

void render(size_t rows, size_t cols, char screen[rows][cols]) {
    printf("%s", RESET_CURSOR);
    // printf("%s", CLEAR);
    // printf("%s", RESET_CURSOR);
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            switch(screen[i][j]) {
                case -1:
                    printf("%s%s", FILL50,FILL50);
                    break;
                case 0:
                    printf("%s%s", FILL25,FILL25);
                    break;
                case 1:
                    printf("%s%s", FILL100,FILL100);
                    break;
                case 2:
                    printf("%s%s", FILL75,FILL75);
                    break;
                case -2:
                    printf("%s%s", FILL0,FILL0);
                    break;
            }
        }
        printf("\n");
    }
}

void get_pov(char screen[SCREEN_HEIGHT][SCREEN_WIDTH], char pov[POV_HEIGHT][POV_WIDTH], int x, int y){
    int x_offset = x - POV_WIDTH / 2; 
    int y_offset = y - POV_HEIGHT / 2; 

    for(int i = 0; i < POV_HEIGHT; i++){
        for(int j = 0; j < POV_WIDTH; j++){
            int world_row = i + y_offset;
            int world_col = j + x_offset;

            if(world_row < 0 || world_row >= SCREEN_HEIGHT || world_col < 0 || world_col >= SCREEN_WIDTH){
                pov[i][j] = -1; 
            }else{
                pov[i][j] = screen[world_row][world_col];
            }
        }
    }
}