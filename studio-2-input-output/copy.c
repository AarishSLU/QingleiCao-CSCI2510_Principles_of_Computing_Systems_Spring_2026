#include <unistd.h>

#define bufferSize 4096

int main(void) {
    char buffer[bufferSize];

    while (1) {
        ssize_t n = read(STDIN_FILENO, buffer, bufferSize);
        if (n == 0) break;   // EOF
        if (n < 0) return 1;    // error

        ssize_t written = 0;
        while (written < n) {
            ssize_t w = write(STDOUT_FILENO, buffer + written, n - written);
            if (w < 0) return 1;
            written += w;
        }
    }
    return 0;
}