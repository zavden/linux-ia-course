#include <stdio.h>
#include "greet.h"

void greet(const char *name) {
    if (name) {
        printf("Hello, %s!\n", name);
    } else {
        printf("Hello, world!\n");
    }
}
