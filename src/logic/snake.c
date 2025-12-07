#include "snake.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

void assign_random_dir(char* dir){
    char valid_dir [] = {'w', 'a', 's', 'd'};
    *dir = valid_dir[rand() % 4];
}
char* getcoordinatesPointer(snake_state* s , int x ,int y){
    return s->snakeLayer+(y*s->w+x);
}

snake_state* initSnakeState(int h, int w , int x , int y){
    snake* head = (snake*)malloc(sizeof(snake));
    snake* del = (snake*)malloc(sizeof(snake));
    head->x = x;
    head->y = y;
    head->next = NULL;
    snake_state* s = (snake_state*)malloc(sizeof(snake_state));
    s->head = head;
    s->end = head;
    s->deleted = del;
    s->h = h;
    s->w = w;
    s->snakeLayer = (char*)malloc(sizeof(char)*h*w);
    memset(s->snakeLayer, 0, h*w);
    s->length = 1;
    *getcoordinatesPointer(s, x, y) = 1;
    s->dir = 'a';
    s->shared_dir = (char*)malloc(sizeof(char));
    *s->shared_dir = 'a';
    s->isActive = 1;
    return s;
}


void compute_snake(snake_state* s, char new_dir) {
    if (new_dir == 0 || new_dir == ' '){
        assign_random_dir(&new_dir);
    }

    s->deleted->x=s->head->x;
    s->deleted->y=s->head->y;

    if (s->head->next == NULL){
        switch (new_dir) {
            case 'w': s->end->y -= 1; break;
            case 's': s->end->y += 1; break;
            case 'a': s->end->x -= 1; break;
            case 'd': s->end->x += 1; break;
        }
        s->dir = new_dir;
        return;
    }
    // prevent reversing
    if ((new_dir == 'w' && s->dir == 's') ||
        (new_dir == 's' && s->dir == 'w') ||
        (new_dir == 'a' && s->dir == 'd') ||
        (new_dir == 'd' && s->dir == 'a')) {
        new_dir = s->dir; // ignore reverse input
    }

    // short names for clarity
    snake *old_tail = s->head;        // this is the list "head" in your naming (the tail of snake)
    snake *new_head_of_list = old_tail->next; // the node that will become the new "head" (list head)
    snake *old_head = s->end;         // the snake's current logical head (list tail->...->end)

    // compute coordinates for the node we're moving (old_tail will become the new logical head)
    int nx = old_head->x;
    int ny = old_head->y;
    switch (new_dir) {
        case 'w': ny = old_head->y - 1; break;
        case 's': ny = old_head->y + 1; break;
        case 'a': nx = old_head->x - 1; break;
        case 'd': nx = old_head->x + 1; break;
    }

    // detach old_tail from the front of the list:
    // new_head_of_list is now the first element of the list (could be NULL if length==1)
    s->head = new_head_of_list;

    // reuse old_tail node as the new logical head: set its coords and attach it after old_head
    old_tail->x = nx;
    old_tail->y = ny;
    old_tail->next = NULL;         // it will become the list's end (logical head)

    // attach: old_head->next was NULL before; now point it to the moved node
    old_head->next = old_tail;

    // update trackers
    s->end = old_tail;   // new logical head
    s->dir = new_dir;
}