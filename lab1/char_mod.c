#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/errno.h>

#define MAJOR_N 0
#define MINOR_N 1
#define DEV_REG_NAME "my-chardev"
#define DEV_NAME "var1"
#define CLASS_NAME "my-chd-class"

static struct class *cl;
static struct cdev c_dev;
static dev_t first;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Polina");
MODULE_DESCRIPTION("io-lab1 char-dev-module");


static int my_open(struct inode *i, struct file *f)
{
    pr_info("Driver: open()\n");
    return 0;
}

static int my_close(struct inode *i, struct file *f)
{
    pr_info("Driver: close()\n");
    return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    pr_info("Driver: read()\n");
    return 0;
}

static ssize_t my_write(struct file *f, const char __user *buf,  size_t len, loff_t *off)
{
    pr_info("Driver: write()\n");
    return len;
}

static int my_dev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0660);
    return 0;
}

static struct file_operations fops =
        {
                .owner      = THIS_MODULE,
                .open       = my_open,
                .release    = my_close,
                .read       = my_read,
                .write      = my_write
        };


static int __init char_mod_init(void)
{
    int err;
    pr_alert("init module registration\n");
    if ( (err = alloc_chrdev_region(&first, MAJOR_N, MINOR_N, DEV_REG_NAME)) < 0 )
    {
        return err;
    }
    pr_info("<major, minor>: <%d, %d>\n", MAJOR(first), MINOR(first));

    if ( IS_ERR(cl = class_create(THIS_MODULE, CLASS_NAME)) )
    {
        unregister_chrdev_region(first, MINOR_N);
        return PTR_ERR(cl);
    }

    cl->dev_uevent = my_dev_uevent;

    struct device *dev_err;
    if ( IS_ERR(dev_err = device_create(cl, NULL, first, NULL, DEV_NAME)) )
    {
        class_destroy(cl);
        unregister_chrdev_region(first, MINOR_N);
        return PTR_ERR(dev_err);
    }

    cdev_init(&c_dev, &fops);
    if ( (err = cdev_add(&c_dev, first, MINOR_N)) < 0)
    {
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, MINOR_N);
        return err;
    }
    return 0;
}

static void __exit char_mod_exit(void)
{
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, MINOR_N);
    pr_alert("char driver unregistered\n");
}

module_init(char_mod_init);
module_exit(char_mod_exit);

