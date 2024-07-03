#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_LEN         128
#define KERN_FILE       "/dev/poll_dev"

int main(int argc, char *argv[])
{
    char kern_data[MAX_LEN];
    int fd, ret;
    size_t d;
    struct pollfd fds;


    fd = open(KERN_FILE, O_RDONLY);
    if(fd == -1) {
        perror("File open: ");
        exit(EXIT_FAILURE);
    }
    fds.fd = fd;
    fds.events = POLLIN | POLLRDNORM;
    while(1) {
        ret = poll(&fds, (unsigned long)1, 1000);
        if(ret < 0) {
            perror("poll: ");
        }

        memset(kern_data, 0, sizeof(kern_data));
        if(fds.revents & POLLIN) {
            read(fds.fd, kern_data, sizeof(kern_data));
            printf("Kernel data: %s", kern_data);
        }
    }
    return 0;
}
