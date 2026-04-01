#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
    char buffer[1024];

    int max_args = 15;
    int max_argv_size = max_args + 2;
    char* cmd;
    char* my_argv[max_argv_size];

    printf("Enter input: ");

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return -1;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    cmd = strtok(buffer, " ");

    if (cmd == NULL) {
        return 0;
    }

    my_argv[0] = cmd;

    int i = 1;
    char* res = strtok(NULL, " ");

    while (res != NULL && i < max_argv_size - 1) {
        my_argv[i] = res;
        i++;
        res = strtok(NULL, " ");
    }

    my_argv[i] = '\0';

    execvp(cmd, my_argv);

    return 0;
}