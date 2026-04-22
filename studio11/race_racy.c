#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int race = 0;
int iterations = 20000000;

void* adder(void* arg) {
    for (int i = 0; i < iterations; i++) {
        race++;
    }
    return NULL;
}

void* subtractor(void* arg) {
    for (int i = 0; i < iterations; i++) {
        race--;
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;
    int ret;

    race = 0;

    ret = pthread_create(&t1, NULL, adder, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_create failed: %d\n", ret);
        return -1;
    }

    ret = pthread_create(&t2, NULL, subtractor, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_create failed: %d\n", ret);
        return -1;
    }

    ret = pthread_join(t1, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_join failed: %d\n", ret);
        return -1;
    }

    ret = pthread_join(t2, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_join failed: %d\n", ret);
        return -1;
    }

    printf("Final value of race: %d\n", race);

    return 0;
}