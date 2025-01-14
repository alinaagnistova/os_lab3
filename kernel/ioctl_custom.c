#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/sched/types.h>
#include <linux/sched/task.h>
#include <linux/init.h>
#include <linux/dcache.h>
#include <linux/path.h>
#include <linux/fs_struct.h>
#include <linux/sched/cputime.h>

#define DEVICE_NAME "task_info"
#define IOCTL_GET_TASK_CPUTIME  _IOWR('t', 1, struct param_task_cputime)
#define IOCTL_GET_INODE_INFO    _IOWR('t', 2, struct param_inode_info)

struct param_task_cputime {
    pid_t pid;
    struct task_cputime cputime;
};


struct param_inode_info {
    pid_t pid;
    unsigned long i_ino;
    loff_t i_size;
};

static int device_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device %s opened\n", DEVICE_NAME);
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device %s closed\n", DEVICE_NAME);
    return 0;
}

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct task_struct *task = NULL;

    switch (cmd) {

    case IOCTL_GET_TASK_CPUTIME:
    {
        struct param_task_cputime param;
        if (copy_from_user(&param, (void __user *)arg, sizeof(param))) {
            return -EFAULT;
        }

        task = get_pid_task(find_get_pid(param.pid), PIDTYPE_PID);
        if (!task) {
            printk(KERN_ERR "IOCTL_GET_TASK_CPUTIME: Task %d not found\n", param.pid);
            return -ESRCH;
        }

        {
            struct task_cputime cputime = {0};
            task_cputime_adjusted(task, &cputime.utime, &cputime.stime);
            cputime.sum_exec_runtime = task->se.sum_exec_runtime;

            param.cputime = cputime;
        }

        put_task_struct(task);
        if (copy_to_user((void __user *)arg, &param, sizeof(param))) {
            return -EFAULT;
        }
        break;
    }

    case IOCTL_GET_INODE_INFO:
    {
        struct param_inode_info param;

        if (copy_from_user(&param, (void __user *)arg, sizeof(param))) {
            return -EFAULT;
        }

        task = get_pid_task(find_get_pid(param.pid), PIDTYPE_PID);
        if (!task) {
            printk(KERN_ERR "IOCTL_GET_INODE_INFO: Task %d not found\n", param.pid);
            return -ESRCH;
        }


        {
            struct path pwd;
            if (!task->fs) {
                printk(KERN_ERR "IOCTL_GET_INODE_INFO: Task %d has no fs struct\n", param.pid);
                put_task_struct(task);
                return -EINVAL;
            }

            get_fs_pwd(task->fs, &pwd);

            if (!pwd.dentry || !pwd.dentry->d_inode) {
                printk(KERN_ERR "IOCTL_GET_INODE_INFO: Invalid dentry for Task %d\n", param.pid);
                path_put(&pwd);
                put_task_struct(task);
                return -EINVAL;
            }

            param.i_ino  = pwd.dentry->d_inode->i_ino;
            param.i_size = pwd.dentry->d_inode->i_size;

            path_put(&pwd);
        }

        put_task_struct(task);

        if (copy_to_user((void __user *)arg, &param, sizeof(param))) {
            return -EFAULT;
        }
        break;
    }

        default:
            return -ENOTTY;
    }

    return 0;
}

static struct file_operations fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = device_ioctl,
    .open           = device_open,
    .release        = device_release,
};

static int __init ioctl_custom_init(void) {
    int major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ERR "Failed to register device '%s'\n", DEVICE_NAME);
        return major;
    }
    printk(KERN_INFO "Module '%s' loaded with device major number %d\n",
           DEVICE_NAME, major);
    return 0;
}

static void __exit ioctl_custom_exit(void) {
    unregister_chrdev(0, DEVICE_NAME);
    printk(KERN_INFO "Module '%s' unloaded\n", DEVICE_NAME);
}

module_init(ioctl_custom_init);
module_exit(ioctl_custom_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");
MODULE_DESCRIPTION("Custom IOCTL kernel module");
