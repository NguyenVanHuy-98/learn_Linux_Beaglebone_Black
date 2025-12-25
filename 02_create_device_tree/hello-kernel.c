#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/gpio.h>

#define GPIO_NUM 28

struct class * hello_kernel_class ;
struct device * test_device ;
    

void led_on(void)
{
    gpio_set_value(GPIO_NUM, 1);
    printk("HELLO KERNEL DRIVER:LED GPIO %d is ON\n", GPIO_NUM);
}

void led_off(void)
{
    gpio_set_value(GPIO_NUM, 0);
    printk("HELLO KERNEL DRIVER:LED GPIO %d is OFF\n", GPIO_NUM);
}
ssize_t test_show(struct device *dev, struct device_attribute *attr,char *buf)
{
    sprintf(buf, "hello\n");
    printk("HELLO KERNEL DRIVER: You just read /sys/class/hello-kernel/device0/test\n");
    return strlen(buf);

    return 0;
}

ssize_t test_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    // printk("HELLO KERNEL DRIVER: You just write [%s] to /sys/class/hello-kernel/device0/test\n", buf);

    // if (count >= 2 && strncmp(buf, "on", 2) == 0) {
    //     printk("HELLO KERNEL DRIVER:LED GPIO %d is ON\n", GPIO_NUM);
    //     gpio_set_value(GPIO_NUM, 1);
        
    //     led_on();  
    // } ;

    // if (strncmp(buf, "off", 3) == 0) {
    //     printk("HELLO KERNEL DRIVER:LED GPIO %d is OFF\n", GPIO_NUM);
    //     gpio_set_value(GPIO_NUM, 0);
        
    //     led_off();
    // } ;

    printk("HELLO KERNEL DRIVER: You just write [%.*s]\n", (int)count, buf);

    if (strncmp(buf, "on", 2) == 0) {
        led_on();   
    } else if (strncmp(buf, "off", 3) == 0) {
        led_off(); 
    } else {
        printk(KERN_WARNING "Invalid value. Use 'on' or 'off'\n");
    }

    return count ;
}

DEVICE_ATTR_RW(test);
int __init hello_init(void)
{

    printk("HELLO KERNEL DRIVER: Module loading\n");
    
    int rets = gpio_request(GPIO_NUM, "mygpio");
    
    gpio_direction_output(GPIO_NUM, 0);
    if (IS_ERR(rets)) {
        printk(" HELLO KERNEL DRIVER: Failed to request GPIO %d\n", GPIO_NUM);
        return -1;
    }
    
    hello_kernel_class = class_create (THIS_MODULE,"hello_kernel");
    if (IS_ERR(hello_kernel_class))
    {
        printk("HELLO KERNEL DRIVER: create hello kernel class failed \n");
        return -1;
    }
    test_device = device_create(hello_kernel_class,NULL,MKDEV(0,0),NULL,"device0") ;

    if (IS_ERR(test_device ))
    {
        printk("HELLO KERNEL DRIVER: create test device failed \n");
        return -1;
    }

    int ret = device_create_file(test_device, &dev_attr_test);
    if (ret) {
        printk( "HELLO KERNEL DRIVER: Failed to create test file\n");
        device_destroy(hello_kernel_class, MKDEV(0, 0));
        class_destroy(hello_kernel_class);
    }
    printk("HELLO KERNEL DRIVER: Module loaded\n");

    return 0;
}

void __exit hello_exit(void)
{
    printk("Hello Kernel Drive: this module exited \n");
    printk( "HELLO KERNEL DRIVER: this module exited\n");

    device_remove_file(test_device, &dev_attr_test);
    device_destroy(hello_kernel_class, MKDEV(0, 0));
    class_destroy(hello_kernel_class);
    gpio_free(GPIO_NUM);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("nguyen van huy");
MODULE_DESCRIPTION("this is simple character driver");
MODULE_VERSION("1.0");

module_init(hello_init);
module_exit(hello_exit);