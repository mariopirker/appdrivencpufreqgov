#include "config.h"
#include <inttypes.h>
#include <math.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


int main(void)
{
        int     fd_in, fd_out;
        int     nbytes;
        pid_t   childpid;
        char    string[] = "Test\n";
        char pipe_in[] = "/tmp/hb_in";
        char pipe_out[] = "/tmp/hb_out";
        char    readbuffer[80];


        if((childpid = fork()) == -1)
        {
                perror("fork");
                return(1);
        }
        if(childpid == 0)
        {
            fd_in = open(pipe_in, O_WRONLY);
            write(fd_in, string, strlen(string) + 1);
        }
        else
        {
            fd_out = open(pipe_out, O_RDONLY);
            nbytes = read(fd_out, readbuffer, 80);
            readbuffer[nbytes] = '\0';
            printf("%s\n", readbuffer);
        }
        return(0);
}
