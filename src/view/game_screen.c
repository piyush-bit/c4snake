#include "../config.h"
#include "../logic/bot.h"
#include "../logic/snake.h"
#include "../platform/input.h"
#include "../platform/terminal.h"
#include "../utils/d_array.h"
#include "../view/render.h"
#include "../view/states.h"
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
  int x;
  int y;
} food;

struct timespec ts = {0, SPEED_MULTIPLIER * 100 * 1000 * 1000};

void clearSnakeLayer(snake_state *ss, char snake_layer[SCREEN_HEIGHT][SCREEN_WIDTH]) {
  snake *temp = ss->head;
  while (temp) {
    snake_layer[temp->y][temp->x] = 0;
    temp = temp->next;
  }
}

int updateSnakeLayer(snake_state *ss, DArray *food_layer, DArray *wall_layer,int dir) {
  compute_snake(ss, dir);
  snake *new_head = ss->end;
  if (*(d_array_get(wall_layer, new_head->x, new_head->y)) == 1) {
    return -1;
  }
  (*getcoordinatesPointer(ss, new_head->x, new_head->y))++;
  if (*d_array_get(food_layer, new_head->x, new_head->y) > 0) {
    (*d_array_get(food_layer, new_head->x, new_head->y))++;
    ss->length++;
    snake *new_end = (snake *)malloc(sizeof(snake));
    new_end->x = ss->deleted->x;
    new_end->y = ss->deleted->y;
    new_end->next = ss->head;
    ss->head = new_end;
    return 1;
  }
  (*getcoordinatesPointer(ss, ss->deleted->x, ss->deleted->y))--;
  return 0;
}


int game_screen() {
  srand(time(NULL));
  pthread_t t;
  pthread_create(&t, NULL, loop, NULL);

  DArray *pov = d_array_create(POV_HEIGHT, POV_WIDTH/2, -1);
  DArray *wall_layer = d_array_create(SCREEN_HEIGHT, SCREEN_WIDTH, 0);
  DArray *food_layer = d_array_create(SCREEN_HEIGHT, SCREEN_WIDTH, 0);

  DArray *screen = d_array_create(SCREEN_HEIGHT, SCREEN_WIDTH, 0);

  /*setup the boundy */
  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    *(d_array_get(wall_layer, 0, i)) = 1;
    *(d_array_get(wall_layer, SCREEN_WIDTH - 1, i)) = 1;
  }
  for (int i = 0; i < SCREEN_WIDTH; i++) {
    *(d_array_get(wall_layer, i, 0)) = 1;
    *(d_array_get(wall_layer, i, SCREEN_HEIGHT - 1)) = 1;
  }

  /*setup the food */
  food food;
  for (int i = 0; i < FOODCOUNT; i++) {
    while (1) {
      food.x = rand() % SCREEN_WIDTH;
      food.y = rand() % SCREEN_HEIGHT;
      if ((*d_array_get(wall_layer, food.x, food.y) == 0) &&
          (*d_array_get(food_layer, food.x, food.y)) == 0) {
        break;
      }
    }
    *(d_array_get(food_layer, food.x, food.y)) = 1;
  }

  /*setup the snake */

  snake_state **states =
      (snake_state **)malloc(sizeof(snake_state *) * (BOTCOUNT + 1));
  for (int i = 0; i <= BOTCOUNT; i++) {
    int x = rand() % SCREEN_WIDTH;
    int y = rand() % SCREEN_HEIGHT;
    while ((*d_array_get(screen, x, y))
         && (x <= 20 || x >= SCREEN_WIDTH - 20 || y <= 20 ||
                                 y >= SCREEN_HEIGHT - 20)) {
      x = rand() % SCREEN_WIDTH;
      y = rand() % SCREEN_HEIGHT;
    }
    snake_state *s = initSnakeState(SCREEN_HEIGHT, SCREEN_WIDTH, x, y);
    (*d_array_get(screen, s->head->x, s->head->y)) = 1;
    states[i] = s;
  }

  int score = 0;

  printf("%s", CLEAR);
  printf("%s", RESET_CURSOR);

  /*game loop */
  while (1) {
    nanosleep(&ts, NULL);
    int foodEaten = 0;
    // char dir = decideMove(states[0], screen);

    if (states[0]->isActive) {
      int res = updateSnakeLayer(states[0], food_layer, wall_layer, dir);
      if (res == -1) {
        break;
      }
      if (res == 1) {
        score++;
        foodEaten++;
      }
    }
    for (int i = 1; i <= BOTCOUNT; i++) {
      if (states[i]->isActive == 0) {
        continue;
      }
      char dir = decideMove(states[i], screen);
      int res = updateSnakeLayer(states[i], food_layer, wall_layer, dir);
      if (res == -1) {
        states[i]->isActive = 0;
      }
      if (res == 1) {
        foodEaten++;
      }
    }
    // detect collision with other snakes
    snake *new_snake_head = states[0]->end;
    int iscolliding = 0;
    for (int i = 1; i <= BOTCOUNT; i++) {
      if (states[i]->isActive == 0) {
        continue;
      }
      if (*(getcoordinatesPointer(states[i], new_snake_head->x,
                                  new_snake_head->y)) > 0) {
        iscolliding = 1;
        break;
      }
    }

    if (iscolliding) {
      break;
    }

    for (int i = 1; i <= BOTCOUNT; i++) {
      if (states[i]->isActive == 0) {
        continue;
      }
      int iscolliding = 0;
      snake *new_snake_head = states[i]->end;
      for (int j = 0; j <= BOTCOUNT; j++) {
        if(states[j]->isActive==0)
            continue;
        if (i != j) {
          if (*(getcoordinatesPointer(states[j], new_snake_head->x,
                                      new_snake_head->y)) > 0) {
            iscolliding = 1;
            break;
          }
        }
      }
      if (iscolliding) {
        states[i]->isActive = 0;
      }
    }

    snake *new_head = states[0]->end;
    compose_layers(screen, wall_layer, food_layer,
                   states);
    get_pov(screen, pov, new_head->x, new_head->y);
    render2(pov);
    // add food
    for (int j = 0; j < foodEaten; j++) {
      while (1) {
        food.x = rand() % SCREEN_WIDTH;
        food.y = rand() % SCREEN_HEIGHT;
        if ((*d_array_get(screen, food.x, food.y)) == 0) {
          break;
        }
      }
      *(d_array_get(food_layer, food.x, food.y)) = 1;
    }
  }
  printf("Game Over :%d\n", score);
  return EXIT_SCREEN;
}