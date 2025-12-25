#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define ON_LED_DTB_PATH "/on_led"

static struct class *on_led_class;
static struct device *led_device;
static struct gpio_desc *g;
static dev_t c_dev;
static struct cdev dev;

void led_on(void)
{
    gpiod_set_value_cansleep(g, 1);
    printk("ON LED DRIVER: LED GPIO 28 is ON\n");
}

void led_off(void)
{
    gpiod_set_value_cansleep(g, 0);
    printk("ON LED DRIVER: LED GPIO 28 is OFF\n");
}

ssize_t on_led_read(struct file *fp, char __user *buf, size_t size, loff_t *off_set)
{
    printk("ON LED DRIVER: File read\n");
    return 0;
}

ssize_t on_led_write(struct file *fp, const char __user *buf, size_t size, loff_t *off_set)
{
    printk("ON LED DRIVER: File write\n");
    return 0;
}

long on_led_unlocked_ioctl(struct file *fp, unsigned int cmd, unsigned long data)
{
    printk("ON LED DRIVER: ioctl call, cmd %d, data 0x%.8lx \n", cmd, data);
    return 0;
}

const struct file_operations on_led_op = {
    .owner = THIS_MODULE,
    .read = on_led_read,
    .write = on_led_write,
    .unlocked_ioctl = on_led_unlocked_ioctl
};

int on_led_driver_in(struct platform_device *pdev)
{
    int ret;

    ret = alloc_chrdev_region(&c_dev, 0, 1, "on_led_driver");
    if (ret < 0) {
        printk("ON LED DRIVER: Create character device failed\n");
        return ret;
    }
    printk("ON LED DRIVER: Create character device done\n");

    cdev_init(&dev, &on_led_op);
    ret = cdev_add(&dev, c_dev, 1);
    if (ret < 0) {
        printk("ON LED DRIVER: Add character device failed\n");
        return ret;
    }
    printk("ON LED DRIVER: Add character device done\n");

    on_led_class = class_create(THIS_MODULE, "on_led_kernel");
    if (IS_ERR(on_led_class)) {
        printk("ON LED DRIVER: Create ON LED class failed\n");
        return PTR_ERR(on_led_class);
    }

    led_device = device_create(on_led_class, NULL, c_dev, NULL, "led_device0");
    if (IS_ERR(led_device)) {
        printk("ON LED DRIVER: Create led device failed\n");
        return PTR_ERR(led_device);
    }

    g = gpiod_get(&pdev->dev, "led_gpio", GPIOD_OUT_LOW);
    if (IS_ERR(g)) {
        printk("ON LED DRIVER: Failed to get GPIO\n");
        return PTR_ERR(g);
    }

    printk("ON LED DRIVER: GPIO configured\n");

    return 0;
}

int on_led_driver_ex(struct platform_device *pdev)
{
    printk("ON LED DRIVER: Module exited\n");

    device_destroy(on_led_class, c_dev);
    class_destroy(on_led_class);
    gpiod_put(g);

    return 0;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("nguyen van huy");
MODULE_DESCRIPTION("This is a simple character driver");
MODULE_VERSION("1.0");

struct of_device_id on_led_of_device[] = {
    {.compatible = "on-led_compatible"},
    {}
};

static struct platform_driver on_led_driver = {
    .probe = on_led_driver_in,
    .remove = on_led_driver_ex,
    .driver = {
        .name = "on_led_kernel",
        .of_match_table = on_led_of_device,
    },
};

module_platform_driver(on_led_driver);
