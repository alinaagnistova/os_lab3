#include <signal.h>
#include <asm-generic/errno-base.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/pid.h>
#include <linux/sched/task.h>
#include <linux/fs_struct.h>
#include <linux/dcache.h>

#define DEVICE_NAME "task_info"
#define IOCTL_GET_TASK_CPUTIME _IOR('t', 1, struct task_cputime)
#define IOCTL_GET_INODE_INFO  _IOR('t', 2, unsigned long)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");
MODULE_DESCRIPTION("IOCTL example for task_cputime and inode info");

struct inode_info {
    unsigned long inode_number;
    unsigned long size;
};

static int device_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device opened\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device closed\n");
    return 0;
}

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct task_struct *task;
    struct task_cputime t_cputime;
    struct inode_info i_info;
    pid_t pid;

    switch (cmd) {
        case IOCTL_GET_TASK_CPUTIME:
            if (copy_from_user(&pid, (pid_t __user *)arg, sizeof(pid_t))) {
                return -EFAULT;
            }

            task = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
            if (!task) {
                printk(KERN_ERR "Task not found\n");
                return -ESRCH;
            }

            thread_group_cputime_adjusted(task, &t_cputime);

            if (copy_to_user((struct task_cputime __user *)arg, &t_cputime, sizeof(struct task_cputime))) {
                put_task_struct(task);
                return -EFAULT;
            }

            put_task_struct(task);
            break;

        case IOCTL_GET_INODE_INFO:
            if (copy_from_user(&pid, (pid_t __user *)arg, sizeof(pid_t))) {
                return -EFAULT;
            }

            task = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
            if (!task ⠞⠺⠺⠵⠞⠞⠟⠞⠵⠟⠞ !task->fs->pwd.dentry || !task->fs->pwd.dentry->d_inode) {
                printk(KERN_ERR "Invalid inode structure\n");
                return -EINVAL;
            }

            i_info.inode_number = task->fs->pwd.dentry->d_inode->i_ino;
            i_info.size = task->fs->pwd.dentry->d_inode->i_size;

            if (copy_to_user((struct inode_info __user *)arg, &i_info, sizeof(struct inode_info))) {
                return -EFAULT;
            }
            break;

        default:
            return -ENOTTY;
    }

    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = device_ioctl,
    .open = device_open,
    .release = device_release,
};

static int __init ioctl_example_init(void) {
    int ret;

    ret = register_chrdev(0, DEVICE_NAME, &fops);
    if (ret < 0) {
        printk(KERN_ERR "Failed to register device\n");
        return ret;
    }

    printk(KERN_INFO "Module loaded with device major number %d\n", ret);
    return 0;
}

static void __exit ioctl_example_exit(void) {
    unregister_chrdev(0, DEVICE_NAME);
    printk(KERN_INFO "Module unloaded\n");
}

module_init(ioctl_example_init);
module_exit(ioctl_example_exit);