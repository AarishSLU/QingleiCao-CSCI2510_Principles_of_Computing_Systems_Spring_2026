// Aarish
// 2026/02/19

#include <unistd.h>

int main(int argc, char* argv[]){
    const char msg[] = "Hello, world!\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    return 0;
}
