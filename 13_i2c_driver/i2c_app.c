#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

#define     STM_OFF_TIME        _IOW('i', 0, uint8_t)
#define     STM_ON_TIME         _IOW('i', 1, uint8_t)
#define     STM_START_BLINK     _IO ('i', 2)
#define     STM_GET_TIME        _IOR('i', 3, uint8_t*)

int main(int argc ,char *argv[]) {
    int fd;
    uint8_t delay, kern_data[2] = {0};

    // Open the device
    fd = open("/dev/stm32f103_dev", O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return -1;
    }

    if(argc == 3) {
        // Set the parameter value
        delay = atoi(argv[1]);
        if (ioctl(fd, STM_OFF_TIME, &delay) == -1) {
            perror("Failed to set on parameter");
            close(fd);
            return -1;
        }
        printf("ON time value is  %d\n", delay);

        // Get the parameter value
        delay = atoi(argv[2]);
        if (ioctl(fd, STM_ON_TIME, &delay) == -1) {
            perror("Failed to set off parameter");
            close(fd);
            return -1;
        }
        printf("OFF time value is %d\n", delay);
    } else if(argc == 1) {

        // Get the parameter value
        if (ioctl(fd, STM_GET_TIME, &kern_data) == -1) {
            perror("Failed to set off parameter");
            close(fd);
            return -1;
        }
        printf("Blink value is ON:%d - OFF:%d\n", kern_data[0], kern_data[1]);
    }
    // Close the device
    close(fd);
    return 0;
}
