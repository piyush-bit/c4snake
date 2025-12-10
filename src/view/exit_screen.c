
#include "exit_screen.h"
#include "../view/states.h"
#include <stdio.h>
#include <stdlib.h>

int exit_screen() {
  printf("Goodbye!\n");
  exit(0);
  return EXIT_SCREEN; // Should not reach here
}
