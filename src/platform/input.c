#include "input.h"
#include <stdio.h>

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