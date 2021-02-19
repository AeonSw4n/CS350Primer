#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IOCTL_TEST _IOW(0, 6, struct ioctl_test_t)
#define KEYBOARD _IOR(1, 7, struct key_press)
#define REGISTER _IOW(0, 8, struct nothing)
#define DEREGISTER _IOW(0, 9, struct nothing)

// Used to get a char from the keyboard using IOCTL
struct key_press {
    char letter;
    char sent;
};

struct nothing {
    char imr;
};

void my_getchar(int fd, struct key_press key);

int main () {

    /* IOCTL_TEST STUFF
    // attribute structures
    struct ioctl_test_t {
      int field1;
      char field2;
    } ioctl_test;

    ioctl_test.field1 = 10;
    ioctl_test.field2 = 'a';

    ioctl (fd, IOCTL_TEST, &ioctl_test);
    */
    struct key_press key;
    key.sent = 0;
    int fd = open ("/proc/ioctl_test", O_RDONLY);
    struct nothing MegaRetard;
    sleep(1);
    //ioctl(fd, REGISTER, &MegaRetard);
    /* This loops until the system detects that the user has pressed the enter key */
    int done = 0;
    char letter;
    while (!done) {
        /* the character is both in key and letter (debugging stuff I dealt with) */
        my_getchar(fd, key);
        if(key.sent){
            key.sent = 0;
            letter = key.letter;
            /*
            if (letter == '\n') {
                printf("here??\n");
                //printf("\nEntered if statement\n");
                done = 1;
            }*/
            //printf("%c\n",letter);

            if ((int)letter == 127){
                char buff[] = "\b \b";
                write(1, buff, sizeof(buff)-1);
            } else {
                char s[3];
                s[0]=letter;
                s[1]=' ';
                s[2]='\b';
                write(1, s, 3);
            }
        }
        //char empty[] = " \b";
        //write(1, empty, 2);
        //printf("(%d-%c)\n", (int)letter, letter);
        //fflush(stdin);
    }
    //printf("Exiting");
    //fflush(stdin);
    //ioctl(fd, DEREGISTER, &MegaRetard);
    return 0;
}

void my_getchar(int fd, struct key_press key) {
    ioctl (fd, KEYBOARD, &key);
    //return key;
}
