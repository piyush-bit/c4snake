#include "./d_array.h"
#include <stdio.h>
#include <string.h>

DArray *d_array_create(int row, int col, char default_value) {
  DArray *array = (DArray *)malloc(sizeof(DArray));
  if (array == NULL) {
    perror("Failed to allocate DArray struct");
    exit(1);
  }
  array->row = row;
  array->col = col;
  array->data = (char *)malloc(row * col * sizeof(char));
  if (array->data == NULL) {
    perror("Failed to allocate DArray data");
    free(array);
    exit(1);
  }
  // Initialize to 0 by default
  memset(array->data, default_value , row * col * sizeof(char));
  return array;
}

void d_array_free(DArray *array) {
  if (array) {
    if (array->data) {
      free(array->data);
    }
    free(array);
  }
}

char *d_array_get(DArray *array, int x, int y) {
  if (x < 0 || x >= array->col || y < 0 || y >= array->row) {
    return NULL;
  }
  return &array->data[y * array->col + x];
}
