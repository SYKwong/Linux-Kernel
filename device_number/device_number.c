#include <linux/module.h>
#include <linux/init.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kyle Kwong");
MODULE_DESCRIPTION("Registers a devicee");

static int driver_open(struct inode *device_file, struct file *file_instance){
	printk("device_number: Open\n");
	return 0;
}

static int driver_close(struct inode *device_file, struct file *file_instance){
	printk("device_number: Close\n");
	return 0;
}

static struct file_operations fops ={
	.owner = THIS_MODULE,
	.open=driver_open,
	.release=driver_close
};

#define MAJOR_DEVICE_NUMBERE 90

static int __init my_init(void) {
	int return_value;
	printk("Hello, Kernel!\n");

	return_value = register_chrdev(MAJOR_DEVICE_NUMBERE, "my_device", &fops);

	if(return_value==0){
		printk("device_number: registered device number, Major: %d, Minor: %d\n", MAJOR_DEVICE_NUMBERE, 0);
	}else if(return_value>0){
		printk("device_number: registered device number, Major: %d, Minor: %d\n", return_value>>20, return_value&0xFFFFF);
	}else{
		printk("Could not register device number\n");
		return -1;
	}

	return 0;
}

static void __exit my_exit(void) {
	unregister_chrdev(MAJOR_DEVICE_NUMBERE, "my_device");
	printk("Goodbye, Kernel\n");
}

module_init(my_init);
module_exit(my_exit);