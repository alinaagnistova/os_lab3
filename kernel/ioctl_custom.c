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

#define DEVICE_NAME "task_info"

/*
 * Определяем собственные команды IOCTL
 */
#define IOCTL_GET_TASK_CPUTIME _IOWR('t', 1, struct param_task_cputime)
#define IOCTL_GET_INODE_INFO   _IOWR('t', 2, struct param_inode_info)

struct my_task_cputime {
    __u64 utime;
    __u64 stime;
    __u64 sum_exec_runtime;
};

struct inode_info {
    unsigned long inode_number;
    unsigned long size;
};


struct param_task_cputime {
    pid_t pid;
    struct my_task_cputime t_cputime;
};

struct param_inode_info {
    pid_t pid;
    struct inode_info i_info;
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

        case IOCTL_GET_TASK_CPUTIME: {
            struct param_task_cputime param;
            if (copy_from_user(&param,
                               (struct param_task_cputime __user *)arg,
                               sizeof(param))) {
                return -EFAULT;
            }
            task = get_pid_task(find_get_pid(param.pid), PIDTYPE_PID);
            if (!task) {
                printk(KERN_ERR "IOCTL_GET_TASK_CPUTIME: Task %d not found\n",
                       param.pid);
                return -ESRCH;
            }
            param.t_cputime.utime = task->utime;
            param.t_cputime.stime = task->stime;
            param.t_cputime.sum_exec_runtime = task->se.sum_exec_runtime;

            put_task_struct(task);

            if (copy_to_user((struct param_task_cputime __user *)arg,
                             &param,
                             sizeof(param))) {
                return -EFAULT;
            }
            break;
        }

        case IOCTL_GET_INODE_INFO: {
            struct param_inode_info param;
            if (copy_from_user(&param,
                               (struct param_inode_info __user *)arg,
                               sizeof(param))) {
                return -EFAULT;
            }

            task = get_pid_task(find_get_pid(param.pid), PIDTYPE_PID);
            if (!task) {
                printk(KERN_ERR "IOCTL_GET_INODE_INFO: Task %d not found\n",
                       param.pid);
                return -ESRCH;
            }

            {
                struct path pwd;
                get_fs_pwd(task->fs, &pwd);
                param.i_info.inode_number = pwd.dentry->d_inode->i_ino;
                param.i_info.size = pwd.dentry->d_inode->i_size;
                path_put(&pwd);
            }

            put_task_struct(task);

            if (copy_to_user((struct param_inode_info __user *)arg,
                             &param,
                             sizeof(param))) {
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

/*
 * Инициализация и выгрузка модуля
 */
static int __init ioctl_custom_init(void) {
    int ret;
    ret = register_chrdev(0, DEVICE_NAME, &fops);
    if (ret < 0) {
        printk(KERN_ERR "Failed to register device '%s'\n", DEVICE_NAME);
        return ret;
    }
    printk(KERN_INFO "Module '%s' loaded with device major number %d\n",
           DEVICE_NAME, ret);
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