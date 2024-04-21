#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/err.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/siginfo.h>


#define MY_SIGNAL 40

/* Function prototype*/
static int gpio_platform_driver_prove(struct platform_device *pdev);
static int gpio_platform_driver_remove(struct platform_device *pdev);

int bt_cdev_open(struct inode *inode, struct file *filp);
int bt_cdev_release(struct inode *inode, struct file *filp);

static irqreturn_t button_irq_handler(int irq, void *dev_id);
void send_sig_to_user(int sig_val);

/* Global variable */
struct gpio_desc *btleft, *btright, *btup, *btdown;
struct gpio_desc *led;
int btleft_irq, btright_irq, btup_irq, btdown_irq;

struct task_struct *task;

/* This holds the device number */
dev_t device_number;

/* Cdev variable */
struct cdev button_cdev;

/*holds the class pointer */
struct class *button_class;
/*holds the device pointer */
struct device *button_device;


struct file_operations fops = {
    .open = bt_cdev_open,    
    .release = bt_cdev_release,
    .owner = THIS_MODULE
};



/* Of match table  */
struct of_device_id match_device_id[] = {
    {.compatible = "snake-gpio-sysfs"},
    {}
};

/* Struct platform driver  */
struct platform_driver gpio_platform_driver = {
    .probe = gpio_platform_driver_prove,
    .remove = gpio_platform_driver_remove,
    .driver = {
        .owner = THIS_MODULE,
        .of_match_table = match_device_id
    }
};

/* Funtion: Send signal from kernel to user */
void send_sig_to_user(int sig_val){
    struct siginfo info;
    info.si_signo = MY_SIGNAL;
    info.si_code = SI_QUEUE;
    info.si_int = sig_val;

    if (task!= NULL) {
		if (send_sig_info(MY_SIGNAL, (struct kernel_siginfo *)&info, task) < 0)
			pr_err("send signal failed\n");
	} else {
		pr_err("pid_task error\n");
	}
}

/* Test interrupt function: on off led */
void swap_led(void){
    if(gpiod_get_value(led)){
        gpiod_set_value(led, 0);
    }else{
        gpiod_set_value(led, 1);
    }
}

/* Interrupt handle function*/
static irqreturn_t button_irq_handler(int irq, void *dev_id){
    if(irq == btleft_irq){
        swap_led();
        send_sig_to_user(1);
    }
    if(irq == btright_irq){
        swap_led();
        send_sig_to_user(2);
    }
    if(irq == btup_irq){
        swap_led();
        send_sig_to_user(3);
    }
    if(irq == btdown_irq){
        swap_led();
        send_sig_to_user(4);
    }

    return IRQ_HANDLED;
}

/* Open file function*/
int bt_cdev_open(struct inode *inode, struct file *filp){
    struct pid *current_pid;

	current_pid = get_task_pid(current, PIDTYPE_PID);
	task = pid_task(current_pid, PIDTYPE_PID);

    return 0;
}
/* Close file function*/
int bt_cdev_release(struct inode *inode, struct file *filp){

    return 0;
}

/* Prove function*/
static int gpio_platform_driver_prove(struct platform_device *pdev){
    struct device *dev = &pdev->dev; 
    int retval;

    //Button left config 
    btleft = gpiod_get(dev, "btleft", GPIOD_ASIS);
    if(IS_ERR(btleft)){
        pr_err("Error to get info of bt left gpio\n");
        return PTR_ERR(btleft);
    }
    gpiod_direction_input(btleft);
    gpiod_set_debounce(btleft, 100);
    btleft_irq =  gpiod_to_irq(btleft);
    retval = devm_request_irq(  dev, 
                                btleft_irq, 
                                button_irq_handler, 
                                IRQF_TRIGGER_FALLING | IRQF_SHARED, 
                                "button-gpio-handler", 
                                NULL);

    //Button right config 
    btright = gpiod_get(dev, "btright", GPIOD_ASIS);
    if(IS_ERR(btright)){
        pr_err("Error to get info of bt right gpio\n");
        return PTR_ERR(btright);
    }
    gpiod_direction_input(btright);
    gpiod_set_debounce(btright, 100);
    btright_irq = gpiod_to_irq(btright);
    retval = devm_request_irq(  dev, 
                                btright_irq, 
                                button_irq_handler, 
                                IRQF_TRIGGER_FALLING | IRQF_SHARED, 
                                "button-gpio-handler", 
                                NULL);

    ////Button up config 
    btup = gpiod_get(dev, "btup", GPIOD_ASIS);
    if(IS_ERR(btup)){
        pr_err("Error to get info of bt up gpio\n");
        return PTR_ERR(btup);
    }
    gpiod_direction_input(btup);
    gpiod_set_debounce(btup, 100);
    btup_irq = gpiod_to_irq(btup);    
    retval = devm_request_irq(  dev, 
                                btup_irq, 
                                button_irq_handler, 
                                IRQF_TRIGGER_FALLING | IRQF_SHARED, 
                                "button-gpio-handler", 
                                NULL);

    //Button down config
    btdown = gpiod_get(dev, "btdown", GPIOD_ASIS);
    if(IS_ERR(btdown)){
        pr_err("Error to get info of bt down gpio\n");
        return PTR_ERR(btdown);
    }
    gpiod_direction_input(btdown);
    gpiod_set_debounce(btdown, 100);
    btdown_irq = gpiod_to_irq(btdown);
    retval = devm_request_irq(  dev, 
                                btdown_irq, 
                                button_irq_handler, 
                                IRQF_TRIGGER_FALLING | IRQF_SHARED, 
                                "button-gpio-handler", 
                                NULL);

    // Led config 
    led = gpiod_get(dev, "led", GPIOD_ASIS);
    if(IS_ERR(led)){
        pr_err("Error to get info of led gpio\n");
        return PTR_ERR(led);
    }
    gpiod_direction_output(led, 0);

    pr_info(" Device proved \n");
    return 0;
}


static int gpio_platform_driver_remove(struct platform_device *pdev){
    gpiod_put(btleft);
    gpiod_put(btright);
    gpiod_put(btup);
    gpiod_put(btdown);
    return 0;
}


int __init init_module(void){
    platform_driver_register(&gpio_platform_driver);
    
    int ret;
    /*1. Dynamically allocate a device number */
    ret = alloc_chrdev_region(&device_number,0,1, "button_dev_num");
    if(ret < 0){
        pr_err("Alloc chrdev failed\n");
        goto out;
    }
    /*2. Initialize the cdev structure with fops*/
    cdev_init(&button_cdev,&fops);
    /* 3. Register a device (cdev structure) with VFS */
    ret = cdev_add(&button_cdev,device_number,1);
    if(ret < 0){
		pr_err("Cdev add failed\n");
		goto unreg_chrdev;
	}

    /*4. create device class under /sys/class/ */
	button_class = class_create(THIS_MODULE,"button_class");
	if(IS_ERR(button_class)){
		pr_err("Class creation failed\n");
		ret = PTR_ERR(button_class);
		goto cdev_del;
	}
    /*5.  populate the sysfs with device information */
	button_device = device_create(button_class,NULL,device_number,NULL,"button");
	if(IS_ERR(button_device)){
		pr_err("Device create failed\n");
		ret = PTR_ERR(button_device);
		goto class_del;
	}

class_del:
	class_destroy(button_class);
cdev_del:
	cdev_del(&button_cdev);	
unreg_chrdev:
    unregister_chrdev_region(device_number,1);
out: 
    platform_driver_unregister(&gpio_platform_driver);

    return ret;
}


void __exit cleanup_module(void){
    device_destroy(button_class,device_number);
	class_destroy(button_class);
	cdev_del(&button_cdev);	
    unregister_chrdev_region(device_number,1);
    platform_driver_unregister(&gpio_platform_driver);

}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("daovantruong121102@gmail.com");
MODULE_DESCRIPTION("button moudle");
MODULE_VERSION("1.0");




