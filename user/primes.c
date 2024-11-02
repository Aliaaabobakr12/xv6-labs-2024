#include "kernel/types.h"
#include "user/user.h"

void sieve(int left_pipe) __attribute__((noreturn));

void sieve(int left_pipe) {
    int prime;
    if (read(left_pipe, &prime, sizeof(prime)) == 0) {
        close(left_pipe);
        exit(0);
    }

    printf("prime %d\n", prime);

    int right_pipe[2];
    pipe(right_pipe);

    if (fork() == 0) {
        close(right_pipe[1]);
        sieve(right_pipe[0]);
    } else {
        close(right_pipe[0]);
        int num;
        while (read(left_pipe, &num, sizeof(num)) != 0) {
            if (num % prime != 0) {
                write(right_pipe[1], &num, sizeof(num));
            }
        }
        close(left_pipe);
        close(right_pipe[1]);
        wait(0);
        exit(0);
    }
}

int main() {
    int start_pipe[2];
    pipe(start_pipe);

    if (fork() == 0) {
        close(start_pipe[1]);
        sieve(start_pipe[0]);
    } else {
        close(start_pipe[0]);
        for (int i = 2; i <= 280; i++) {
            write(start_pipe[1], &i, sizeof(i));
        }
        close(start_pipe[1]);
        wait(0);
        exit(0);
    }
}

