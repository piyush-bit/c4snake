#ifndef SNAKE_H
#define SNAKE_H

#include <stddef.h>

typedef struct snake {
    int x;
    int y;
    struct snake* next;
} snake;

typedef struct {
    snake* head; // head is the snake's tail
    snake* end; // end is the snake's head
    snake* deleted;
    int h,w;
    char* snakeLayer;
    int length;
    char dir;
    char* shared_dir;
    int isActive;
} snake_state;

void compute_snake(snake_state* s, char new_dir);

snake_state* initSnakeState(int h, int w , int x , int y);
char* getcoordinatesPointer(snake_state* s , int x ,int y);

#endif // SNAKE_H