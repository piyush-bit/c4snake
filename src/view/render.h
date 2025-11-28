#ifndef RENDER_H
#define RENDER_H

#include <stddef.h>
#include "../config.h"

extern const char *FILL100;
extern const char *FILL75;
extern const char *FILL50;
extern const char *FILL25;
extern const char *FILL0;

extern const char *CLEAR;
extern const char *RESET_CURSOR;

void render(size_t rows, size_t cols, char screen[rows][cols]);
void get_pov(char screen[SCREEN_HEIGHT][SCREEN_WIDTH], char pov[POV_HEIGHT][POV_WIDTH], int x, int y);
void compose_layers(size_t rows, size_t cols, char screen[rows][cols], char wall_layer[rows][cols], char food_layer[rows][cols], char snake_layer[rows][cols]);

#endif // RENDER_H