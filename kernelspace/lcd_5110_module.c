#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/spi/spidev.h>
#include <linux/mod_devicetable.h>
#include <linux/cdev.h>
#include "lcd_5110.h"
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>



/* Function prototype */
static long lcd_dev_ioctl(struct file *filp, unsigned int, unsigned long);
static int  lcd_dev_open(struct inode *inode, struct file *filp);
static int  lcd_dev_release(struct inode *inode, struct file *filp);
ssize_t lcd_dev_write(struct file *, const char __user *, size_t, loff_t *);

static int	lcd_spi_prove(struct spi_device *spi);
static int	lcd_spi_remove(struct spi_device *spi);


/*=======================  LCD_Device   ===========================*/

/* This holds the device number */
dev_t device_number;
/* Cdev variable */
struct cdev lcd_cdev;
/*holds the class pointer */
struct class *class_lcd;
struct device *device_lcd;

struct file_operations fops = {
    .open = lcd_dev_open,
    .release = lcd_dev_release,
    .unlocked_ioctl = lcd_dev_ioctl,
    .write = lcd_dev_write
};

static long lcd_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
    void __user *argp = (void __user *)arg;
	unsigned char *value = (unsigned char *)argp;
	unsigned char contrast;
	Position_t *pos = NULL;
	Draw_Pixel_t *pixel = NULL;
	Draw_Shape_t *shape = NULL;
	Draw_Circle_t *cir = NULL;
    int ret;

	switch (cmd) {
	case IOCTL_SEND_BUFF:
        LCD_Refresh();
        break;
	case IOCTL_CLEAR:
        LCD_Clear();
        break;

	case IOCTL_HOME:
        LCD_Home();
        break;

	case IOCTL_SET_CONTRAST:
        get_user(contrast, value);
        LCD_SetContrast(contrast);
        break;

	case IOCTL_GOTOXY:
        pos = kmalloc(sizeof(Position_t), GFP_KERNEL);
        ret = copy_from_user(pos, argp, sizeof(Position_t));
        if (ret) {
		    pr_err("can not copy from user\n");
		    return -ENOMSG;
	    }
        LCD_GotoXY(pos->x, pos->y);
        break;

	case IOCTL_DRAW_PIXEL:
        pixel = kmalloc(sizeof(Draw_Pixel_t), GFP_KERNEL);
        ret = copy_from_user(pixel, argp, sizeof(Draw_Pixel_t));
        if (ret) {
		    pr_err("can not copy from user\n");
		    return -ENOMSG;
	    }
        LCD_DrawPixel(pixel->x, pixel->y, pixel->pixel);
        break;

	case IOCTL_DRAW_LINE:
        shape = kmalloc(sizeof(Draw_Shape_t), GFP_KERNEL);
        ret = copy_from_user(shape, argp, sizeof(Draw_Shape_t));
        if (ret) {
		    pr_err("can not copy from user\n");
		    return -ENOMSG;
	    }
        LCD_DrawLine(shape->x0, shape->y0, shape->x1, shape->y1,
                shape->pixel);
        break;

	case IOCTL_DRAW_RECT:
        shape = kmalloc(sizeof(Draw_Shape_t), GFP_KERNEL);
        ret = copy_from_user(shape, argp, sizeof(Draw_Shape_t));
        if (ret) {
		    pr_err("can not copy from user\n");
		    return -ENOMSG;
	    }
        LCD_DrawRectangle(shape->x0, shape->y0, shape->x1,
                shape->y1, shape->pixel);
        break;

	case IOCTL_DRAW_FILL_RECT:
        shape = kmalloc(sizeof(Draw_Shape_t), GFP_KERNEL);
        ret = copy_from_user(shape, argp, sizeof(Draw_Shape_t));
        if (ret) {
		    pr_err("can not copy from user\n");
		    return -ENOMSG;
	    }
        LCD_DrawFilledRectangle(shape->x0, shape->y0, shape->x1,
                shape->y1, shape->pixel);
        break;

	case IOCTL_DRAW_CIRCLE:
        cir = kmalloc(sizeof(Draw_Circle_t), GFP_KERNEL);
        ret = copy_from_user(cir, argp, sizeof(Draw_Circle_t));
        if (ret) {
		    pr_err("can not copy from user\n");
		    return -ENOMSG;
	    }
        LCD_DrawCircle(cir->x, cir->y, cir->r, cir->pixel);
        break;

	case IOCTL_DRAW_FILL_CIRCLE:
        cir = kmalloc(sizeof(Draw_Circle_t), GFP_KERNEL);
        ret = copy_from_user(cir, argp, sizeof(Draw_Circle_t));
        if (ret) {
		    pr_err("can not copy from user\n");
		    return -ENOMSG;
	    }
        LCD_DrawFilledCircle(cir->x, cir->y, cir->r, cir->pixel);
        break;

	default:
		return -ENOTTY;
	}

	return 0;
}

static int  lcd_dev_open(struct inode *inode, struct file *filp){

    return 0;
}

static int  lcd_dev_release(struct inode *inode, struct file *flip){

    return 0;
}

ssize_t lcd_dev_write(struct file *flip, const char __user *buf, size_t len, loff_t *offset){
    int ret;
	Draw_String_t *str = NULL;
	str = kmalloc(sizeof(Draw_String_t), GFP_KERNEL );
	ret = copy_from_user(str, buf, sizeof(Draw_String_t));
	if (ret) {
		pr_err("can not copy from user\n");
		return -ENOMSG;
	}

	LCD_Puts(str->message, str->pixel, str->font);

	return len;
}


/*=======================  SPI   ===========================*/
struct gpio_desc *rs_pin, *dc_pin;
struct spi_device *spidev;

static const struct spi_device_id spi_dev_id[] = {
    {.name = "lcd-5110-spi"},
    {},
};
MODULE_DEVICE_TABLE(spi, spi_dev_id);

static const struct of_device_id spidev_dt_ids[] = {
    {.compatible = "nokia,lcd-5110"},
    {},
};
MODULE_DEVICE_TABLE(of, spidev_dt_ids);

struct spi_driver spi_lcd_driver = {
    .probe = lcd_spi_prove,
    .remove = lcd_spi_remove,
    .id_table = spi_dev_id,
    .driver = {
        .name = "lcd-5110-spi",
        .of_match_table = spidev_dt_ids,
        .owner = THIS_MODULE,
    }
};


static int	lcd_spi_prove(struct spi_device *spi){
    spidev = spi;
    pr_info("spi0 is matched \n");
    struct device *dev = &spi->dev;
    /* Enable DC pin and Reset Pin for LCD */
    rs_pin = gpiod_get(dev, "rs", GPIOD_ASIS);
    if(IS_ERR(rs_pin)){
        pr_err("Error to get info of bt left gpio\n");
        return PTR_ERR(rs_pin);
    }

    dc_pin = gpiod_get(dev, "dc", GPIOD_ASIS);
    if(IS_ERR(dc_pin)){
        pr_err("Error to get info of bt right gpio\n");
        return PTR_ERR(dc_pin);
    }
    return 0;
}
static int	lcd_spi_remove(struct spi_device *spi){
    gpiod_put(rs_pin);
    gpiod_put(dc_pin);
    
    return 0;
}




/*=======================  Module  ===========================*/

int __init init_module(void){
    spi_register_driver(&spi_lcd_driver);

    int ret;
    ret = alloc_chrdev_region(&device_number, 1, 0, "lcd_dev");
    if(ret < 0){
		pr_err("Alloc chrdev failed\n");
		goto out;
	}

    pr_info("Device number <major>:<minor> = %d:%d\n",MAJOR(device_number),MINOR(device_number));

	/*2. Initialize the cdev structure with fops*/
	cdev_init(&lcd_cdev,&fops);

	/* 3. Register a device (cdev structure) with VFS */
	lcd_cdev.owner = THIS_MODULE;
	ret = cdev_add(&lcd_cdev,device_number,1);
	if(ret < 0){
		pr_err("Cdev add failed\n");
		goto unreg_chrdev;
	}

	/*4. create device class under /sys/class/ */
	class_lcd = class_create(THIS_MODULE,"lcd_class");
	if(IS_ERR(class_lcd)){
		pr_err("Class creation failed\n");
		ret = PTR_ERR(class_lcd);
		goto cdev_del;
	}

	/*5.  populate the sysfs with device information */
	device_lcd = device_create(class_lcd,NULL,device_number,NULL,"lcd");
	if(IS_ERR(device_lcd)){
		pr_err("Device create failed\n");
		ret = PTR_ERR(device_lcd);
		goto class_del;
	}

	pr_info("Module init was successful\n");

	return 0;

class_del:
	class_destroy(class_lcd);
cdev_del:
	cdev_del(&lcd_cdev);	
unreg_chrdev:
	unregister_chrdev_region(device_number,1);
out:
	pr_info("Module insertion failed\n");

    return ret;
}


void __exit cleanup_module(void){
    device_destroy(class_lcd,device_number);
	class_destroy(class_lcd);
	cdev_del(&lcd_cdev);	
	unregister_chrdev_region(device_number,1);
    spi_unregister_driver(&spi_lcd_driver);
    pr_info("module unloaded\n");
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("daovantruong121102@gmail.com");
MODULE_DESCRIPTION("lcd 5110 kernel module");
MODULE_VERSION("1.0");