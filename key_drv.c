/**********************************************
name : key_drv.c
function: the  led  lights  when the button  press
time :	2017/3/12
author :	srfdjkk
***********************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

/*定义*/
#define DEVICE_NAME "key_drv"		//设备名称
volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

/*定义类*/
static struct class * key_drv_class;
static struct class_device  *key_drv_class_dev;

// 组织硬件资源结构体



/*中断函数*/


static int key_open(struct inode *inode, struct file *file)
{
	*gpfcon &= ~((0x03<<(0*2))|(0x03<<(2*2)));  //设置GPF0,2 为输入模式
	*gpgcon &= ~(0x03<<(3*2));  //设置GPG3 为输入模式
	return 0;
}

/*返回3个按键的电平*/
ssize_t key_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	unsigned char  key_vals[3];
	int regval;
	
	if(size!=sizeof(key_vals))
		return -EINVAL;
	
	//读取GPF0 2 的值
	regval = *gpfdat;
	key_vals[0] = regval &(1<<0)? 1:0;
	key_vals[1] = regval &(1<<2)? 1:0;
	//读取 GPG3 的值
	regval = *gpgdat;
	key_vals[2] = regval &(1<<3)? 1:0;

	copy_to_user(buf, key_vals, sizeof(key_vals));

	return sizeof(key_vals);
	
}

/* 这个结构是字符设备驱动程序的核心
 * 当应用程序操作设备文件时所调用的open、read、write等函数，
 * 最终会调用这个结构中的对应函数
 */
static struct file_operations key_fops ={
	.owner 	= THIS_MODULE,
	.open	= key_open,
	.read 	= key_read,
	
};


/*
**注册字符设备
*/
int  major;
static int __init buttons_init(void)
{
	major = register_chrdev(0, DEVICE_NAME, &key_fops);
	if(major <0)		
	{
		printk(DEVICE_NAME"register faild \n");
		return major;

	}
	else
	{
		printk(DEVICE_NAME"register successful!  \n");
	}
	
	//创建类，并在类下面创建一个设备节点
	key_drv_class = class_create(THIS_MODULE, DEVICE_NAME);
	key_drv_class_dev = class_device_create(key_drv_class, NULL, MKDEV(major,0), NULL, "key");  

	//io口映射，，映射为虚拟地址
	gpfcon = (volatile unsigned  long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;
	
	return  0;
}

/*
**销毁字符设备
*/
static  void  __exit buttons_exit(void)
{
	//卸载 设备
	unregister_chrdev(major, DEVICE_NAME);

	//删除类
	class_destroy(key_drv_class);
	class_device_unregister(key_drv_class_dev);

	//取消映射  减少内存
	iounmap(gpfcon);
	iounmap(gpgcon);
	return 0;
}


module_init(buttons_init);
module_exit(buttons_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SHIRUNFA");
MODULE_DESCRIPTION("S3c2440 buttons drivers");