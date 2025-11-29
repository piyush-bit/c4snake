#include "input.h"
#include <stdio.h>
#include <unistd.h>

char dir = 'd';

void *loop(void* args) {
    (void)args;
    char c;

    while (1) {
        read(STDIN_FILENO, &c, 1);  

        if (c == '\x1b') {
            char b1, b2;

            read(STDIN_FILENO, &b1, 1);
            read(STDIN_FILENO, &b2, 1);

            if (b1 == '[') {
                switch (b2) {
                    case 'A': dir = 'w'; break;
                    case 'B': dir = 's'; break;
                    case 'C': dir = 'd'; break;
                    case 'D': dir = 'a'; break;
                }
            }
            continue;
        }

        switch (c) {
            case 'w': case 'W': dir = 'w'; break;
            case 's': case 'S': dir = 's'; break;
            case 'a': case 'A': dir = 'a'; break;
            case 'd': case 'D': dir = 'd'; break;
        }
    }
}
