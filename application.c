#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(void)
{
	char wbuf[256] = "Hyeanseok-OH 2022112053, Welcome to Embedded Software Class"; // string to write to device driver
	char rbuf[256]; // string buffer to read from device driver
 
	//Implemented using open, write and read functions

    // Open the device with read/write access
    int fd = open("/dev/dummy_device", O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return -1;
    }

    // Write to the device
    int bytes_written = write(fd, wbuf, strlen(wbuf));
    if (bytes_written < 0) {
        perror("Failed to write to the device");
        return -1;
    }

    // Read from the device
    int bytes_read = read(fd, rbuf, sizeof(rbuf));
    if (bytes_read < 0) {
        perror("Failed to read from the device");
        return -1;
    }

    // // Ensure null termination
    // rbuf[bytes_read] = '\0';

    // Print the read content
    printf("Read from device: %s\n", rbuf);

    // Close the device
    close(fd);

    return 1;
}