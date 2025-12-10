#ifndef RENDER_H
#define RENDER_H

#include "../config.h"
#include "../logic/snake.h"
#include "../utils/d_array.h"
#include <stddef.h>

extern const char *FILL100;
extern const char *FILL75;
extern const char *FILL50;
extern const char *FILL25;
extern const char *FILL0;

extern const char *CLEAR;
extern const char *RESET_CURSOR;

void render(DArray *screen);
void get_pov(DArray *screen, DArray *pov, int x, int y);
void compose_layers(DArray *screen, DArray *wall_layer, DArray *food_layer,
                    snake_state *states[]);
void render_menu(char *banner[], int banner_lines, char *options[],
                 int option_count, int selected_option);

#endif // RENDER_H