#ifndef RENDER_H
#define RENDER_H

#include <stddef.h>

extern const char *FILL100;
extern const char *FILL75;
extern const char *FILL50;
extern const char *FILL25;
extern const char *FILL0;

extern const char *CLEAR;
extern const char *RESET_CURSOR;

void render(size_t rows, size_t cols, char screen[rows][cols]);
void get_pov(char screen[30][70], char pov[30][70], int x, int y);

#endif // RENDER_H