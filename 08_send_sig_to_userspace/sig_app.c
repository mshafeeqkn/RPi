#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>

#define KERN_FILE       "/dev/sig_dev"

#define SIG_NUM         44
#define REGISTER_APP    _IO('R', 'g')

void signal_handler(int sig) {
        printf("Signal sent from kernel!\n");
}

int main() {
        int fd;
        signal(SIG_NUM, signal_handler);

        printf("PID: %d\n", getpid());

        /* Open the device file */
        fd = open(KERN_FILE, O_RDONLY);
        if(fd < 0) {
                perror("Could not open device file");
                return -1;
        }

        /* Register app to KM */
        if(ioctl(fd, REGISTER_APP, NULL)) {
                perror("Error registering app");
                close(fd);
                return -1;
        }

        /* Wait for Signal */
        printf("Wait for signal...\n");
        while(1) {
                sleep(1);
    }

        return 0;
}
