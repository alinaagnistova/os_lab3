#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>

#define DEVICE_PATH "/dev/task_info"

struct my_task_cputime {
    __u64 utime;
    __u64 stime;
    __u64 sum_exec_runtime;
};

struct inode_info {
    unsigned long inode_number;
    unsigned long size;
};

#define IOCTL_GET_TASK_CPUTIME  _IOWR('t', 1, struct param_task_cputime)
#define IOCTL_GET_INODE_INFO    _IOWR('t', 2, struct param_inode_info)

struct param_task_cputime {
    pid_t pid;
    struct my_task_cputime t_cputime;
};

struct param_inode_info {
    pid_t pid;
    struct inode_info i_info;
};

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <PID> <mode (1=cputime, 2=inode)>\n", argv[0]);
        return EXIT_FAILURE;
    }

    pid_t pid = atoi(argv[1]);
    int mode  = atoi(argv[2]);

    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    if (mode == 1) {
        struct param_task_cputime param;
        param.pid = pid;
        if (ioctl(fd, IOCTL_GET_TASK_CPUTIME, &param) == -1) {
            perror("IOCTL_GET_TASK_CPUTIME failed");
            close(fd);
            return EXIT_FAILURE;
        }
        printf("User time:            %llu\n", (unsigned long long)param.t_cputime.utime);
        printf("System time:          %llu\n", (unsigned long long)param.t_cputime.stime);
        printf("Sum execution runtime:%llu\n", (unsigned long long)param.t_cputime.sum_exec_runtime);

    } else if (mode == 2) {
        struct param_inode_info param;
        param.pid = pid;

        if (ioctl(fd, IOCTL_GET_INODE_INFO, &param) == -1) {
            perror("IOCTL_GET_INODE_INFO failed");
            close(fd);
            return EXIT_FAILURE;
        }

        printf("Inode number: %lu\n", param.i_info.inode_number);
        printf("Size:         %lu\n", param.i_info.size);

    } else {
        fprintf(stderr, "Invalid mode. Use 1 for cputime, 2 for inode.\n");
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);
    return EXIT_SUCCESS;
}