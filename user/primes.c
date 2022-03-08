#include "kernel/types.h"
#include "user/user.h"


#define READEND 0
#define WRITEEND 1

/*
 * solution
 * p = get a number from left neighbor
 * print p
 * loop: 
 *  n = get a number from left neighbor
 *  if (p does not divide n)
 *      send n to right neighbor
 *
 */

// subfunction to handler primes
void pipeline(int*);

int main(int argc, char* argv[])
{
    // argument validate
//    if (argc != 2) {
//        fprintf(2, "ERROR: argument is not enough\n");
//        exit(1);
//    }
//    int number = atoi(argv[1]);
    int number = 35;
    int pchannel[2] = {0,0};
    int pid = 0;
    // init channel
    pipe(pchannel);

    pid = fork();
    if (pid == 0 ) { // this is child process
        pipeline(pchannel);
        exit(0);
    } else if (pid > 0) { // this is parent process
        close(pchannel[READEND]);
        for (int i = 2; i < number + 1; i ++) {
            if (write(pchannel[WRITEEND], &i, sizeof(int)) == 0) {
                fprintf(2, "p[%d]->c[%d] send %d failed", getpid(), pid, i);
                close(pchannel[WRITEEND]);
                wait((int*)0);
            }
        }
        close(pchannel[WRITEEND]);
        wait((int*)0);
        exit(0);
    } else {
        fprintf(2, "ERROR: fork failed");
        exit(1);
    }
}

// pipeline is used to create pipe line workflow
void pipeline(int* pchannel)
{
    close(pchannel[WRITEEND]);
    int buf = 0;
    if (read(pchannel[READEND], &buf, sizeof(int)) <= 0) {
        close(pchannel[READEND]);
        exit(0);
    }
    int primes_number = buf;
    // print primes
    fprintf(1, "prime %d\n", primes_number);
    int cchanel[2] = {0,0};
    pipe(cchanel);
    int pid = fork();
    if (pid == 0) { // this is child process
        pipeline(cchanel);
    } else if (pid > 0) { // parent
        close(cchanel[READEND]);
        // read from parent'channel
        while (read(pchannel[READEND], &buf, sizeof(int)) > 0)  
        {  
            if (buf%primes_number!=0) {
                if (write(cchanel[WRITEEND], &buf , sizeof(int)) <= 0) {  // write to child'channel
                    fprintf(2, "ERROR: receive data from channel failed");
                    close(pchannel[READEND]);
                    close(cchanel[WRITEEND]);
                    wait((int*)0);
                    exit(1);
                }
            }
        }
        close(pchannel[READEND]);
        close(cchanel[WRITEEND]);
        wait((int*)0);
        exit(0);

    } else { // fork failed
        fprintf(2, "ERROR: c[%d] fork failed", getpid());
        close(pchannel[READEND]);
        close(cchanel[WRITEEND]);
        close(cchanel[READEND]);
        exit(-1);
    }
    close(pchannel[READEND]);
    fprintf(1, "pchannel[READEND] has closed");
    exit(0);
}
