#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
struct termios orig_termios;

const char *FILL100 = "█";
const char *FILL75 = "▓";
const char *FILL50 = "▒";
const char *FILL25 = "░";
const char *FILL0 = " ";

const char *CLEAR = "\033[2J";
const char *RESET_CURSOR = "\033[H";


void disableRawMode() { 
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
 }

void enableRawMode() { 
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;     // <-- REQUIRED for VSCode/Ghostty
    raw.c_cc[VTIME] = 0;    // <-- REQUIRED for VSCode/Ghostty
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
 }

 void disable(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
 }

char dir = 'w';

 void *loop(){
    while(1){
        char c;
        scanf("%c", &c);
        if (c == 'w' || c == 's' || c == 'a' || c == 'd') {
            dir = c;
        }
    }
 }

 int count =0;

 void render(size_t rows , size_t cols , char screen[rows][cols]) {
    printf("%s", RESET_CURSOR);
    printf("%s", CLEAR);
    printf("%s", RESET_CURSOR);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%s", screen[i][j] ? FILL100 : FILL25);
        }
        printf("\n");
    }
    printf("%d\n", count);
    count++;
    // printf("%c was written\n", dir);
}

typedef struct {
    int x;
    int y;
} snake;

void compute(size_t rows , size_t cols , char screen[rows][cols], char dir, snake *snake){
    screen[snake->x][snake->y] = 0;
    switch (dir) {
        case 'w':
            snake->x=(snake->x-1+rows)%rows;
            break;
        case 's':
            snake->x=(snake->x+1+rows)%rows;
            break;
        case 'a':
            snake->y=(snake->y-1+cols)%cols;
            break;
        case 'd':
            snake->y=(snake->y+1+cols)%cols;
            break;
    }
    screen[snake->x][snake->y] = 1;
}




struct timespec ts = {0, 2*100*1000*1000};

int main() {
    enableRawMode();
    pthread_t t;
    pthread_create(&t, NULL, loop, NULL);
    char screen[30][70];
    memset(screen, 0, sizeof(screen));
    for(int i = 0; i < 30; i++){
        screen[i][0] = 1;
        screen[i][69] = 1;
    }
    for(int i = 0; i < 70; i++){
        screen[0][i] = 1;
        screen[29][i] = 1;
    }
    snake s = {10, 10};
    screen[10][10] = 1;
    while(1){
        nanosleep(&ts, NULL);
        compute(30, 70, screen, dir, &s);
        render(30, 70, screen);
        // dir= '\0';
    }
    pthread_join(t, NULL);
  return 0;
}