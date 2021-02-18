make -C /usr/src/linux-2.6.33.2/ SUBDIRS=$PWD modules
gcc ioctl_test.c -o ioctl_test.o