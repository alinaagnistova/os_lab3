cmake_minimum_required(VERSION 3.30)
project(os_lab3 C)

set(CMAKE_C_STANDARD 11)

add_executable(os_lab3 main.c
        kernel/ioctl_custom.c
        user/main-user.c)
