#include <stdio.h>

int main() {
    char buffer[1024];

    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        printf("Program 2 got: %s\n", buffer);
    }

    return 0;
}