#include "terminal.h"

#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>

struct termios orig_termios;

int POV_WIDTH = 0;
int POV_HEIGHT = 0;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON); 
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void updatePOVSize() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    POV_WIDTH = w.ws_col/2;
    POV_HEIGHT = w.ws_row-2;
}

void handleResize(int sig) {
    (void)sig;  // unused
    updatePOVSize();
}

void initTerminal() {
    enableRawMode();
    updatePOVSize();
    signal(SIGWINCH, handleResize);  // handle terminal resize TODO: update only when not in game can cause seg fault
}
