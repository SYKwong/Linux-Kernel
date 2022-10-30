#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kyle Kwong");
MODULE_DESCRIPTION("GPIO driver");

static dev_t device_number;
static struct class *device_class;
static struct cdev device;

#define DRIVER_NAME  "GPIO_driver"
#define DRIVER_CLASS "GPIO_class"

static ssize_t driver_read(struct file *file_instance, char *user_buffer, size_t bytes, loff_t *offs){
	char tmp[3] = " \n";

	const int bytes_to_copy = min(bytes,  sizeof(tmp));
	int bytes_not_copied, diff;

	printk("Value of button: %d\n", gpio_get_value(17));
	tmp[0]=gpio_get_value(17)+'0';
	bytes_not_copied = copy_to_user(user_buffer, &tmp, bytes_to_copy);
	diff = bytes_to_copy - bytes_not_copied;

	return diff;
}

static ssize_t driver_write(struct file *file_instance,const char *user_buffer, size_t bytes, loff_t *offs){
	char value;
	const int bytes_to_write = 1;
	const int bytes_not_written = copy_from_user(&value, user_buffer, bytes_to_write);
	const int diff = bytes_to_write-bytes_not_written;

	printk("value: %d\n", value);
	// printk("size: %ld\n", bytes);

	switch (value){
	case '0':
		gpio_set_value(4,0);
		printk("Turning off LED\n");
		break;
	case '1':
		gpio_set_value(4,1);
		printk("Turning on LED\n");
	case '\n':
		break;
	default:
		printk("Invalid\n");
		break;
	}

	return diff;
}

static int driver_open(struct inode *device_file, struct file *file_instance){
	printk("GPIO: Open\n");
	return 0;
}

static int driver_close(struct inode *device_file, struct file *file_instance){
	printk("GPIO: Close\n");
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
		printk("GPIO device_number Major: %d, Minor: %d\n", device_number>>20, device_number&0xFFFFF);
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

		if(gpio_request(4, "rpi-gpio-4") > 0){
			printk("GPIO allocation faliled");
			goto addError;
		}

		if(gpio_direction_output(4,0) > 0){
			printk("GPIO output setting faliled");
			goto GPIO4Error;
		}

		if(gpio_request(17, "rpi-gpio-17") > 0){
			printk("GPIO allocation faliled");
			goto addError;
		}

		if(gpio_direction_input(17) > 0){
			printk("GPIO input setting faliled");
			goto GPIO17Error;
		}

	}


	return return_value;

GPIO17Error:
	gpio_free(17);

GPIO4Error:
	gpio_free(4);

addError:
	device_destroy(device_class, device_number);

fileError:
	class_destroy(device_class);

classError:
	unregister_chrdev(device_number, "DRIVER_NAME"); 
	return -1;
}

static void __exit my_exit(void) {
	gpio_set_value(4,0);
	gpio_free(17);
	gpio_free(4);
	cdev_del(&device);
	device_destroy(device_class, device_number);
	class_destroy(device_class);
	unregister_chrdev(device_number, "DRIVER_NAME"); 
	printk("Goodbye, Kernel\n");
}

module_init(my_init);
module_exit(my_exit);