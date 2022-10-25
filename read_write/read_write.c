#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kyle Kwong");
MODULE_DESCRIPTION("Reading and writing");

static char buffer[255];
static int buffer_pointer;

static dev_t device_number;
static struct class *device_class;
static struct cdev device;

#define DRIVER_NAME  "dummy_driver"
#define DRIVER_CLASS "module_class"

static ssize_t driver_read(struct file *file_instance, char *user_buffer, size_t bytes, loff_t *offs){
	const int bytes_to_copy = min(bytes,  buffer_pointer);
	const int bytes_not_copied = copy_to_user(user_buffer, buffer, bytes_to_copy);
	const int diff = bytes_to_copy - bytes_not_copied;

	return diff;
}

static ssize_t driver_write(struct file *file_instance,const char *user_buffer, size_t bytes, loff_t *offs){
	const int bytes_to_write = min(bytes, sizeof(buffer));
	const int bytes_not_written = copy_from_user(buffer, user_buffer, bytes_to_write);
	const int diff = bytes_to_write-bytes_not_written;

	buffer_pointer = bytes_to_write;

	return diff;
}

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
	.release=driver_close,
	.read=driver_read,
	.write=driver_write
};

static int __init my_init(void) {
	int return_value = -1;
	printk("Hello, Kernel!\n");

	if(alloc_chrdev_region(&device_number, 0, 1, DRIVER_NAME)<0){
		printk("Device number cannot be allocated\n");
	}else{
		printk("read_write device_number Major: %d, Minor: %d", device_number>>20, device_number&0xFFFFF);
		return_value=0;

		device_class=class_create(THIS_MODULE, DRIVER_CLASS);	
		if(NULL==device_class){
			printk("device_class cannot be created\n");
			goto classError;
		}

		if(NULL==device_create(device_class, NULL, device_number, NULL, DRIVER_NAME)){
			printk("device file cannot be created\n");
			goto fileError;
		}

		cdev_init(&device,&fops);
		if(cdev_add(&device, device_number, 1) == -1){
			printk("Device register failed");
			goto addError;
		}

	}
	return return_value;

addError:
	device_destroy(device_class, device_number);

fileError:
	class_destroy(device_class);

classError:
	unregister_chrdev(device_number, "DRIVER_NAME"); 
	return -1;
}

static void __exit my_exit(void) {
	cdev_del(&device);
	device_destroy(device_class, device_number);
	class_destroy(device_class);
	unregister_chrdev(device_number, "DRIVER_NAME"); 
	printk("Goodbye, Kernel\n");
}

module_init(my_init);
module_exit(my_exit);