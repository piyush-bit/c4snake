#ifndef TERMINAL_H
#define TERMINAL_H

#include <termios.h>

extern struct termios orig_termios;
extern int POV_WIDTH;
extern int POV_HEIGHT;

void disableRawMode();
void enableRawMode();

void initTerminal();

#endif // TERMINAL_H