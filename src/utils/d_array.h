#ifndef D_ARRAY_H
#define D_ARRAY_H

#include <stdlib.h>

/**
 * structure describing the 2d array
 *
 * @param row the number of rows
 * @param col the number of columns
 * @param data the pointer to the raw data
 */
typedef struct {
  char *data;
  int row;
  int col;
} DArray;

/**
 * create a new DArray with the given rows and cols
 */
DArray *d_array_create(int row, int col, char default_value);

/**
 * free the DArray
 */
void d_array_free(DArray *array);

/**
 * get the pointer to the element at the given x and y
 */
char *d_array_get(DArray *array, int x, int y);

#endif
