#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>

/**
 * This kernel module create a file in procfs. Also
 * this module demostrate how to use the sequence file
 * operations to display a list content using a virtual
 * file.
 *
 * Usage:
 * user can add the members details through the procfs file
 * in the order of name<space>place<space>age. Note that
 * none of the item shouldn't contain the space.
 * Eg: echo "Shafeeque Kerala 32" > /proc/names/list
 *
 * The data entered can be displayed by cat /proc/names/list
 * Eg: cat /proc/names/list
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robert W. Oliver II");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.01");

#define NAME_MAX_LEN    64
#define PLACE_MAX_LEN   64

typedef struct proc_dir_entry   proc_entry_t;
typedef struct name_node {
    struct name_node *next;
    struct name_node *prev;
    unsigned int num;
    char *name;
    char *place;
    int age;
} name_t;

static proc_entry_t *proc_dir;
static proc_entry_t *proc_name_list;
static name_t *head = NULL;

static ssize_t procfs_write(struct file *f, const char *buffer,
                                    size_t count, loff_t *off);
static ssize_t procfs_read(struct file *f, char *buffer,
                                    size_t count, loff_t *off);
static struct proc_ops ops = {
    .proc_read = procfs_read,
    .proc_write = procfs_write,
};

static ssize_t procfs_read(struct file *f, char *buffer,
                                    size_t count, loff_t *off)
{
    if(*off > 0) {
        // Run only once
        return 0;   // EOF
    }

    if(NULL == head) {
        pr_err("No entries found");
        return -ENOEXEC;
    }

    count = snprintf(buffer, count, "%32s - %32s - %4u\n",
            head->name, head->place, head->age);

    *off += count;

    return count;
}

static ssize_t procfs_write(struct file *f, const char *buffer,
                                    size_t count, loff_t *off)
{
    char name[NAME_MAX_LEN];
    char place[PLACE_MAX_LEN];
    unsigned int age;
    int ret, len;

    ret = sscanf(buffer, "%s %s %u", name, place, &age);
    if(ret < 3) {
        return -EINVAL;
    }

    if(head == NULL) {
        head = kzalloc(sizeof(name_t), GFP_KERNEL);
        if(NULL == head) {
            pr_err("Unable to allocate structure memory\n");
            return -ENOMEM;
        }

        len = strlen(name);
        head->name = kzalloc(len, GFP_KERNEL);
        if(NULL == head->name) {
            pr_err("Unable to allocate name memory\n");
            return -ENOMEM;
        }
        strncpy(head->name, name, len);

        len = strlen(place);
        head->place = kzalloc(len, GFP_KERNEL);
        if(NULL == head->place) {
            pr_err("Unable to allocate place memory\n");
            return -ENOMEM;
        }
        strncpy(head->place, place, len);

        head->age = age;
    }
    return count;
}

static int __init proc_start(void)
{
    proc_dir = proc_mkdir("names", NULL);
    if(NULL == proc_dir) {
        pr_err("Unable to create 'name' folder inside procfs\n");
        return -ENOMEM;
    }

    proc_name_list = proc_create("list", 0666, proc_dir, &ops);
    if(NULL == proc_name_list) {
        pr_err("Unable to create the 'list' file inside procfs\n");
        proc_remove(proc_dir);
        return -ENOMEM;
    }

    pr_info("proc_module: Loaded into the kernel\n");
    return 0;
}

static void __exit proc_end(void)
{
    proc_remove(proc_name_list);
    proc_remove(proc_dir);
    if(NULL != head) {
        if(head->name != NULL)
            kfree(head->name);

        if(head->place != NULL)
            kfree(head->place);

        kfree(head);
        head = NULL;
    }
    pr_info("proc_end: Goodbye Mr.\n");
}

module_init(proc_start);
module_exit(proc_end);
