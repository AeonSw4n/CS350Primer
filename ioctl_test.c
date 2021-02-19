#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IOCTL_TEST _IOW(0, 6, struct ioctl_test_t)
#define KEYBOARD _IOR(1, 7, struct key_struct)
#define REGISTER _IOW(0, 8, struct register_struct)
#define DEREGISTER _IOW(0, 9, struct register_struct)

// Used to get a char from the keyboard using IOCTL
struct key_struct {
    char c;
    char sent;
};

struct register_struct {
    char useless;
};

void get_key(int fd, struct key_struct *key);

int main () {
    char loading[] = "Loading... ";
    write(1, loading, strlen(loading));
    sleep(1);
    struct key_struct key;
    key.sent = 0;
    int fd = open ("/proc/ioctl_test", O_RDONLY);
    struct register_struct MegaRetard;
    ioctl(fd, REGISTER, &MegaRetard);
    printf("Done!\n");
    int done = 0;
    char c;
    while (!done) {
        get_key(fd, &key);
        if(key.sent){
            key.sent = 0;
            c = key.c;
            if ((int)c == 127){
                char buff[] = "\b \b";
                write(1, buff, sizeof(buff)-1);
            } else {
                char s[3];
                s[0]=c;
                s[1]=' ';
                s[2]='\b';
                write(1, s, 3);
            }
            if((int)c == '\\'){
                done = 1;
            }
        }

    }
    printf("\n");
    fflush(stdin);
    ioctl(fd, DEREGISTER, &MegaRetard);
    return 0;
}

void get_key(int fd, struct key_struct *key) {
    ioctl (fd, KEYBOARD, key);
}