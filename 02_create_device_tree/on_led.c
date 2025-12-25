#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/gpio.h>


#define ON_LED_DTB_PATH "/on_led"

struct class * on_led_class ;
struct device * led_device ;
static struct gpio_desc *g;

void led_on(void)
{
    gpiod_set_value_cansleep(g, 1);
    printk("ON LED DRIVER:LED GPIO 28 is ON\n");
    
}

void led_off(void)
{
    gpiod_set_value_cansleep(g, 0);
    printk("ON LED DRIVER:LED GPIO 28 is OFF\n");
}
ssize_t on_led_show(struct device *dev, struct device_attribute *attr,char *buf)
{
    sprintf(buf, "on led\n");
    printk("ON LED DRIVER: You just read /sys/class/hello-kernel/device0/test\n");
    return strlen(buf);
    
    return 0;
}

ssize_t on_led_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{

    printk("ON LED DRIVER: You just write [%.*s]\n", (int)count, buf);

    if (strncmp(buf, "on", 2) == 0) {
        led_on();   
    } else if (strncmp(buf, "off", 3) == 0) {
        led_off(); 
    } else {
        printk("ON LED DRIVER: Invalid value. Use 'on' or 'off'\n");
    }

    return count ;
}

DEVICE_ATTR_RW(on_led);
int __init on_led_init(void)
{
    struct device_node *np;

    printk("ON LED DRIVER: Module loading\n");

    np = of_find_node_by_path(ON_LED_DTB_PATH);
    if (!np) {
        printk("ON LED DRIVER: Device tree node not found: %s\n", ON_LED_DTB_PATH);
        return -ENODEV;
    }
    

    g = gpiod_get_from_of_node(np, "gpios", 0, GPIOD_OUT_LOW, NULL);
    of_node_put(np);

    if (IS_ERR(g)) {
        pr_err("ON LED  DRIVER: gpiod_get_from_of_node failed: %ld\n", PTR_ERR(g));
        return PTR_ERR(g);
    }

    gpiod_set_value_cansleep(g, 1);
    
    on_led_class = class_create (THIS_MODULE,"on_led_kernel");
    if (IS_ERR( on_led_class))
    {
        printk("ON LED DRIVER: create ON LED class failed \n");
        return -1;
    }

    led_device = device_create(on_led_class,NULL,MKDEV(0,0),NULL,"led_device0") ;

    if (IS_ERR(led_device ))
    {
        printk("ON LED DRIVER: create led device failed \n");
        return -1;
        
    }

    int ret = device_create_file(led_device, &dev_attr_on_led);
    if (ret) {
        printk( "ON LED DRIVER: Failed to create led file\n");
        device_destroy(on_led_class, MKDEV(0,0));
        class_destroy(on_led_class);
    }
    printk("ON LED DRIVER: Module loaded\n");

    return 0;
}

void __exit on_led_exit(void)
{
    printk("ON LED Drive: this module exited \n");
    printk( "ON LED DRIVER: this module exited\n");

    device_remove_file(led_device, &dev_attr_on_led);
    device_destroy(on_led_class , MKDEV(0,0));
    class_destroy(on_led_class);
   
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("nguyen van huy");
MODULE_DESCRIPTION("this is simple character driver");
MODULE_VERSION("1.0");

module_init(on_led_init);
module_exit(on_led_exit);