#include "render.h"
#include "../platform/terminal.h"
#include "../utils/d_array.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const char *FILL100 = "█";
const char *FILL75 = "▓";
const char *FILL50 = "▒";
const char *FILL25 = "░";
const char *FILL0 = " ";

const char *CLEAR = "\033[2J";
const char *RESET_CURSOR = "\033[H";

void compose_layers(DArray *screen, DArray *wall_layer, DArray *food_layer,
                    snake_state *states[]) {
  size_t rows = screen->row;
  size_t cols = screen->col;
  for (size_t y = 0; y < rows; y++) {
    for (size_t x = 0; x < cols; x++) {
      if (*d_array_get(wall_layer, x, y) > 0) {
        (*d_array_get(screen, x, y)) = 1;
      } else if (*d_array_get(food_layer, x, y) == 1) {
        (*d_array_get(screen, x, y)) = 2;
      } else if (*d_array_get(food_layer, x, y) > 1) {
        (*d_array_get(food_layer, x, y)) = 0;
      } else {
        if (((y / 4) + (x / 4)) % 2 == 0) {
          (*d_array_get(screen, x, y)) = 0;
        } else {
          (*d_array_get(screen, x, y)) = -1;
        }
      }
      for (int k = 0; k < BOTCOUNT + 1; k++) {
        if (states[k]->isActive == 0) {
          continue;
        }
        if (*getcoordinatesPointer(states[k], x, y) > 0) {
          (*d_array_get(screen, x, y)) = 1;
        }
      }
    }
  }
}

void render(DArray *screen) {
  printf("%s", RESET_CURSOR);
  printf("%s", CLEAR);
  printf("%s", RESET_CURSOR);
  size_t rows = screen->row;
  size_t cols = screen->col;

  for (size_t i = 0; i < rows; i++) {
    for (size_t j = 0; j < cols; j++) {
      int value = *d_array_get(screen, j, i);
      switch (value) {
      case -1:
        printf("%s", FILL50);
        break;
      case 0:
        printf("%s", FILL25);
        break;
      case 1:
        printf("%s", FILL100);
        break;
      case 2:
        printf("%s", FILL75);
        break;
      case -2:
        printf("%s", FILL0);
        break;
      default:
        printf("%c", (char)value);
      }
    }
    printf("\n");
  }
}

void get_pov(DArray *screen, DArray *pov, int x, int y) {
  int x_offset = x - POV_WIDTH / 2;
  int y_offset = y - POV_HEIGHT / 2;

  for (int i = 0; i < POV_HEIGHT; i++) {
    for (int j = 0; j < POV_WIDTH; j++) {
      int world_row = i + y_offset;
      int world_col = j + x_offset;

      if (world_row < 0 || world_row >= SCREEN_HEIGHT || world_col < 0 ||
          world_col >= SCREEN_WIDTH) {
        (*d_array_get(pov, j, i)) = -1;
      } else {
        (*d_array_get(pov, j, i)) =
            (*d_array_get(screen, world_col, world_row));
      }
    }
  }
}
void render_menu(char *banner[], int banner_lines, char *options[],
                 int option_count, int selected_option) {
  printf("%s", CLEAR);
  printf("%s", RESET_CURSOR);

  int center_x = POV_WIDTH / 2;
  int center_y = POV_HEIGHT / 2;

  // Calculate starting Y for banner to center it vertically along with options
  // Total height = banner_lines + gap + option_count
  int total_content_height = banner_lines + 3 + option_count;
  int start_y = center_y - (total_content_height / 2);

  // Render Banner
  for (int i = 0; i < banner_lines; i++) {
    int banner_width = strlen(banner[i]);
    int start_x = center_x - (banner_width / 2);

    // Move cursor
    printf("\033[%d;%dH", start_y + i, start_x);
    printf("%s", banner[i]);
  }

  // Render Options
  int option_start_y = start_y + banner_lines + 3; // 3 lines gap
  for (int i = 0; i < option_count; i++) {
    int option_width = strlen(options[i]) + 4; // +4 for pointer/brackets
    int start_x = center_x - (option_width / 2);

    printf("\033[%d;%dH", option_start_y + i, start_x);

    if (i == selected_option) {
      printf("> %s <", options[i]);
    } else {
      printf("  %s  ", options[i]);
    }
  }

  printf("\033[H"); // Move cursor back to top-left to avoid flickering cursor
                    // in middle
  fflush(stdout);
}
