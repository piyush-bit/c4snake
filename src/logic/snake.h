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
    int length;
    char dir;
} snake_state;

void compute_snake(snake_state* s, char new_dir);

#endif // SNAKE_H