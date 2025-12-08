#ifndef BOT_H
#define BOT_H
#include "../config.h"
#include "../utils/d_array.h"
#include "snake.h"
char decideMove(snake_state* ss, DArray* screen);
#endif