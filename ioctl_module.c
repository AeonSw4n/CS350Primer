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

MODULE_LICENSE("GPL");

static struct workqueue_struct *my_workqueue;

struct key_press {
    char letter;
    char sent;
} key;

struct nothing {
    char imr;
};

#define IOCTL_TEST _IOW(0, 6, struct ioctl_test_t)
#define KEYBOARD _IOR(1, 7, struct key_press)
#define REGISTER _IOW(0, 8, struct nothing)
#define DEREGISTER _IOW(0, 9, struct nothing)

static int pseudo_device_ioctl(struct inode *inode, struct file *file,
                               unsigned int cmd, unsigned long arg);

static struct file_operations pseudo_dev_proc_operations;

static struct proc_dir_entry *proc_entry;

// Inline assembly Functions
static inline unsigned char inb( unsigned short usPort ) {

    unsigned char uch;

    asm volatile( "inb %1,%0" : "=a" (uch) : "Nd" (usPort) );
    return uch;
}

static inline void outb( unsigned char uch, unsigned short usPort ) {

    asm volatile( "outb %0,%1" : : "a" (uch), "Nd" (usPort) );
}

static char character;
static char cc;
static char enabled;

static char key_pressed(){
    //1 released
    //0 pressed
    return *((char *)inb(0x60)) & 0x80 ? 1: 0;
}

// Character retrieval method
char my_getchar ( void ) {

    char c;
    unsigned char status;
    static char scancode[128] = "\0\e1234567890-=\177\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000789-456+1230.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";


    /* Poll keyboard status register at port 0x64 checking bit 0 to see if
     * output buffer is full. We continue to poll if the msb of port 0x60
     * (data port) is set, as this indicates out-of-band data or a release
     * keystroke
     */
    //while( !(inb( 0x64 ) & 0x1) || ( ( c = inb( 0x60 ) ) & 0x80 ) );
    //printk("1");
    if(!( ( c = inb( 0x60 ) ) & 0x80 )){
        //printk("2");
        c = inb(0x60);
        return scancode[ (int)c ];
    }
    else {
        return 0;
    }
    //status = inb(0x64);
    //c = inb(0x60);
    //printk("%c",scancode[ (int)c ]);
    //static char s[1];
    //s[0] = scancode[(int)c];
    //char *sptr = &s;
    //*(sptr++) = (char)0;
    //my_printk(s);
    //return scancode[ (int)c ];

}

char toUpper (char c, char keyPress){
    if(keyPress){
        if(c >= 'a' && c<='z')
            return (char)(c-22);
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

irqreturn_t irq_handler(int irq, void *dev_id, struct pt_regs *regs){
    static int init = 0;
    //static unsigned char scancode;
    //static struct work_struct task;
    unsigned char status;


//    status = inb(0x64);
//    scancode = inb(0x60);

    if (init == 0){
        printk("init");
        //INIT_WORK(&task, got_char);
        init = 1;
    } else {
        //printk("got something");
        char c = my_getchar();
        if(c){
            key.letter = c;
            character = c;
            //char str[1];
            //str[0] = c;
            //my_printk(str);
            enabled = 1;

            //cc = toUpper(c, key_pressed());
            //printk("3");
        }
        //PREPARE_WORK(&task, got_char, &scancode);
    }
    //queue_work(my_workqueue, &task);
    return IRQ_HANDLED;
}

// Init Routine
static int __init initialization_routine(void) {
    printk("<1> Loading module\n");
    pseudo_dev_proc_operations.ioctl = pseudo_device_ioctl;
    /* Start create proc entry */
    proc_entry = create_proc_entry("ioctl_test", 0444, NULL);
    if(!proc_entry)
    {
        printk("<1> Error creating /proc entry.\n");
        return 1;
    }
    //free_irq(1, NULL);
    character = -1;
    enabled = 0;
    msleep(1000);
    proc_entry->proc_fops = &pseudo_dev_proc_operations;
    //my_workqueue = create_workqueue(MY_WORK_QUEUE_NAME);
    my_workqueue = create_singlethread_workqueue("0BrainCellsQueue");
    free_irq(1, (void *)(irq_handler));
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
    struct key_press key;

    switch (cmd){
        case REGISTER:
            //sleep(0.10);
            //disable_irq(1);
            //sleep(0.10);
            printk("<1>Disabled IRQ\n");
            break;
        case DEREGISTER:
            //sleep(0.10);
            //enable_irq(1);
            printk("<1>Enabled IRQ\n");
            break;
        case KEYBOARD:
            //key.letter ='a';
            //my_printk("Disabled IRQ\n");
            //key.letter = my_getchar();
            //my_printk("Got Char\n");
            //ioctl_arg = arg;
            if(enabled){
                key.letter = character;
                key.sent = 1;
                enabled = 0;
                copy_to_user((struct key_press *)arg, &key, sizeof(struct key_press));
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
