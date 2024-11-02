#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX 38
#define FIRST_PRIME 2

int create_natural_numbers();
int filter_primes(int in_fd, int prime);

int main(int argc, char *argv[]) {
    int prime;
    int in = create_natural_numbers();
    int child_pid;

    while (read(in, &prime, sizeof(int))) {
        printf("Found prime: %d\n", prime);
        in = filter_primes(in, prime);
    }

    while ((child_pid = wait((int*)0)) > 0) {
    }

    exit(0);
}

int create_natural_numbers() {
    int out_pipe[2];
    pipe(out_pipe);

    if (fork() == 0) {
        for (int i = FIRST_PRIME; i < MAX; i++) {
            write(out_pipe[1], &i, sizeof(int));
        }
        close(out_pipe[1]);
        exit(0);
    }

    close(out_pipe[1]);
    return out_pipe[0];
}

int filter_primes(int in_fd, int prime) {
    int num;
    int out_pipe[2];
    pipe(out_pipe);

    if (fork() == 0) {
        while (read(in_fd, &num, sizeof(int))) {
            if (num % prime) {
                write(out_pipe[1], &num, sizeof(int));
            }
        }
        close(in_fd);
        close(out_pipe[1]);
        exit(0);
    }

    close(in_fd);
    close(out_pipe[1]);
    return out_pipe[0];
}
