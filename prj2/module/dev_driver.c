#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>
#include "fpga_dot_font.h"

#define MAJOR_NUM 242
#define DEV_NAME "mydev"

#define IOM_FND_ADDRESS 0x08000004
#define IOM_LED_ADDRESS 0x08000016
#define IOM_LCD_ADDRESS 0x08000090
#define IOM_DOT_ADDRESS 0x08000210
#define GET_VALUE 0x0000000FF

static unsigned char *iom_fpga_fnd_addr;
static unsigned char *iom_fpga_lcd_addr;
static unsigned char *iom_fpga_led_addr;
static unsigned char *iom_fpga_dot_addr;

int dev_usage;
struct timer_list timer;
int count,fnd_start,fnd_value,interval,i;
int name_offset,name_dir,id_offset,id_dir,fnd_count;
char val[4] = {0,0,0,0};
unsigned char blank[33];
unsigned char *name = "AhnSiHyun";
unsigned char *id = "20141544";
static void timer_func(unsigned long timeout);

int my_dev_open(struct inode *minode, struct file *mfile);
int my_dev_release(struct inode *minode, struct file *mfile);
long my_dev_ioctl(struct file *inode, unsigned int cmd, unsigned long arg);
void action_fnd(void);
void action_led(void);
void action_dot(void);
void action_lcd(void);
void undo_action(void);

static struct file_operations dev_fops =
{
	.owner		=	THIS_MODULE,
	.open		=	my_dev_open,	
	.unlocked_ioctl = 	my_dev_ioctl,	
	.release	=	my_dev_release
}; // resgister function


int my_dev_open(struct inode *minode, struct file *mfile) 
{	
    printk("dev_open is called\n");
    val[0] = 0;  // init value
    val[1] = 0; 
    val[2] = 0; 
    val[3] = 0;
    name_offset = 0; name_dir = 0;
    id_offset = 0; id_dir = 0;
    fnd_count = 0;
	if(dev_usage != 0) return -EBUSY;
	dev_usage = 1;
	return 0;
}

int my_dev_release(struct inode *minode, struct file *mfile) 
{
    printk("dev_release is called\n");
	dev_usage = 0;
	return 0;
}

long my_dev_ioctl(struct file *inode,unsigned int cmd,unsigned long arg)
{
    unsigned int param; // receive 4byte stream
    if(copy_from_user(&param,(char *)arg,sizeof(unsigned int))){
        return -1;
    }
    printk("dev_ioctl is called\n");
    
	if(cmd!=0){ // wrong input
		printk("Wrong cmd input\n");
		return -1;
	}
    
    fnd_start = (param >> 24) & GET_VALUE; // get start pos
    fnd_value = (param >> 16) & GET_VALUE; // get start value
    interval = (param >> 8) & GET_VALUE; // get interval
    count = param & GET_VALUE; // get count

	timer.expires = get_jiffies_64() + (interval * HZ / 10);
	timer.data = (unsigned long)&count;	
	timer.function = timer_func;
	add_timer(&timer);
    
    return 0;
}

static void timer_func(unsigned long arg){

    if(count == 0){
        undo_action();
        return;
    }
    count--;

    action_fnd(); // action function
    action_led();
    action_dot();
    action_lcd();

	timer.expires = get_jiffies_64() + (interval * HZ / 10);
	timer.data = (unsigned long)&count;
	timer.function = timer_func;

    add_timer(&timer);

}

void action_fnd(){ // fnd output
    unsigned short int value;
    if(fnd_count == 8){
        val[fnd_start] = 0;
        fnd_start = (fnd_start + 1) % 4;
        fnd_count = 0;
    }
    val[fnd_start] = fnd_value;
    fnd_count++;

    value = val[0]<<12 | val[1] << 8 | val[2] << 4 | val[3];
    outw(value,(unsigned int)iom_fpga_fnd_addr);
    return;
}

void action_led(){ // led output
    unsigned short s_value = 128;
    for(i = 0 ; i < fnd_value-1; i++){
        s_value /= 2;
    }
    
    outw(s_value, (unsigned int)iom_fpga_led_addr);
}
void action_dot(){ // dot output
    unsigned short int s_value;
    for(i = 0 ; i < 10 ; i++){
        s_value = fpga_number[fnd_value][i] & 0x7f;
        outw(s_value,(unsigned int)iom_fpga_dot_addr+i*2);
    }
    fnd_value = (fnd_value % 8) + 1;
}
void action_lcd(){ // lcd output
    unsigned short int s_value = 0;
    unsigned char value[33];
    memcpy(value,blank,33);
    memcpy(value + id_offset,id,8);
    memcpy(value + name_offset + 16,name,9);

    if(name_dir == 0){
        name_offset++;
        if(name_offset == 7) name_dir = 1;
    }
    else{
        name_offset--;
        if(name_offset == 0) name_dir = 0;
    }
    if(id_dir == 0){
        id_offset++;
        if(id_offset == 8) id_dir = 1;
    }
    else{
        id_offset--;
        if(id_offset == 0) id_dir = 0;
    }

    for(i = 0 ; i < 32; i++){
        s_value = (value[i] & 0xFF) << 8 | (value[i+1] & 0xFF);
        outw(s_value,(unsigned int)iom_fpga_lcd_addr+i);
        i++;
    }
}

void undo_action(){ // reset function called before termination
    unsigned short int s_value;
    unsigned char value[33];
    memcpy(value,blank,33);

    outw(0,(unsigned int)iom_fpga_fnd_addr); // reset fnd
    outw(0,(unsigned int)iom_fpga_led_addr); // reset led

    for(i = 0 ; i < 10 ; i++){
        s_value = fpga_set_blank[i];
        outw(s_value,(unsigned int)iom_fpga_dot_addr + i*2); //reset dot
    }
    for(i = 0 ; i < 32 ; i++){
        s_value = (value[i] & 0xFF) << 8 | (value[i+1] & 0xFF);
        outw(s_value,(unsigned int)iom_fpga_lcd_addr+i); // reset lcd
    }

}

int __init my_dev_init(void)
{
    int result;
    printk("dev_init is called\n");
	result = register_chrdev(MAJOR_NUM, DEV_NAME, &dev_fops);

	if(result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return -1;
	}

	iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
	iom_fpga_led_addr = ioremap(IOM_LED_ADDRESS, 0x1);
	iom_fpga_lcd_addr = ioremap(IOM_LCD_ADDRESS, 0x32);
	iom_fpga_dot_addr = ioremap(IOM_DOT_ADDRESS, 0x10);
	
    for(i = 0 ; i <= 32 ; i++)
        blank[i] = ' ';
    
	init_timer(&timer);
	printk("init module, %s major number : %d\n", DEV_NAME, MAJOR_NUM);

	return 0 ;
}

void __exit my_dev_exit(void) 
{
    printk("dev_exit is called\n");
	iounmap(iom_fpga_fnd_addr);
	iounmap(iom_fpga_led_addr);
	iounmap(iom_fpga_lcd_addr);
	iounmap(iom_fpga_dot_addr);

    dev_usage = 0;
    del_timer_sync(&timer);
	unregister_chrdev(MAJOR_NUM, DEV_NAME);
    return;
}

module_init(my_dev_init);
module_exit(my_dev_exit);

MODULE_LICENSE("GPL");

