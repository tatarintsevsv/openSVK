#include <iostream>
#include <string>
#include "svkmain.h"
#include <unistd.h>

using namespace std;



int main(int argc,char* argv[])
{
    svkMain* svk = new svkMain();
    svk->run();
    int interval=svk->get_pollinterval();
    delete svk;
    if(interval==0)
        return 0;

    // self fork
    int pipefd[2];
    if (pipe(pipefd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
    }
    pid_t cpid = fork();
    if (cpid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (cpid == 0) {    /* Child */
        while ((dup2(pipefd[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
        close(pipefd[1]);
        close(pipefd[0]);
        sleep(interval);
        execl(argv[0],argv[0],NULL);
        _exit(EXIT_FAILURE);
    }


    return 0;
}
