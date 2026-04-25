#define _GNU_SOURCE
#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <crypt.h>
#include <pthread.h>

#define MAX_KEYSIZE 8
#define ALPHABET_SIZE 26

char target[256];
char salt[3];

int num_threads;
int max_keysize;

volatile int found = 0;
char found_password[MAX_KEYSIZE + 1];

pthread_mutex_t found_mutex = PTHREAD_MUTEX_INITIALIZER;

struct thread_args {
    int thread_id;
    int length;
};

void index_to_password(long long index, int length, char *password) {
    for (int i = length - 1; i >= 0; i--) {
        password[i] = 'a' + (index % ALPHABET_SIZE);
        index /= ALPHABET_SIZE;
    }
    password[length] = '\0';
}

long long power26(int length) {
    long long result = 1;
    for (int i = 0; i < length; i++) {
        result *= ALPHABET_SIZE;
    }
    return result;
}

void* crack_length(void* arg) {
    struct thread_args* args = (struct thread_args*) arg;

    int tid = args->thread_id;
    int length = args->length;

    struct crypt_data data;
    data.initialized = 0;

    char password[MAX_KEYSIZE + 1];

    long long total = power26(length);

    for (long long i = tid; i < total; i += num_threads) {
        if (found) {
            break;
        }

        index_to_password(i, length, password);

        char* result = crypt_r(password, salt, &data);
        if (result == NULL) {
            continue;
        }

        if (strcmp(result, target) == 0) {
            pthread_mutex_lock(&found_mutex);

            if (!found) {
                found = 1;
                strcpy(found_password, password);
                printf("%s\n", found_password);
                fflush(stdout);
            }

            pthread_mutex_unlock(&found_mutex);
            break;
        }
    }

    return NULL;
}

int run_threads_for_length(int length) {
    pthread_t* threads = malloc(sizeof(pthread_t) * num_threads);
    struct thread_args* args = malloc(sizeof(struct thread_args) * num_threads);

    if (threads == NULL || args == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(threads);
        free(args);
        return -1;
    }

    for (int i = 0; i < num_threads; i++) {
        args[i].thread_id = i;
        args[i].length = length;

        int ret = pthread_create(&threads[i], NULL, crack_length, &args[i]);
        if (ret != 0) {
            fprintf(stderr, "pthread_create failed: %d\n", ret);
            free(threads);
            free(args);
            return -1;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        int ret = pthread_join(threads[i], NULL);
        if (ret != 0) {
            fprintf(stderr, "pthread_join failed: %d\n", ret);
            free(threads);
            free(args);
            return -1;
        }
    }

    free(threads);
    free(args);

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: crack <threads> <keysize> <target>\n");
        return -1;
    }

    num_threads = atoi(argv[1]);
    max_keysize = atoi(argv[2]);

    if (num_threads < 1) {
        fprintf(stderr, "Number of threads must be at least 1\n");
        return -1;
    }

    if (max_keysize < 1 || max_keysize > MAX_KEYSIZE) {
        fprintf(stderr, "Keysize must be between 1 and 8\n");
        return -1;
    }

    strncpy(target, argv[3], sizeof(target) - 1);
    target[sizeof(target) - 1] = '\0';

    if (strlen(target) < 2) {
        fprintf(stderr, "Target hash must be at least 2 characters\n");
        return -1;
    }

    salt[0] = target[0];
    salt[1] = target[1];
    salt[2] = '\0';

    for (int length = 1; length <= max_keysize; length++) {
        if (run_threads_for_length(length) == -1) {
            return -1;
        }

        if (found) {
            break;
        }
    }

    return 0;
}