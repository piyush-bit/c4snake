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
const char *FILL0 =  " ";

const char *CLEAR = "\033[2J";
const char *RESET_CURSOR = "\033[H";

typedef struct snake {
    int x;
    int y;
    struct snake* next;
} snake;

typedef struct {
    snake* head; // head is the snake's tail
    snake* end; // end is the snake's head
    char dir;
} snake_state;

void compute_snake(snake_state* s, char new_dir) {
    // prevent reversing
    if ((new_dir == 'w' && s->dir == 's') ||
        (new_dir == 's' && s->dir == 'w') ||
        (new_dir == 'a' && s->dir == 'd') ||
        (new_dir == 'd' && s->dir == 'a')) {
        new_dir = s->dir; // ignore reverse input
    }

    // short names for clarity
    snake *old_tail = s->head;        // this is the list "head" in your naming (the tail of snake)
    snake *new_head_of_list = old_tail->next; // the node that will become the new "head" (list head)
    snake *old_head = s->end;         // the snake's current logical head (list tail->...->end)

    // compute coordinates for the node we're moving (old_tail will become the new logical head)
    int nx = old_head->x;
    int ny = old_head->y;
    switch (new_dir) {
        case 'w': ny = old_head->y - 1; break;
        case 's': ny = old_head->y + 1; break;
        case 'a': nx = old_head->x - 1; break;
        case 'd': nx = old_head->x + 1; break;
    }

    // detach old_tail from the front of the list:
    // new_head_of_list is now the first element of the list (could be NULL if length==1)
    s->head = new_head_of_list;

    // reuse old_tail node as the new logical head: set its coords and attach it after old_head
    old_tail->x = nx;
    old_tail->y = ny;
    old_tail->next = NULL;         // it will become the list's end (logical head)

    // attach: old_head->next was NULL before; now point it to the moved node
    old_head->next = old_tail;

    // update trackers
    s->end = old_tail;   // new logical head
    s->dir = new_dir;
}

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

char dir = 'd';

 void *loop(){
    while(1){
        char c;
        scanf("%c", &c);
        if (c == 'w' || c == 's' || c == 'a' || c == 'd') {
            dir = c;
        }
    }
 }


 void render(size_t rows , size_t cols , char screen[rows][cols]) {
    printf("%s", RESET_CURSOR);
    printf("%s", CLEAR);
    printf("%s", RESET_CURSOR);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            char* element ;
            switch(screen[i][j]) {
                case -1:
                    printf("%s%s", FILL50,FILL50);
                    break;
                case 0:
                    printf("%s%s", FILL25,FILL25);
                    break;
                case 1:
                    printf("%s%s", FILL100,FILL100);
                    break;
                case 2:
                    printf("%s%s", FILL50,FILL75);
                    break;
            }
        }
        printf("\n");
    }
}

void get_pov(char screen[30][70], char pov[30][70], int x, int y){
    int x_offset = x - 35; 
    int y_offset = y - 15; 

    for(int i = 0; i < 30; i++){
        for(int j = 0; j < 70; j++){
            int world_row = i + y_offset;
            int world_col = j + x_offset;

            if(world_row < 0 || world_row >= 30 || world_col < 0 || world_col >= 70){
                pov[i][j] = -1; 
            }else{
                pov[i][j] = screen[world_row][world_col];
            }
        }
    }
}


struct timespec ts = {0, 0.5*100*1000*1000};

typedef struct{
    int x;
    int y;
}food;

int main() {
    enableRawMode();
    pthread_t t;
    pthread_create(&t, NULL, loop, NULL);
    char screen[30][70];
    char pov[30][70];
    memset(screen, 0, sizeof(screen));
    memset(pov, -1, sizeof(pov));
    for(int i = 0; i < 30; i++){
        screen[i][0] = 1;
        screen[i][69] = 1;
    }
    for(int i = 0; i < 70; i++){
        screen[0][i] = 1;
        screen[29][i] = 1;
    }
    struct snake s = {10, 10};
    s.next = &(struct snake){10, 9, NULL};
    s.next->next = NULL;

    snake_state ss = {&s,s.next,'w'};

    snake* temp = ss.head;
    while(temp){
        screen[temp->y][temp->x] = 1;
        temp = temp->next;
    }
    food food;
    while(1){
        food.x = rand() % 70;
        food.y = rand() % 30;
        if(screen[food.y][food.x] == 0){
            break;
        }
    }
    screen[food.y][food.x] = 2;
    int score = 0;
    while(1){
        nanosleep(&ts, NULL);
        snake* end = ss.head;
        screen[end->y][end->x] = 0;
        compute_snake(&ss, dir);
        snake* new_head = ss.end;
        screen[new_head->y][new_head->x] = 1;
        if(new_head->x < 0 || new_head->x >= 70 || new_head->y < 0 || new_head->y >= 30){
            break;
        }
        get_pov(screen, pov, new_head->x, new_head->y);
        if(new_head->x == food.x && new_head->y == food.y){
            score++;
            screen[food.y][food.x] = 1;
            screen[end->y][end->x] = 1;
            snake* new_end = (snake*)malloc(sizeof(snake));
            new_end->x = end->x;
            new_end->y = end->y;
            new_end->next = ss.head;
            ss.head = new_end;
            while (screen[food.y][food.x] == 1) {
                food.x = rand() % 70;
                food.y = rand() % 30;
            }
            screen[food.y][food.x] = 2;
        }
        
        render(30, 70, pov);
    }
    printf("Game Over :%d\n", score);
    // pthread_join(t, NULL);
  return 0;
}