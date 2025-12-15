#include "game_screen.h"
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
} cordinates;

void bountrywallPattern(DArray *wall_layer){
      /*setup the boundy */
  for (int i = 0; i < wall_layer->row; i++) {
    *(d_array_get(wall_layer, 0, i)) = 1;
    *(d_array_get(wall_layer, wall_layer->col - 1, i)) = 1;
  }
  for (int i = 0; i < wall_layer->col; i++) {
    *(d_array_get(wall_layer, i, 0)) = 1;
    *(d_array_get(wall_layer, i, wall_layer->row - 1)) = 1;
  }
}

void createWall(DArray *wall_layer, void (*pattern)(DArray*)){
    pattern(wall_layer);
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

struct gameComponent {
  DArray* wall_layer;
  DArray* food_layer;
  DArray* screen;
  snake_state** player_snake;
  size_t player_count;
  size_t max_players;
  // Add other game components as needed
};


void compose_layers_spawn(struct gameComponent* game) {
  size_t rows = game->screen->row;
  size_t cols = game->screen->col;
  for (size_t y = 0; y < rows; y++) {
    for (size_t x = 0; x < cols; x++) {
      if (*d_array_get(game->wall_layer, x, y) > 0) {
        (*d_array_get(game->screen, x, y)) = 1;
      } else if (*d_array_get(game->food_layer, x, y) == 1) {
        (*d_array_get(game->screen, x, y)) = 2;
      } else if (*d_array_get(game->food_layer, x, y) > 1) {
        (*d_array_get(game->food_layer, x, y)) = 0;
      } else {
        if (((y / 4) + (x / 4)) % 2 == 0) {
          (*d_array_get(game->screen, x, y)) = 0;
        } else {
          (*d_array_get(game->screen, x, y)) = -1;
        }
      }
    }
  }
  for (int i = 0; i < game->player_count; i++) {
    if (game->player_snake[i]->isActive == 0)
      continue;
    snake *temp = game->player_snake[i]->head;
    while (temp) {
      (*d_array_get(game->screen, temp->x, temp->y)) = 1;
      temp = temp->next;
    }
  }
}

cordinates findEmptyCellToSpawn(struct gameComponent* game) {
  // Implementation to find an empty cell for spawning food
  cordinates empty;
  compose_layers_spawn(game);
  while(1){
    empty.x = rand() % game->screen->col;
    empty.y = rand() % game->screen->row;
    if (*(d_array_get(game->screen, empty.x, empty.y)) == 0) {
      break;
    }
  }
  return empty;
}

int game_screen(struct GameOptions* state) {
  long target_frame_ns =
      (long)((1.0 / state->speed_multiplier) * 100.0 * 1000.0 * 1000.0);
  struct timespec start_time, end_time, sleep_time;
  srand(time(NULL));
  pthread_t t;
  pthread_create(&t, NULL, loop, NULL);

  DArray *pov = d_array_create(POV_HEIGHT, POV_WIDTH / 2, -1);
  DArray *wall_layer = d_array_create(state->map_height, state->map_width, 0);
  DArray *food_layer = d_array_create(state->map_height, state->map_width, 0);

  DArray *screen = d_array_create(state->map_height, state->map_width, 0);

  
  
  struct gameComponent game = {
    .wall_layer = wall_layer,
    .food_layer = food_layer,
    .screen = screen,
    .player_snake = NULL,
    .player_count = 0
  };


  /*setup the boundy */
  createWall(wall_layer, bountrywallPattern);

  /*setup the food */
  for (int i = 0; i < state->foodcount; i++) {
    cordinates food = findEmptyCellToSpawn(&game);
    *(d_array_get(food_layer, food.x, food.y)) = 1;
  }

  /*setup the snake */

  snake_state **states =
      (snake_state **)malloc(sizeof(snake_state *) * (state->player_count));
  game.player_snake = states;
  for (int i = 0; i < state->player_count; i++) {
    
    cordinates spawn = findEmptyCellToSpawn(&game);
    snake_state *s = initSnakeState(state->map_height, state->map_width, spawn.x, spawn.y);
    (*d_array_get(screen, s->head->x, s->head->y)) = 1;
    states[i] = s;
    game.player_count = i + 1;
  }

  int score = 0;

  printf("%s", CLEAR);
  printf("%s", RESET_CURSOR);

  /*game loop */
  while (game.player_count > 0) {
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    int foodEaten = 0;
    char dir = decideMove(states[0], screen);

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
    for (int i = 1; i < state->player_count; i++) {
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
    for (int i = 1; i < state->player_count; i++) {
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

    for (int i = 1; i < state->player_count; i++) {
      if (states[i]->isActive == 0) {
        continue;
      }
      int iscolliding = 0;
      snake *new_snake_head = states[i]->end;
      for (int j = 0; j < state->player_count; j++) {
        if (states[j]->isActive == 0)
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

    // add food logic
    for (int j = 0; j < foodEaten; j++) {
      cordinates food = findEmptyCellToSpawn(&game);
      *(d_array_get(food_layer, food.x, food.y)) = 1;
    }

    snake *new_head = states[0]->end;
    compose_layers_in_pov(screen, wall_layer, food_layer, states,0);
    get_pov(screen, pov, new_head->x, new_head->y);

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long elapsed_ns = (end_time.tv_sec - start_time.tv_sec) * 1000000000L +
                      (end_time.tv_nsec - start_time.tv_nsec);
    long sleep_ns = target_frame_ns - elapsed_ns;

    if (sleep_ns > 0){
      sleep_time.tv_sec = sleep_ns / 1000000000L;
      sleep_time.tv_nsec = sleep_ns % 1000000000L;
      nanosleep(&sleep_time, NULL);
    }

    render2(pov);
    printf("Score: %d\n", score);
  }
  return EXIT_SCREEN;
}