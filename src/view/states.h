#ifndef STATES_H
#define STATES_H
#include "../logic/snake.h"
enum GAMESTATE {
  START_SCREEN,
  GAME_SCREEN,
  ADVANCE_SCREEN,
  EXIT_SCREEN,
};

struct GameOptions {
  int map_width;
  int map_height;
  int foodcount;
  float speed_multiplier;
  int player_count;
  int score;
};

extern struct GameOptions GAMECONTEXT;

#endif