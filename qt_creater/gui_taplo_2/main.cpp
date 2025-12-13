#include "mainwindow.h"

#include <QApplication>

#include <QApplication>
#include <QDebug>
#include <stdio.h>
#include <linux/gpio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <poll.h>
#include <linux/input.h>
#include <pthread.h>



MainWindow* MainWindow::instance = nullptr;

int led_blink = 1;

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
            led_blink = !led_blink;
            printf("led blink: %d \n", led_blink);
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

        while (!led_blink)
        {

            QMetaObject::invokeMethod(MainWindow::instance,
                                      "setLightLeftVisible",
                                      Qt::QueuedConnection,
                                      Q_ARG(bool, true));

            write(fd_led, (void *)"on", 3);

            sleep(1);

            QMetaObject::invokeMethod(MainWindow::instance,
                                      "setLightLeftVisible",
                                      Qt::QueuedConnection,
                                      Q_ARG(bool, false));

            write(fd_led, (void *)"off", 4);

            sleep(1);

        }
        printf(".");
        sleep(1);

    }
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();


    pthread_t t1, t2;

    pthread_create(&t1, NULL, control_led, NULL);
    pthread_create(&t2, NULL, read_button, NULL);
    a.exec();
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);


    return 0 ;
}
