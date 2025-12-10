#include "../platform/terminal.h"
#include "../utils/d_array.h"
#include "../view/render.h"
#include "../view/states.h"
#include "start_screen.h"
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum UPDOWN {
    UP = -1,
    DOWN = 1,
    NONE = 0,
    RETURN = 2
};

int updownCapture() {
    char c;

    read(STDIN_FILENO, &c, 1);

    if (c == '\x1b') {
        char b1, b2;

        read(STDIN_FILENO, &b1, 1);
        read(STDIN_FILENO, &b2, 1);

        if (b1 == '[') {
            switch (b2) {
                case 'A': return UP;   
                case 'B': return DOWN; 
            }
        }
    }

   
    switch (c) {
        case 'w': 
        case 'W': return UP;
        case 's': 
        case 'S': return DOWN;

        case '\n':  
        case '\r':  
            return RETURN;
    }

    return NONE;
}

int startGame(){
    return GAME_SCREEN;
}

int exitGame(){
    return EXIT_SCREEN;
}

int advanceOptions(){
    return ADVANCE_SCREEN;
}


int start_screen(struct GameOptions* state) {
  srand(time(NULL));

  printf("%s", CLEAR);
  printf("%s", RESET_CURSOR);

  DArray* screen = d_array_create(POV_HEIGHT, POV_WIDTH, ' ');

  char* Banner[] ={
    "  /$$$$$$  /$$   /$$  /$$$$$$                      /$$                 ",
    " /$$__  $$| $$  | $$ /$$__  $$                    | $$                 ",
    "| $$  \\__/| $$  | $$| $$  \\__/ /$$$$$$$   /$$$$$$ | $$   /$$  /$$$$$$  ",
    "| $$      | $$$$$$$$|  $$$$$$ | $$__  $$ |____  $$| $$  /$$/ /$$__  $$ ",
    "| $$      |_____  $$ \\____  $$| $$  \\ $$  /$$$$$$$| $$$$$$/ | $$$$$$$$ ",
    "| $$    $$      | $$ /$$  \\ $$| $$  | $$ /$$__  $$| $$_  $$ | $$_____/ ",
    "|  $$$$$$/      | $$|  $$$$$$/| $$  | $$|  $$$$$$$| $$ \\  $$|  $$$$$$$ ",
    " \\______/       |__/ \\______/ |__/  |__/ \\_______/|__/  \\__/ \\_______/ ",
    };

  char* options[] = {
    "Start Game",
    "Advance Options",
    "Quit"
  };

  int (*option_funcs[])() = { startGame,advanceOptions,exitGame };

  int selected_option = 0;
  while (1) {

      int option_count = sizeof(options)/sizeof(options[0]);
      int starting_row = (POV_HEIGHT/2)-4-(option_count*3/2);
      int starting_col = (POV_WIDTH/2)-35;

      for(int i = 0 ; i < 10 ; i++){
        for(int j=0;j<74;j++){
          if(i==0||i==9||j<2 || j>=72){
            (*d_array_get(screen, j+starting_col, i+starting_row-1)) = ' ';
            continue;
          }
          (*d_array_get(screen, j+starting_col-2, i+starting_row-1)) = Banner[i-1][j-2];
        }
      }

      for (int k = 0; k < option_count; k++) {
        size_t len = strlen(options[k]);
        starting_col = POV_WIDTH/2 - len/2 + 1;
        starting_row = POV_HEIGHT - 1 - (option_count - k) * 2;

        for (int i = 0; i < 2; i++) {
          for (int j = 0; j < (int)(len + 4); j++) {
            char *cell = d_array_get(screen, j + starting_col - 2, i + starting_row - 1);

            if (i == 0 || j == 1 || j == (int)(len + 2)) {
              *cell = ' ';
              continue;
            }

            if (j == 0) {
              *cell = (selected_option == k) ? '>' : ' ';
              continue;
            }

            if (j == (int)(len + 3)) {
              *cell = (selected_option == k) ? '<' : ' ';
              continue;
            }

            /* now j is between 2 and len+1 inclusive */
            *cell = options[k][j - 2];
          }
        }
      }
      render(screen);
      switch (updownCapture()) {
        case UP : selected_option = (selected_option-1+option_count)%option_count;break;
        case DOWN : selected_option = (selected_option+1+option_count)%option_count;break;
        case RETURN : return option_funcs[selected_option]();break;
      }
  }
    
}