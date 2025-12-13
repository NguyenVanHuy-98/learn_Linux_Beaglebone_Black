#include <stdio.h>
#include <linux/gpio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <poll.h>
#include <linux/input.h>
#include <pthread.h>

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int led1_blink = 1;
int led2_blink = 1;

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
            printf("led blink: %d \n", led1_blink);
        }

        if (ev.type == EV_KEY && ev.code == KEY_2 && ev.value == 1)
        {
            pthread_mutex_lock(&lock);
            led2_blink = !led2_blink;
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&lock);
            printf("led blink: %d \n", led1_blink);
        }

    }
}

void *control_led(void *arg)
{
    int fd_led = open("/dev/led_device0", O_RDWR);
    if (fd_led < 0)
    {
        printf("Open file failed (error code: %d)\n", fd_led);
        return NULL;
    }
    while (1)
    {
        
        while (!led1_blink)
        {
            write(fd_led, (void *)"on", 3);
            sleep(1);
            write(fd_led, (void *)"off", 4);
            sleep(1);
        }

        while (!led2_blink)
        {
            write(fd_led, (void *)"on", 3);
            sleep(1);
            write(fd_led, (void *)"off", 4);
            sleep(1);
        }

        pthread_mutex_lock(&lock);
        pthread_cond_wait(&cond, &lock);
        pthread_mutex_unlock(&lock);
        printf(".");
        sleep(1);

    }
}

int main()
{
    int res = 0;
    printf("START PROGRAM: LED CONTROL BY BUTTON DEMO \n");

    pthread_t t1, t2;
    int id1;
    pthread_create(&t1, NULL, control_led, NULL);
    pthread_create(&t2, NULL, read_button, NULL);
    //pthread_join(t1, NULL);
    //pthread_join(t2, NULL);

    return res;
}
 