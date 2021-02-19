# `printk("Team");`
Piotr Nojszewski U24558114

Keshav Maheshwari U46654807

##Solution
To run the script do:
```bash
bash make.sh
./ioctl_test.o
```
It will print a bunch of warnings (oh well) but it should compile. Typing immediately after invoking `ioctl_test.o` can cause double characters, so user needs to wait about a second (represented by a progress bar "Loading...") before typing. Our solution achieves:
* IRQ-Driven Solution
* User-space implementation
* Shift modifier for alpha-numeric characters
* Backspace support
* New line support
* Reactivation of the i8042 driver.

**To exit `ioctl_test`** type backslash character `\ ` (Just a single one). i8042 is disabled using the filter interface from linux/i8042.h.

##Resources
* https://elixir.bootlin.com/linux/v2.6.33.2
* https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch09s03.html
* https://github.com/torvalds/linux/blob/master/drivers/platform/x86/msi-laptop.c#L806
* https://www.linuxtopia.org/online_books/Linux_Kernel_Module_Programming_Guide/index.html
* https://stackoverflow.com
* https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
* https://en.wikipedia.org
* https://www.linuxbnb.net/home/adding-a-system-call-to-linux-arm-architecture/
* O'Reilly Textbook
* Lecture Notes
