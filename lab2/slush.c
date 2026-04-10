#define _POSIX_C_SOURCE 200809L

// SLUSH - The SLU SHELL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_LINE 256
#define MAX_ARGS 15
#define MAX_CMDS 64

typedef struct {
    char *argv[MAX_ARGS + 2];
    int argc;
} Command;

static volatile sig_atomic_t current_pgid = 0;

void sigint_handler(int signum) {
    (void)signum;
    if (current_pgid != 0) {
        kill(-current_pgid, SIGINT);
    }
    write(STDOUT_FILENO, "\n", 1);
}

int preprocess_input(const char *src, char *dst, size_t dst_size) {
    size_t i = 0;
    size_t j = 0;

    while (src[i] != '\0') {
        if (src[i] == '(') {
            if (j + 3 >= dst_size) {
                return -1;
            }
            dst[j++] = ' ';
            dst[j++] = '(';
            dst[j++] = ' ';
        } else {
            if (j + 1 >= dst_size) {
                return -1;
            }
            dst[j++] = src[i];
        }
        i++;
    }

    if (j >= dst_size) {
        return -1;
    }

    dst[j] = '\0';
    return 0;
}

int parse_line(char *line, Command cmds[], int *num_cmds) {
    char processed[MAX_LINE * 3];
    char *tok;
    int cmd_index = 0;
    int expect_command = 1;

    *num_cmds = 0;

    if (preprocess_input(line, processed, sizeof(processed)) == -1) {
        fprintf(stderr, "Input too long\n");
        return -1;
    }

    tok = strtok(processed, " \t\n");
    if (tok == NULL) {
        return 1;
    }

    cmds[0].argc = 0;

    while (tok != NULL) {
        if (strcmp(tok, "(") == 0) {
            if (expect_command || cmds[cmd_index].argc == 0) {
                fprintf(stderr, "Invalid null command\n");
                return -1;
            }

            cmds[cmd_index].argv[cmds[cmd_index].argc] = NULL;
            cmd_index++;

            if (cmd_index >= MAX_CMDS) {
                fprintf(stderr, "Too many commands\n");
                return -1;
            }

            cmds[cmd_index].argc = 0;
            expect_command = 1;
        } else {
            if (cmds[cmd_index].argc >= MAX_ARGS + 1) {
                fprintf(stderr, "Too many arguments\n");
                return -1;
            }

            cmds[cmd_index].argv[cmds[cmd_index].argc++] = tok;
            expect_command = 0;
        }

        tok = strtok(NULL, " \t\n");
    }

    if (expect_command || cmds[cmd_index].argc == 0) {
        fprintf(stderr, "Invalid null command\n");
        return -1;
    }

    cmds[cmd_index].argv[cmds[cmd_index].argc] = NULL;
    *num_cmds = cmd_index + 1;
    return 0;
}

int run_cd(Command *cmd) {
    if (cmd->argc != 2) {
        fprintf(stderr, "Invalid cd command\n");
        return -1;
    }

    if (chdir(cmd->argv[1]) == -1) {
        perror("cd");
        return -1;
    }

    return 0;
}

int execute_pipeline(Command cmds[], int num_cmds) {
    int prev_read = -1;
    pid_t pids[MAX_CMDS];
    int pid_count = 0;
    pid_t pgid = 0;

    for (int i = num_cmds - 1; i >= 0; i--) {
        int fd[2] = {-1, -1};

        if (i > 0) {
            if (pipe(fd) == -1) {
                perror("pipe");
                if (prev_read != -1) {
                    close(prev_read);
                }
                return -1;
            }
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            if (prev_read != -1) {
                close(prev_read);
            }
            if (i > 0) {
                close(fd[0]);
                close(fd[1]);
            }
            return -1;
        }

        if (pid == 0) {
            if (pgid == 0) {
                setpgid(0, 0);
            } else {
                setpgid(0, pgid);
            }

            signal(SIGINT, SIG_DFL);

            if (prev_read != -1) {
                if (dup2(prev_read, STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(-1);
                }
            }

            if (i > 0) {
                if (dup2(fd[1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(-1);
                }
            }

            if (prev_read != -1) {
                close(prev_read);
            }
            if (i > 0) {
                close(fd[0]);
                close(fd[1]);
            }

            execvp(cmds[i].argv[0], cmds[i].argv);
            perror(cmds[i].argv[0]);
            exit(-1);
        }

        if (pgid == 0) {
            pgid = pid;
        }
        setpgid(pid, pgid);

        pids[pid_count++] = pid;

        if (prev_read != -1) {
            close(prev_read);
        }

        if (i > 0) {
            close(fd[1]);
            prev_read = fd[0];
        } else {
            prev_read = -1;
        }
    }

    current_pgid = pgid;

    for (int i = 0; i < pid_count; i++) {
        while (waitpid(pids[i], NULL, 0) == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("waitpid");
            break;
        }
    }

    current_pgid = 0;
    return 0;
}

int main(void) {
    char line[MAX_LINE];
    struct sigaction sa;

    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return -1;
    }

    while (1) {
        Command cmds[MAX_CMDS];
        int num_cmds = 0;
        int parse_result;

        printf("slush> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            if (feof(stdin)) {
                printf("\n");
                break;
            }

            if (ferror(stdin)) {
                if (errno == EINTR) {
                    clearerr(stdin);
                    continue;
                }
                perror("fgets");
                break;
            }
        }

        parse_result = parse_line(line, cmds, &num_cmds);

        if (parse_result == 1) {
            continue;
        }
        if (parse_result == -1) {
            continue;
        }

        if (num_cmds == 1 && strcmp(cmds[0].argv[0], "cd") == 0) {
            run_cd(&cmds[0]);
            continue;
        }

        execute_pipeline(cmds, num_cmds);
    }

    return 0;
}