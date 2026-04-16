#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct thread_args {
    int arg1;
    char arg2[100];
    int ret;
};

void* thread_entry(void* args) {
    struct thread_args* arg_ptr = (struct thread_args*) args;

    printf("thread arg1: %d\n", arg_ptr->arg1);
    printf("thread arg2: %s\n", arg_ptr->arg2);

    arg_ptr->ret = arg_ptr->arg1 * 2;

    return NULL;
}

int main() {
    int num_threads = 5;
    pthread_t tids[5];
    struct thread_args args[5];
    int ret;
    int i;

    for (i = 0; i < num_threads; i++) {
        args[i].arg1 = i;
        snprintf(args[i].arg2, sizeof(args[i].arg2), "This is thread %d", i);
        args[i].ret = 0;

        ret = pthread_create(&tids[i], NULL, thread_entry, &args[i]);
        if (ret != 0) {
            fprintf(stderr, "pthread_create failed: %d\n", ret);
            return -1;
        }
    }

    for (i = 0; i < num_threads; i++) {
        ret = pthread_join(tids[i], NULL);
        if (ret != 0) {
            fprintf(stderr, "pthread_join failed: %d\n", ret);
            return -1;
        }
    }

    for (i = 0; i < num_threads; i++) {
        printf("main saw ret from thread %d: %d\n", i, args[i].ret);
    }

    return 0;
}