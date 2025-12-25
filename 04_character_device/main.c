#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

#define DEV_NAME "/dev/led_device0"

int main() {
    printf("START PROGRAM: CUSTOM DRIVER\n");

    // Open the GPIO device
    int gpio_fp = open(DEV_NAME, O_RDWR);
    if (gpio_fp < 0) {
        printf("GPIO Control: Open failed (error code: %d)\n", gpio_fp);
        return -1;
    }

    // Initialize GPIO line request
    struct gpio_v2_line_request led_line_req = {
        .consumer = "LED DEMO",
        .config = GPIO_V2_LINE_FLAG_OUTPUT,
        .flags = GPIO_V2_LINE_FLAG_OUTPUT,
        .lines = 1
    };

    // Set up the GPIO line
    int res = ioctl(gpio_fp, 1234, 0xABCD1234);
    if (res < 0) {
        printf("GPIO Control: ioctl failed (error code: %d)\n", res);
        close(gpio_fp);
        return -1;
    }

    // Simulate the delay (sleep for 100ms)
    sleep(100);

    printf("DEMO Finished\n");

    // Clean up
    close(led_line_req.fd);
    close(gpio_fp);

    return 0;
}
