#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/kdev_t.h>


struct class * hello_kernel_class ;
struct device * test_device ;

ssize_t test_show(struct device *dev, struct device_attribute *attr,char *buf)
{
    sprintf(buf, "hello\n");
    printk("HELLO KERNEL DRIVER: You just read /sys/class/hello-kernel/device0/test\n");
    return strlen(buf);

    return 0;
}

ssize_t test_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    printk("HELLO KERNEL DRIVER: You just write [%s] to /sys/class/hello-kernel/device0/test\n", buf);
    return count ;
}

DEVICE_ATTR_RW(test);
int __init hello_init(void)
{
    
    printk("HELLO KERNEL DRIVER: this module loaded \n");
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

    return 0;
}

void __exit hello_exit(void)
{
    printk("Hello Kernel Drive: this module exited \n");
    printk( "HELLO KERNEL DRIVER: this module exited\n");

    device_remove_file(test_device, &dev_attr_test);
    device_destroy(hello_kernel_class, MKDEV(0, 0));
    class_destroy(hello_kernel_class);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("nguyen van huy");
MODULE_DESCRIPTION("this is simple character driver");
MODULE_VERSION("1.0");

module_init(hello_init);
module_exit(hello_exit);