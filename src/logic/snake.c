#include "snake.h"
#include <stddef.h>

void compute_snake(snake_state* s, char new_dir) {
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