#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
//#include <asm/io.h>
#include <linux/unistd.h>
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/tty.h>
#include <linux/delay.h>
#include <linux/i8042.h>
#include <linux/kallsyms.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");

struct key_struct {
    char c;
    char sent;
} key;

struct register_struct {
    char useless;
};

#define IOCTL_TEST _IOW(0, 6, struct ioctl_test_t)
#define KEYBOARD _IOR(1, 7, struct key_struct)
#define REGISTER _IOW(0, 8, struct register_struct)
#define DEREGISTER _IOW(0, 9, struct register_struct)

static int pseudo_device_ioctl(struct inode *inode, struct file *file,
                               unsigned int cmd, unsigned long arg);

static struct file_operations pseudo_dev_proc_operations;

static struct proc_dir_entry *proc_entry;


static char character;
static char enabled;
static char shift;

// Inline assembly Functions
static inline unsigned char inb( unsigned short usPort ) {

    unsigned char uch;

    asm volatile( "inb %1,%0" : "=a" (uch) : "Nd" (usPort) );
    return uch;
}

static inline void outb( unsigned char uch, unsigned short usPort ) {

    asm volatile( "outb %0,%1" : : "a" (uch), "Nd" (usPort) );
}

/* 'printk' version that prints to active tty. */
void my_printk(char *string)
{
    struct tty_struct *my_tty;

    my_tty = current->signal->tty;
    //printk("%s %d\n", string, strlen(string));
    if (my_tty != NULL) {
        (*my_tty->driver->ops->write)(my_tty, string, strlen(string));
        (*my_tty->driver->ops->write)(my_tty, "\015\012", 2);
    }
}

char toUpper (char c){
    if(shift){
        if(c >= 'a' && c<='z')
            return (char)(c-32);
        switch(c){
            case '0':
                return ')';
            case '1':
                return '!';
            case '2':
                return '@';
            case '3':
                return '#';
            case '4':
                return '$';
            case '5':
                return '%';
            case '6':
                return '^';
            case '7':
                return '&';
            case '8':
                return '*';
            case '9':
                return '(';
        }
    }
    return c;
}

// Character retrieval method
char my_getchar ( void ) {
    char c;
    unsigned char status;
    static char scancode[128] = "\0\e1234567890-=\177\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000789-456+1230.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";


    c = inb( 0x60 );
    if(!( c & 0x80 )){
        if(c == 42)
            shift = 1;
        //printk("[%d %d]\n", c, shift);
        return toUpper(scancode[ (int)c ]);
    }
    else if (c & 0x80){
        c &= 0x7f;
        if(c == 42)
            shift = 0;
        //printk("(- %d %d)\n", c, shift);
        return 0;
    }
    return 0;
}




irqreturn_t irq_handler(int irq, void *dev_id, struct pt_regs *regs){
    static int init = 0;
    if (init == 0){
        init = 1;
    } else {
        char c = my_getchar();
        if(c){
            key.c = c;
            character = c;
            enabled = 1;
        }
    }

    return IRQ_HANDLED;
}

static bool i8042_filter(unsigned char data, unsigned char str, struct serio *port){
    return true;
}

// Init Routine
static int __init initialization_routine(void) {
    printk("<1> Loading module\n");

    pseudo_dev_proc_operations.ioctl = pseudo_device_ioctl;
    shift = 0;
    /* Start create proc entry */
    proc_entry = create_proc_entry("ioctl_test", 0444, NULL);
    if(!proc_entry)
    {
        printk("<1> Error creating /proc entry.\n");
        return 1;
    }
    character = -1;
    enabled = 0;
    proc_entry->proc_fops = &pseudo_dev_proc_operations;
    my_workqueue = create_singlethread_workqueue("0BrainCellsQueue");
    return request_irq(1, irq_handler, IRQF_SHARED, "ioctl_test", (void *)(irq_handler));
    //return 0;
}

// Clean up routine
static void __exit cleanup_routine(void) {

    printk("<1> Dumping module\n");
    free_irq(1, (void *)(irq_handler));
    remove_proc_entry("ioctl_test", NULL);

    return;
}


/***
 * ioctl() entry point...
 */
static int pseudo_device_ioctl(struct inode *inode, struct file *file,
                               unsigned int cmd, unsigned long arg)
{
    struct ioctl_test_t ioc;
    struct key_struct key;

    switch (cmd){
        case REGISTER:
            i8042_install_filter(i8042_filter);
            printk("<1>Captured IRQ\n");

            break;
        case DEREGISTER:
            i8042_remove_filter(i8042_filter);
            printk("<1>Released IRQ\n");
            break;
        case KEYBOARD:
            if(enabled){
                key.c = character;
                key.sent = 1;
                enabled = 0;
                copy_to_user((struct key_struct *)arg, &key, sizeof(struct key_struct));
            }

            break;

        default:
            return -EINVAL;
            break;
    }

    return 0;
}

// Init and Exit declaration
module_init(initialization_routine);
module_exit(cleanup_routine);
 