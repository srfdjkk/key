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

/*����*/
#define DEVICE_NAME "key_drv"		//�豸����
volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

/*������*/
static struct class * key_drv_class;
static struct class_device  *key_drv_class_dev;

// ��֯Ӳ����Դ�ṹ��



/*�жϺ���*/


static int key_open(struct inode *inode, struct file *file)
{
	*gpfcon &= ~((0x03<<(0*2))|(0x03<<(2*2)));  //����GPF0,2 Ϊ����ģʽ
	*gpgcon &= ~(0x03<<(3*2));  //����GPG3 Ϊ����ģʽ
	return 0;
}

/*����3�������ĵ�ƽ*/
ssize_t key_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	unsigned char  key_vals[3];
	int regval;
	
	if(size!=sizeof(key_vals))
		return -EINVAL;
	
	//��ȡGPF0 2 ��ֵ
	regval = *gpfdat;
	key_vals[0] = regval &(1<<0)? 1:0;
	key_vals[1] = regval &(1<<2)? 1:0;
	//��ȡ GPG3 ��ֵ
	regval = *gpgdat;
	key_vals[2] = regval &(1<<3)? 1:0;

	copy_to_user(buf, key_vals, sizeof(key_vals));

	return sizeof(key_vals);
	
}

/* ����ṹ���ַ��豸��������ĺ���
 * ��Ӧ�ó�������豸�ļ�ʱ�����õ�open��read��write�Ⱥ�����
 * ���ջ��������ṹ�еĶ�Ӧ����
 */
static struct file_operations key_fops ={
	.owner 	= THIS_MODULE,
	.open	= key_open,
	.read 	= key_read,
	
};


/*
**ע���ַ��豸
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
	
	//�����࣬���������洴��һ���豸�ڵ�
	key_drv_class = class_create(THIS_MODULE, DEVICE_NAME);
	key_drv_class_dev = class_device_create(key_drv_class, NULL, MKDEV(major,0), NULL, "key");  

	//io��ӳ�䣬��ӳ��Ϊ�����ַ
	gpfcon = (volatile unsigned  long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;
	
	return  0;
}

/*
**�����ַ��豸
*/
static  void  __exit buttons_exit(void)
{
	//ж�� �豸
	unregister_chrdev(major, DEVICE_NAME);

	//ɾ����
	class_destroy(key_drv_class);
	class_device_unregister(key_drv_class_dev);

	//ȡ��ӳ��  �����ڴ�
	iounmap(gpfcon);
	iounmap(gpgcon);
	return 0;
}


module_init(buttons_init);
module_exit(buttons_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SHIRUNFA");
MODULE_DESCRIPTION("S3c2440 buttons drivers");