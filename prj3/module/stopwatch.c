#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <mach/gpio.h>
#include <linux/platform_device.h>
#include <asm/gpio.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#define IOM_FND_ADDRESS 0x08000004 // fnd's physical address

static int inter_major=242, inter_minor=0;
static int result;
static dev_t inter_dev;
static struct cdev inter_cdev;
static int inter_open(struct inode *, struct file *);
static int inter_release(struct inode *, struct file *);
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);


/*user defined*/
DECLARE_WAIT_QUEUE_HEAD(my_queue); // wait_queue 선언
static unsigned char *iom_fpga_fnd_addr; // fnd write를 위한 address
struct timer_list timer; // stopwatch를 위한 timer
struct timer_list timer1; // 종료를 위한 timer
void action_fnd(void); // fnd function
void undo_fnd(void); // fnd 초기화 하고 종료
static void timer_func(unsigned long timeout); // stopwatch timer function
static void timer_end_func(unsigned long timeout); // 종료를 위한 timer function
static int home_use; // check home button
static int back_use; // check back button
static int timer_use; // check if timer use or not
static int timer1_use; // check if end_timer use or not
static int back_flag; // back_flag가 아무 입력도 없이 처음에 눌렸을때 무시하기 위함
static int count; // timer변수
unsigned short int min; // fnd 분
unsigned short int sec; // fnd 초
unsigned short int value_short; // fnd output value
unsigned long left_time; // save timer left time
char value[4] = {0,0,0,0}; // fnd출력을 위한 배열
/*user define end*/

irqreturn_t inter_handler1(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler2(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler3(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler4(int irq, void* dev_id, struct pt_regs* reg);

static struct file_operations inter_fops =
{
	.open = inter_open,
	.write = inter_write,
	.release = inter_release,
};


/////////////////////////////////////////////////////////////////////////
/* timer function으로 fnd value를 1초마다 갱신하는 function 입니다.     /
 * count는 1로 초기화 하여 1초마다 타이머가 작동될수 있도록 하였습니다 */
/////////////////////////////////////////////////////////////////////////
static void timer_func(unsigned long arg){
	
	action_fnd(); // print fnd

	timer.expires = get_jiffies_64() + HZ;
	timer.data = (unsigned long)&count;
	timer.function = timer_func;

	add_timer(&timer);
	
}

///////////////////////////////////////////////////////////////////////////
/* end timer function으로 vol-가 3초동안 눌렸을 시 작동하는 timer 입니다  /
 * undo_fnd 함수를 통해 종료 전 fnd를 초기화 해주고 stopwatch가 작동중이면/
 * stopwatch timer를 삭제해주고 종료합니다                               */
///////////////////////////////////////////////////////////////////////////
static void timer_end_func(unsigned long arg){
    timer1_use = 0;
    if(timer_use == 1)
        del_timer(&timer);
    undo_fnd();
    __wake_up(&my_queue,1,1,NULL);
}

///////////////////////////////////////////////////////////////////////////
/* stopwatch에서 작동하는 fnd출력용 함수입니다. sec와 min을 update합니다 */
///////////////////////////////////////////////////////////////////////////
void action_fnd(){
	if (++sec == 60) { // sec update
		sec = 0;
		min++;
	}
	value[0] = min / 10;
	value[1] = min % 10;
	value[2] = sec / 10;
	value[3] = sec % 10;
	value_short = value[0] << 12 | value[1] << 8 | value[2] << 4 | value[3];
    outw(value_short,(unsigned int)iom_fpga_fnd_addr); // fnd 출력
    return;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/* home button interrupt handler입니다. 처음에 home button이 눌리면 stopwatch를 시작합니다. /
 * home button이 한번 눌리면 flag를 통해 무시될 수 있도록 했습니다.                        */
/////////////////////////////////////////////////////////////////////////////////////////////
irqreturn_t inter_handler1(int irq, void* dev_id, struct pt_regs* reg) { //home button
	printk(KERN_ALERT "home button is pushed = %x\n", gpio_get_value(IMX_GPIO_NR(1, 11)));
	back_flag = 1; // back button use flag
	count = 1;
    if(home_use) // home button 한번 눌리면 무시
        return IRQ_HANDLED;
	home_use = 1; // make home button input ignore
    timer_use = 1;
	timer.expires = get_jiffies_64() + HZ;
	timer.data = (unsigned long)&count;
	timer.function = timer_func;
	add_timer(&timer);

	return IRQ_HANDLED;
}

///////////////////////////////////////////////////////////////////////////////////////////////
/* back button interrupt handler 입니다. back button이 눌리면 stopwatch start/stop이 되도록  //
 * 설정 하였습니다. left_time 변수를 통해 현재 타이머에 남은 시간을 계산하고 다시 시작 됐을때//
 * 그 시간만큼 진행 되도록 설정하였습니다.                                                   */
///////////////////////////////////////////////////////////////////////////////////////////////
irqreturn_t inter_handler2(int irq, void* dev_id, struct pt_regs* reg) { //back button
    printk(KERN_ALERT "back button is pushed = %x\n", gpio_get_value(IMX_GPIO_NR(1, 12)));
	if (back_flag == 0){
        return IRQ_HANDLED;
    }
	if (back_use == 0) { // stopwatch가 도중에 멈췄을때
		left_time = timer.expires - get_jiffies_64(); // compute left time
		del_timer(&timer); // delete timer
		back_use = 1;
		timer_use = 0;
	}
	else { // stopwatch가 다시 시작할때
		back_use = 0; 
		timer_use = 1;
		timer.expires = get_jiffies_64() + left_time;
		timer.data = (unsigned long)&count;
		timer.function = timer_func;
		add_timer(&timer);
	}

	return IRQ_HANDLED;
}

///////////////////////////////////////////////////////////////////////////////////////////
/* vol+ button interrupt handler 입니다. fnd를 초기화 해주고 min,sec을 통해 시간을 초기화//
 * 할 수 있도록 했습니다.                                                                */
///////////////////////////////////////////////////////////////////////////////////////////
irqreturn_t inter_handler3(int irq, void* dev_id,struct pt_regs* reg) { //vol+ button
    printk(KERN_ALERT "vol+ button is pushed = %x\n", gpio_get_value(IMX_GPIO_NR(2, 15)));
    if(timer_use){
        del_timer(&timer);
        timer.expires = get_jiffies_64() + HZ;
        timer.data = (unsigned long)&count;
        timer.function = timer_func;
        add_timer(&timer);
    }
    left_time = HZ;
	min = 0;
    sec = 0;
	undo_fnd();
	return IRQ_HANDLED;
}

/////////////////////////////////////////////////////////////////////////////////////////
/* vol- button interrupt handler입니다. 버튼이 눌렸을때와 떼졌을때 누 경우를 받아서 만약/
 * 3초가 되기 전에 손을 떼면 end_timer가 삭제 되도록 했습니다.                         */
/////////////////////////////////////////////////////////////////////////////////////////
irqreturn_t inter_handler4(int irq, void* dev_id, struct pt_regs* reg) { //vol- button
	printk(KERN_ALERT "vol- button is pushed = %x\n", gpio_get_value(IMX_GPIO_NR(5, 14)));
    if(gpio_get_value(IMX_GPIO_NR(5,14)) == 1){ // 3초전에 버튼에서 손을 떼면 timer 삭제
        if(timer1_use==1){
            timer1_use = 0; // timer1 use count renew
            del_timer(&timer1); // delete timer1
            printk(KERN_ALERT "delete timer!\n");
        }
        return IRQ_HANDLED;
    }
    if(timer1_use == 0){
        timer1.expires = get_jiffies_64() + 3 * HZ; // 3초 뒤에 작동하게 함
        timer1.data = 1;
        timer1.function = timer_end_func; // register timer function
        timer1_use = 1;
        add_timer(&timer1);
    }
    return IRQ_HANDLED;
}

//////////////////////////////////////////////////////////////////////////////
/* open 함수입니다. 사용한 변수 초기화 및 request_irq 함수를 통해            /
 * 각 button별 interrupt를 등록해주었습니다                                 */
//////////////////////////////////////////////////////////////////////////////
static int inter_open(struct inode *minode, struct file *mfile){
	int ret;
	int irq;
    /* 변수 초기화 */
    home_use = 0;
    back_use = 0;
    timer_use = 0;
    timer1_use = 0;
    back_flag = 0;
    sec = 0;
    min = 0;
    /* 변수 초기화 end */
	printk(KERN_ALERT "Open Module\n");

	// int1 register for home button
	gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq,inter_handler1,IRQF_TRIGGER_FALLING,"home",0);

	// int2 register for back button
	gpio_direction_input(IMX_GPIO_NR(1,12));
	irq = gpio_to_irq(IMX_GPIO_NR(1,12));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq,inter_handler2,IRQF_TRIGGER_FALLING,"back",0);

	// int3 register for vol+ button
	gpio_direction_input(IMX_GPIO_NR(2,15));
	irq = gpio_to_irq(IMX_GPIO_NR(2,15));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq,inter_handler3,IRQF_TRIGGER_FALLING,"vol+",0);

	// int4 register for vol- button
	gpio_direction_input(IMX_GPIO_NR(5,14));
	irq = gpio_to_irq(IMX_GPIO_NR(5,14));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq,inter_handler4,IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,"vol-",0);

	return 0;
}

//////////////////////////////////////////////////////////////////
/* interrupt free함수입니다. button별 interrupt를 free해줍니다  */
//////////////////////////////////////////////////////////////////
static int inter_release(struct inode *minode, struct file *mfile){ // interrupt free function
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL); // free int1
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL); // free int2
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL); // free int3
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL); // free int4
	printk(KERN_ALERT "Release Module\n");
	return 0;
}

static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos ){
	printk("write\n");
	interruptible_sleep_on(&my_queue);
	return 0;
}

//////////////////////////////////////////////////////////////////
/* fnd 초기화 함수로 종료 전이나 초기화 시 0을 출력해 줍니다.   */
//////////////////////////////////////////////////////////////////
void undo_fnd(){ 
    outw(0,(unsigned int)iom_fpga_fnd_addr); // print 0
}

static int inter_register_cdev(void) 
{
	int error;
	if(inter_major) {
		inter_dev = MKDEV(inter_major, inter_minor);
		error = register_chrdev_region(inter_dev,1,"inter");
	}else{
		error = alloc_chrdev_region(&inter_dev,inter_minor,1,"inter");
		inter_major = MAJOR(inter_dev);
	}
	if(error<0) {
		printk(KERN_WARNING "inter: can't get major %d\n", inter_major);
		return result;
	}
	printk(KERN_ALERT "major number = %d\n", inter_major);
	cdev_init(&inter_cdev, &inter_fops);
	inter_cdev.owner = THIS_MODULE;
	inter_cdev.ops = &inter_fops;
	error = cdev_add(&inter_cdev, inter_dev, 1);
	if(error)
	{
		printk(KERN_NOTICE "inter Register Error %d\n", error);
	}
	return 0;
}
//////////////////////////////////////////////////
/* initialize function 입니다                   //
 * fnd ioremap 부분을 추가하였습니다.           //
 * timer1, timer2를 초기화 하였습니다.          */
//////////////////////////////////////////////////
static int __init inter_init(void) { // init function
	int result;
	if((result = inter_register_cdev()) < 0 )
		return result;
	printk(KERN_ALERT "Init Module Success \n");
	printk(KERN_ALERT "Device : /dev/inter, Major Num : 242 \n");

    iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4); // ioremap fnd
    init_timer(&timer);
    init_timer(&timer1);
	return 0;
}

static void __exit inter_exit(void) { // exit function
	iounmap(iom_fpga_fnd_addr); // unmap fnd
	cdev_del(&inter_cdev);
	unregister_chrdev_region(inter_dev, 1);
	printk(KERN_ALERT "Remove Module Success \n");
}

module_init(inter_init);
module_exit(inter_exit);
MODULE_LICENSE("GPL");
