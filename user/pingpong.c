#include "kernel/types.h"
#include "user/user.h"


#define READEND    0
#define WRITEEND   1

int main(int argc, char* argv[])
{
    int p1[2] ;     // this is used to read in child, write in parent
    int p2[2];  // this is used to read in parent, write in child
    pipe(p1);
    pipe(p2);
    char buf[1] = " ";
    int pid = fork();
    if (pid == 0) { // this is child process 
        close(p1[WRITEEND]);
        close(p2[READEND]);
        if (read(p1[READEND],buf ,1) != 1) { // read 1 byte
            fprintf(2, "child:[%d]\tERROR: read from pipe failed\n", getpid());
            close(p1[READEND]);
            close(p2[WRITEEND]);
            exit(-1);
        } 
        fprintf(1, "%d: received ping\n", getpid());
        if (write(p2[WRITEEND], buf,1) != 1) {
            fprintf(2, "child : [%d]\tERROR: write to pipe failed\n", getpid());
            close(p1[READEND]);
            close(p2[WRITEEND]);
            exit(-1);
        }
        close(p1[READEND]);
        close(p2[WRITEEND]);
        exit(0);

    } else if(pid > 0){    // this is parent process
        close(p1[READEND]);
        close(p2[WRITEEND]);
        if (write(p1[WRITEEND], buf, 1) != 1) {
            fprintf(2, "parent: [%d]\tERROR: write to pipe failed\n", getpid());
            close(p1[READEND]);
            close(p2[WRITEEND]);
            exit(-1);
        }
        if (read(p2[READEND], buf, 1)!=1) {
            fprintf(2, "parent: [%d]\t ERROR: read from pipe faile\n", getpid());
            close(p1[READEND]);
            close(p2[WRITEEND]);
            exit(-1);
        }
        fprintf(1, "%d: received pong\n", getpid());
        close(p1[READEND]);
        close(p2[WRITEEND]);
        exit(0);
    } else {
        exit(1);
    }

}

