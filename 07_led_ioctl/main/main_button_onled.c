#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <pthread.h>
#include <linux/input.h>
#include <linux/ioctl.h>

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int led1_blink = 1;
int led2_blink = 1;

#define DEV_PATH        "/dev/led_device0"

#define LED_IOC_MAGIC   'L'
#define LED1_IOC_ON     _IO(LED_IOC_MAGIC, 0)
#define LED1_IOC_OFF    _IO(LED_IOC_MAGIC, 1)

#define LED2_IOC_ON     _IO(LED_IOC_MAGIC, 3)
#define LED2_IOC_OFF    _IO(LED_IOC_MAGIC, 4)

struct led_array {
    int count;
    int *vals;   /* user pointer */
};

#define LED_IOC_GET     _IOWR(LED_IOC_MAGIC, 2, struct led_array)

static int do_ioctl_simple(int fd, unsigned long req)
{
    if (ioctl(fd, req) < 0) {
        perror("ioctl");
        return -1;
    }
    return 0;
}

static int do_ioctl_get(int fd, int count)
{
    if (count <= 0 || count > 8) {
        fprintf(stderr, "count must be 1..8\n");
        return -1;
    }

    int *vals = calloc((size_t)count, sizeof(int));
    if (!vals) {
        perror("calloc");
        return -1;
    }

    struct led_array ua;
    ua.count = count;
    ua.vals = vals;

    if (ioctl(fd, LED_IOC_GET, &ua) < 0) {
        perror("ioctl(LED_IOC_GET)");
        free(vals);
        return -1;
    }

    for (int i = 0; i < count; i++) {
        printf("LED[%d] = %d\n", i + 1, vals[i]);
    }

    free(vals);
    return 0;
}



void *read_button(void *arg)
{
    int fd_key = open("/dev/input/event0", O_RDONLY);

    if (fd_key < 0)
    {
        printf("Open file failed (error code: %d)\n", fd_key);
        return NULL;
    }
    struct pollfd pfd = {
        .fd = fd_key,
        .events = POLLIN,

    };

    while (1)
    {
        poll(&pfd, 1, -1);
        printf("Event occur \n");
        struct input_event ev = {0};
        read(fd_key, (void *)&ev, sizeof(ev));
        if (ev.type == EV_KEY && ev.code == KEY_1 && ev.value == 1)
        {
            pthread_mutex_lock(&lock);
            led1_blink = !led1_blink;
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&lock);
            printf("led 1 blink: %d \n", led1_blink);
        }

        if (ev.type == EV_KEY && ev.code == KEY_2 && ev.value == 1)
        {
            pthread_mutex_lock(&lock);
            led2_blink = !led2_blink;
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&lock);
            printf("led 2 blink: %d \n", led2_blink);
        }

    }
}
void *control_led(void *arg)
{
    (void)arg;

    int fd_led = open(DEV_PATH, O_RDWR);
    if (fd_led < 0) {
        perror("open(/dev/led_device0)");
        return NULL;
    }

    printf("[LED] Opened: %s\n", DEV_PATH);

    int led1_on = 0;
    int led2_on = 0;

    while (1) {
        pthread_mutex_lock(&lock);
        int b1 = led1_blink;
        int b2 = led2_blink;
        pthread_mutex_unlock(&lock);

        /* LED1 */
        if (b1) {
            do_ioctl_simple(fd_led, led1_on ? LED1_IOC_OFF : LED1_IOC_ON);
            led1_on = !led1_on;
        } else {
            if (led1_on) {
                do_ioctl_simple(fd_led, LED1_IOC_OFF);
                led1_on = 0;
            }
        }

        /* LED2 */
        if (b2) {
            do_ioctl_simple(fd_led, led2_on ? LED2_IOC_OFF : LED2_IOC_ON);
            led2_on = !led2_on;
        } else {
            if (led2_on) {
                do_ioctl_simple(fd_led, LED2_IOC_OFF);
                led2_on = 0;
            }
        }

        
        pthread_mutex_lock(&lock);
        if (!led1_blink && !led2_blink)
            pthread_cond_wait(&cond, &lock);
        pthread_mutex_unlock(&lock);

        usleep(1000 * 1000);   // 1000 ms = 1 giây
    }

    close(fd_led);
    return NULL;
}


int main(void)
{
    printf("START PROGRAM: LED CONTROL BY BUTTON DEMO \n");

    pthread_t t1, t2;

    pthread_create(&t1, NULL, control_led, NULL);
    pthread_create(&t2, NULL, read_button, NULL);

    /* Giữ chương trình sống */
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;

}
 