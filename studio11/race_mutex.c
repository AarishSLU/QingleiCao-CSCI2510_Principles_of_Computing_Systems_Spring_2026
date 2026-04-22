#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int race = 0;
int iterations = 20000000;

struct thread_data {
    pthread_mutex_t* mutex;
};

void* adder(void* arg) {
    struct thread_data* data = (struct thread_data*) arg;

    pthread_mutex_lock(data->mutex);
    for (int i = 0; i < iterations; i++) {
        race++;
    }
    pthread_mutex_unlock(data->mutex);

    return NULL;
}

void* subtractor(void* arg) {
    struct thread_data* data = (struct thread_data*) arg;

    pthread_mutex_lock(data->mutex);
    for (int i = 0; i < iterations; i++) {
        race--;
    }
    pthread_mutex_unlock(data->mutex);

    return NULL;
}

int main() {
    pthread_t t1, t2;
    int ret;

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    struct thread_data data;
    data.mutex = &mutex;

    race = 0;

    ret = pthread_create(&t1, NULL, adder, &data);
    if (ret != 0) {
        fprintf(stderr, "pthread_create failed: %d\n", ret);
        return -1;
    }

    ret = pthread_create(&t2, NULL, subtractor, &data);
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