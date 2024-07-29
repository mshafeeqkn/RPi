#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

#define     STM_OFF_TIME        _IOW('i', 0, uint8_t)
#define     STM_ON_TIME         _IOW('i', 1, uint8_t)
#define     STM_START_BLINK     _IO ('i', 3)
#define     STM_GET_TIME        _IOR('i', 4, uint8_t*)

#define     ON_INDEX            0
#define     OFF_INDEX           1

int main(int argc ,char *argv[]) {
    int fd;
    uint8_t tmp, kern_data[2] = {0};

    // Open the device
    fd = open("/dev/stm32f103_dev", O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return -1;
    }

    if(argc == 3) {
        // Set the parameter value
        tmp = atoi(argv[1]);
        if (ioctl(fd, STM_ON_TIME, &tmp) == -1) {
            perror("Failed to set on parameter");
            close(fd);
            return -1;
        }
        kern_data[ON_INDEX] = tmp;
        printf("ON time value is  %d\n", tmp);

        // Get the parameter value
        tmp = atoi(argv[2]);
        if (ioctl(fd, STM_OFF_TIME, &tmp) == -1) {
            perror("Failed to set off parameter");
            close(fd);
            return -1;
        }
        kern_data[OFF_INDEX] = tmp;
        printf("OFF time value is %d\n", tmp);

        if (ioctl(fd, STM_START_BLINK, &kern_data) == -1) {
            perror("Failed to set on parameter");
            close(fd);
            return -1;
        }
        printf("Data sent to the STM board\n", tmp);
    } else if(argc == 1) {

        // Get the parameter value
        if (ioctl(fd, STM_GET_TIME, &kern_data) == -1) {
            perror("Failed to set off parameter");
            close(fd);
            return -1;
        }
        printf("Blink data:\nON Time:%d\nOFF Time:%d\n", 
            kern_data[ON_INDEX], kern_data[OFF_INDEX]);
    } else {
        printf("Incorrect usage\n");
    }
    // Close the device
    close(fd);
    return 0;
}
