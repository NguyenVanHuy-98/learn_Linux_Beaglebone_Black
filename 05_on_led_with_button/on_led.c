// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

#define DRV_NAME        "on_led_kernel"
#define DEV_NAME        "led_device0"
#define CLS_NAME        "on_led_kernel"

/* IOCTL definitions */
#define LED_IOC_MAGIC   'L'

#define LED1_IOC_ON     _IO(LED_IOC_MAGIC, 0)          /* Bật LED 1  */
#define LED1_IOC_OFF    _IO(LED_IOC_MAGIC, 1)          /* Tắt LED 1  */

/* Dùng NR=2 cho lệnh GET_MANY (struct + mảng) */
struct led_array {
    int count;          /* số LED */
    int __user *vals;   /* con trỏ tới mảng user-space */
};

#define LED_IOC_GET     _IOWR(LED_IOC_MAGIC, 2, struct led_array)

/* Nếu sau này muốn thêm:
#define LED2_IOC_ON     _IO(LED_IOC_MAGIC, 3)
#define LED2_IOC_OFF    _IO(LED_IOC_MAGIC, 4)
*/

#define LED2_IOC_ON     _IO(LED_IOC_MAGIC, 3)          /* Bật LED 2 */
#define LED2_IOC_OFF    _IO(LED_IOC_MAGIC, 4)          /* Tắt LED 2 */

static struct class *on_led_class;
static struct device *on_led_dev;
static struct gpio_desc *led1_gpiod;
static struct gpio_desc *led2_gpiod;

static dev_t led_devt;
static struct cdev led_cdev;

/* Trạng thái LED1 cho hàm read() */
static atomic_t led_state = ATOMIC_INIT(0); /* 0 = OFF, 1 = ON */

static void led1_on(void)
{
    if (led1_gpiod)
        gpiod_set_value_cansleep(led1_gpiod, 1);

    atomic_set(&led_state, 1);
    pr_info("ON LED DRIVER: LED 1 is ON\n");
}

static void led1_off(void)
{
    if (led1_gpiod)
        gpiod_set_value_cansleep(led1_gpiod, 0);

    atomic_set(&led_state, 0);
    pr_info("ON LED DRIVER: LED 1 is OFF\n");
}

static void led2_on(void)
{
    if (led2_gpiod)
        gpiod_set_value_cansleep(led2_gpiod, 1);

    pr_info("ON LED DRIVER: LED 2 is ON\n");
}

static void led2_off(void)
{
    if (led2_gpiod)
        gpiod_set_value_cansleep(led2_gpiod, 0);

    pr_info("ON LED DRIVER: LED 2 is OFF\n");
}

static ssize_t on_led_read(struct file *fp, char __user *buf,
                           size_t size, loff_t *ppos)
{
    char kbuf[8];
    int len;

    if (*ppos > 0)
        return 0;

    /* Đọc trạng thái logic từ led_state (LED1) */
    len = scnprintf(kbuf, sizeof(kbuf), "%d\n", atomic_read(&led_state));
    if (len > size)
        len = size;

    if (copy_to_user(buf, kbuf, len))
        return -EFAULT;

    *ppos += len;
    return len;
}

static ssize_t on_led_write(struct file *fp, const char __user *buf,
                            size_t size, loff_t *ppos)
{
    char kbuf[8];

    if (copy_from_user(kbuf, buf, min(size, sizeof(kbuf) - 1)))
        return -EFAULT;

    kbuf[min(size, sizeof(kbuf) - 1)] = '\0';

    if (sysfs_streq(kbuf, "on"))
        led1_on();
    else if (sysfs_streq(kbuf, "off"))
        led1_off();
    else
        return -EINVAL;

    return size;
}

static long on_led_unlocked_ioctl(struct file *fp,
                                  unsigned int cmd, unsigned long arg)
{
    switch (cmd) {

    case LED1_IOC_ON:
        led1_on();              /* Bật LED 1 */
        pr_info("IOCTL: LED1_ON\n");
        return 0;

    case LED1_IOC_OFF:
        led1_off();             /* Tắt LED 1 */
        pr_info("IOCTL: LED1_OFF\n");
        return 0;

    case LED2_IOC_ON:
        led2_on();              /* Bật LED 2 */
        pr_info("IOCTL: LED2_ON\n");
        return 0;

    case LED2_IOC_OFF:
        led2_off();             /* Tắt LED 2 */
        pr_info("IOCTL: LED2_OFF\n");
        return 0;

    case LED_IOC_GET: {
        struct led_array ua;
        int kbuf[8];            /* tối đa 8 LED (có thể tăng) */
        int i;

        /* Copy struct từ user vào kernel */
        if (copy_from_user(&ua, (void __user *)arg, sizeof(ua)))
            return -EFAULT;

        if (ua.count <= 0 || ua.count > 8)
            return -EINVAL;

        if (!ua.vals)
            return -EINVAL;

        /* Đọc từng LED bằng for */
        for (i = 0; i < ua.count; i++) {
            if (i == 0)
                kbuf[i] = gpiod_get_value_cansleep(led1_gpiod);
            else if (i == 1)
                kbuf[i] = gpiod_get_value_cansleep(led2_gpiod);
            else
                kbuf[i] = -1; /* nếu sau này chưa có LED3,4... */
        }

        /* Gửi mảng ngược lại user-space */
        if (copy_to_user(ua.vals, kbuf, ua.count * sizeof(int)))
            return -EFAULT;

        pr_info("IOCTL: LED_GET (count=%d)\n", ua.count);
        return 0;
    }

    default:
        pr_info("IOCTL: unsupported cmd=0x%x\n", cmd);
        return -ENOTTY;
    }
}

static const struct file_operations on_led_fops = {
    .owner          = THIS_MODULE,
    .read           = on_led_read,
    .write          = on_led_write,
    .unlocked_ioctl = on_led_unlocked_ioctl,
    .llseek         = no_llseek,
};

/* OF match table */
static const struct of_device_id on_led_of_match[] = {
    { .compatible = "led_on1" },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, on_led_of_match);

static int on_led_probe(struct platform_device *pdev)
{
    int ret;
    struct device_node *np = pdev->dev.of_node;

    if (!np) {
        dev_err(&pdev->dev, "No device tree node\n");
        return -EINVAL;
    }

    /* Lấy GPIO từ DT: gpio_led1 và gpio_led2 */
    led1_gpiod = gpiod_get_from_of_node(np, "gpio_led1", 0,
                                        GPIOD_OUT_LOW, "led1");
    if (IS_ERR(led1_gpiod)) {
        ret = PTR_ERR(led1_gpiod);
        dev_err(&pdev->dev, "Failed to get gpio_led1 (%d)\n", ret);
        return ret;
    }

    led2_gpiod = gpiod_get_from_of_node(np, "gpio_led2", 0,
                                        GPIOD_OUT_LOW, "led2");
    if (IS_ERR(led2_gpiod)) {
        ret = PTR_ERR(led2_gpiod);
        dev_err(&pdev->dev, "Failed to get gpio_led2 (%d)\n", ret);
        gpiod_put(led1_gpiod);
        return ret;
    }

    /* Character device region */
    ret = alloc_chrdev_region(&led_devt, 0, 1, DRV_NAME);
    if (ret) {
        dev_err(&pdev->dev, "alloc_chrdev_region failed\n");
        goto err_put_gpio;
    }

    cdev_init(&led_cdev, &on_led_fops);
    ret = cdev_add(&led_cdev, led_devt, 1);
    if (ret) {
        dev_err(&pdev->dev, "cdev_add failed\n");
        goto err_unreg_chrdev;
    }

    on_led_class = class_create(THIS_MODULE, CLS_NAME);
    if (IS_ERR(on_led_class)) {
        ret = PTR_ERR(on_led_class);
        dev_err(&pdev->dev, "class_create failed\n");
        on_led_class = NULL;
        goto err_cdev_del;
    }

    on_led_dev = device_create(on_led_class, NULL,
                               led_devt, NULL, DEV_NAME);
    if (IS_ERR(on_led_dev)) {
        ret = PTR_ERR(on_led_dev);
        dev_err(&pdev->dev, "device_create failed\n");
        on_led_dev = NULL;
        goto err_class_destroy;
    }

    /* Đảm bảo cả 2 LED OFF lúc đầu */
    led2_off();
    led1_off();

    dev_info(&pdev->dev, "on_led driver probed; /dev/%s ready\n", DEV_NAME);
    return 0;

err_class_destroy:
    class_destroy(on_led_class);
    on_led_class = NULL;

err_cdev_del:
    cdev_del(&led_cdev);

err_unreg_chrdev:
    unregister_chrdev_region(led_devt, 1);

err_put_gpio:
    if (led2_gpiod)
        gpiod_put(led2_gpiod);
    if (led1_gpiod)
        gpiod_put(led1_gpiod);

    return ret;
}

static int on_led_remove(struct platform_device *pdev)
{
    led2_off();
    led1_off();

    if (on_led_dev)
        device_destroy(on_led_class, led_devt);
    if (on_led_class)
        class_destroy(on_led_class);

    cdev_del(&led_cdev);
    unregister_chrdev_region(led_devt, 1);

    if (led1_gpiod)
        gpiod_put(led1_gpiod);

    if (led2_gpiod)
        gpiod_put(led2_gpiod);

    dev_info(&pdev->dev, "on_led driver removed\n");
    return 0;
}

static struct platform_driver on_led_driver = {
    .probe  = on_led_probe,
    .remove = on_led_remove,
    .driver = {
        .name           = DRV_NAME,
        .of_match_table = on_led_of_match,
    },
};

module_platform_driver(on_led_driver);

MODULE_AUTHOR("nguyen van huy");
MODULE_DESCRIPTION("Simple dual-LED char driver with ioctl");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
