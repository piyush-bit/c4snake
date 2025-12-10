#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "view/states.h"
#include "view/start_screen.h"
#include "view/game_screen.h"
#include "view/exit_screen.h"
#include "platform/terminal.h"


enum GAMESTATE currentState = START_SCREEN;

int main() {
  int (*option_funcs[10])();
  option_funcs[START_SCREEN] = start_screen;
  option_funcs[GAME_SCREEN] = game_screen;
  option_funcs[EXIT_SCREEN] = exit_screen;
  initTerminal();
  while (1) {
    currentState = option_funcs[currentState]();
  }
}