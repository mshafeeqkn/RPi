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
#include <linux/seq_file.h>

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
 *
 * The list can be cleared by writing an empty string to the
 * same file
 * echo "" > /proc/names/list
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robert W. Oliver II");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.01");

#define NAME_MAX_LEN    64
#define PLACE_MAX_LEN   64

typedef struct proc_dir_entry   proc_entry_t;
typedef struct name_node {
    struct list_head list;
    unsigned int num;
    char *name;
    char *place;
    int age;
} name_t;

static proc_entry_t *proc_dir;
static proc_entry_t *proc_name_list;
static LIST_HEAD(name_list);


static ssize_t procfs_write(struct file *f, const char *buffer,
                                    size_t count, loff_t *off);
static int      procfs_open(struct inode *, struct file *);

static const struct proc_ops ops = {
    .proc_open = procfs_open,
    .proc_read = seq_read,
    .proc_write = procfs_write,
};

static void* name_seq_start(struct seq_file *m, loff_t *pos);
static void  name_seq_stop(struct seq_file *m, void *v) {}
static void* name_seq_next(struct seq_file *m, void *v, loff_t *pos);
static int   name_seq_show(struct seq_file *m, void *v);

static const struct seq_operations seq_ops = {
    .start = name_seq_start,
    .stop  = name_seq_stop,
    .next  = name_seq_next,
    .show  = name_seq_show,
};

static void clear_list(void)
{
    name_t *node, *tmp;

    list_for_each_entry_safe(node, tmp, &name_list, list) {
        list_del(&node->list);
        if(node->name != NULL)
            kfree(node->name);

        if(node->place != NULL)
            kfree(node->place);
        kfree(node);
    }
}

static int procfs_open(struct inode *node, struct file *f)
{
    return seq_open(f, &seq_ops);
}

void *name_seq_start(struct seq_file *f, loff_t *pos)
{
    name_t *node;
    loff_t i = 0;

    list_for_each_entry(node, &name_list, list) {
        if(i++ == *pos) {
            return node;
        }
    }

    return NULL;
}

static void* name_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
    name_t *node = (name_t *)v;

    (*pos)++;
    if (list_is_last(&node->list, &name_list)) {
        return NULL;
    }

    return list_next_entry(node, list);
}

static int   name_seq_show(struct seq_file *m, void *v)
{
    name_t *node = (name_t *)v;
    seq_printf(m, "%s\t%s\t%d\n", node->name, node->place, node->age);
    return 0;
}

static ssize_t procfs_write(struct file *f, const char *buffer,
                                    size_t count, loff_t *off)
{
    char name[NAME_MAX_LEN];
    char place[PLACE_MAX_LEN];
    unsigned int age;
    int ret, len;
    name_t *name_node;

    ret = sscanf(buffer, "%s %s %u", name, place, &age);
    if(ret == 1) {
        clear_list();
        return count;
    } else if(ret < 3) {
        return -EINVAL;
    }

    name_node = kzalloc(sizeof(name_t), GFP_KERNEL);
    if(NULL == name_node) {
        pr_err("Unable to allocate structure memory\n");
        return -ENOMEM;
    }

    len = strlen(name);
    name_node->name = kzalloc(len + 1, GFP_KERNEL);
    if(NULL == name_node->name) {
        pr_err("Unable to allocate name memory\n");
        return -ENOMEM;
    }
    strncpy(name_node->name, name, len);

    len = strlen(place);
    name_node->place = kzalloc(len + 1, GFP_KERNEL);
    if(NULL == name_node->place) {
        pr_err("Unable to allocate place memory\n");
        return -ENOMEM;
    }
    strncpy(name_node->place, place, len);

    name_node->age = age;
    list_add_tail(&name_node->list, &name_list);

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
    clear_list();
    pr_info("proc_end: Goodbye Mr.\n");
}

module_init(proc_start);
module_exit(proc_end);
