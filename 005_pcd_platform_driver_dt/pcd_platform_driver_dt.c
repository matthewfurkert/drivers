// SPDX-License-Identifier: GPL-2.0
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
#include<linux/platform_device.h>
#include "platform.h"
#include<linux/slab.h>
#include<linux/mod_devicetable.h>
#include<linux/of.h>
#include<linux/of_device.h>

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt, __func__

struct device_config {
	int config_item1;
	int config_item2;
};

enum pcdev_names {
	PCDEVA1X,
	PCDEVB1X,
	PCDEVC1X,
	PCDEVD1X
};

struct device_config pcdev_config[] = {
	[PCDEVA1X] = {.config_item1 = 60, .config_item2 = 21},
	[PCDEVB1X] = {.config_item1 = 50, .config_item2 = 22},
	[PCDEVC1X] = {.config_item1 = 40, .config_item2 = 23},
	[PCDEVD1X] = {.config_item1 = 30, .config_item2 = 24}
};

/*Device private data structure*/
struct pcdev_private_data {
	struct pcdev_platform_data pdata;
	char *buffer;
	dev_t dev_num;
	struct cdev cdev;
};

/*Driver private data structure*/
struct pcdrv_private_data {
	int total_devices;
	dev_t device_num_base;
	struct class *class_pcd;
	struct device *device_pcd;
}
;
struct pcdrv_private_data pcdrv_data;

int check_permission(int dev_perm, int acc_mode)
{
	if (dev_perm == RDONLY)
		return 0;
	/*Ensure read only access*/
	if ((dev_perm == RDONLY) && ((acc_mode & FMODE_READ) && !(acc_mode & FMODE_WRITE)))
		return 0;
	/*Ensures write only access*/
	if ((dev_perm == WRONLY) && ((acc_mode & FMODE_WRITE) && !(acc_mode & FMODE_READ)))
		return 0;
	return -EPERM;
}


loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{

	return 0;
}
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{

	return 0;
}
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{

	return -ENOMEM;
}

int pcd_open(struct inode *inode, struct file *filp)
{

	return 0;
}
int pcd_release(struct inode *inode, struct file *filp)
{
	pr_info("release was successful\n");
	return 0;
}

/* file operations of the driver*/
struct file_operations pcd_fops = {
	.open = pcd_open,
	.release = pcd_release,
	.read = pcd_read,
	.write = pcd_write,
	.llseek = pcd_lseek,
	.owner = THIS_MODULE
};
/*gets called when the device is removed from the system*/
int pcd_platform_driver_remove(struct platform_device *pdev)
{
#if 1
	struct pcdev_private_data *dev_data = dev_get_drvdata(&pdev->dev);
	/*1. Remove a device that was created with device create*/
	device_destroy(pcdrv_data.class_pcd, dev_data->dev_num);
	/*2. Remove a cdev entry from the system*/
	cdev_del(&dev_data->cdev);
	pcdrv_data.total_devices--;
#endif
	dev_info(&pdev->dev, "A device is removed\n");
	return 0;
}

struct pcdev_platform_data *pcdev_get_platdata_from_dt(struct device *dev)
{
	struct device_node *dev_node = dev->of_node;
	struct pcdev_platform_data *pdata;

	if (!dev_node)
		/* this probe didn't happen because of device tree node*/
		return NULL;
	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);

	if (!pdata) {
		dev_info(dev, "Cannot allocate dev memory\n");
		return ERR_PTR(-ENOMEM);
	}
	if (of_property_read_string(dev_node, "org,device-serial-num", &pdata->serial_number)) {
		dev_info(dev, "Missing serial number property");
		return ERR_PTR(-EINVAL);
	}
	if (of_property_read_u32(dev_node, "org,size", &pdata->size))
		dev_info(dev, "Missing size property");

	if (of_property_read_u32(dev_node, "org,perm", &pdata->perm))
		dev_info(dev, "Missing permission property");

	return pdata;
}

struct of_device_id org_pcdev_dt_match[];

/*gets called when a matched platform device is found*/
int pcd_platform_driver_probe(struct platform_device *pdev)
{
	int ret;
	int driver_data;
	struct pcdev_private_data *dev_data;
	struct pcdev_platform_data *pdata;
	struct device *dev = &pdev->dev;

	/*used to store matched entry of 'of_device_id' list of this driver*/
	const struct of_device_id *match;

	dev_info(dev, "A device is detected\n");

	/*match will always be null if linux doesn't support device tree i.e. CONFIG_OF is off*/
	match = of_match_device(of_match_ptr(org_pcdev_dt_match), dev);
	if (match) {
		pdata = pcdev_get_platdata_from_dt(dev);
		if (IS_ERR(pdata))
			return PTR_ERR(pdata);
		driver_data = (int)match->data;
	}	else {
			pdata = (struct pcdev_platform_data *)dev_get_platdata(dev);\
			driver_data = pdev->id_entry->driver_data;
	}

	/*1. Get the platfom data*/
	if (!pdata) {
		pr_info("no platform data available");
		return -EINVAL;
	}

	/*2. Dynamically allocate the memory for the device private data*/
	dev_data = devm_kzalloc(&pdev->dev, sizeof(*dev_data), GFP_KERNEL);
	if (!dev_data) {
		pr_info("Cannot allocate memory\n");
		return -ENOMEM;
	}

	/*save the device private data pointer in platform device structure*/
	dev_set_drvdata(&pdev->dev, dev_data);

	dev_data->pdata.size = pdata->size;
	dev_data->pdata.perm = pdata->perm;
	dev_data->pdata.serial_number = pdata->serial_number;

	pr_info("Device serial number = %s\n", dev_data->pdata.serial_number);
	pr_info("Device size = %d\n", dev_data->pdata.size);
	pr_info("Device permission = %d\n", dev_data->pdata.perm);

	pr_info("Config item 1 = %d\n", pcdev_config[driver_data].config_item1);
	pr_info("Config item 2 = %d\n", pcdev_config[driver_data].config_item2);

	/*3. Dynamically allocate the memory for the device buffer using size information from the platform data*/
	dev_data->buffer = devm_kzalloc(&pdev->dev, dev_data->pdata.size, GFP_KERNEL);
	if (!dev_data->buffer) {
		pr_info("Cannot allocate memory\n");
		return -ENOMEM;
	}
	/*4. Get the device number*/
	dev_data->dev_num = pcdrv_data.device_num_base + pcdrv_data.total_devices;
	/*5. Do cdev init and cdev add*/
	cdev_init(&dev_data->cdev, &pcd_fops);

	dev_data->cdev.owner = THIS_MODULE;
	ret = cdev_add(&dev_data->cdev, dev_data->dev_num, 1);
	if (ret < 0) {
		pr_err("Cdev add failed\n");
		return ret;
	}
	/*6. Create device files for the detected platfrom device*/
	pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, dev, dev_data->dev_num, NULL, "pcdev-%d", pcdrv_data.total_devices);
	if (IS_ERR(pcdrv_data.device_pcd)) {
		pr_err("Device creation failed\n");
		ret = PTR_ERR(pcdrv_data.device_pcd);
		cdev_del(&dev_data->cdev);
		return ret;
	}

	pcdrv_data.total_devices++;

	pr_info("Probe was successful\n");

	return 0;
}

struct platform_device_id pcdevs_ids[] = {
	{.name = "pcdev-A1x", .driver_data = PCDEVA1X},
	{.name = "pcdev-B1x", .driver_data = PCDEVB1X},
	{.name = "pcdev-C1x", .driver_data = PCDEVD1X},
	{.name = "pcdev-D1x", .driver_data = PCDEVD1X},
	{}
};

struct of_device_id org_pcdev_dt_match[] = {
	{.compatible = "pcdev-A1x", .data = (void *)PCDEVA1X},
	{.compatible = "pcdev-B1x", .data = (void *)PCDEVB1X},
	{.compatible = "pcdev-C1x", .data = (void *)PCDEVC1X},
	{.compatible = "pcdev-D1x", .data = (void *)PCDEVD1X},
	{}
};
struct platform_driver pcd_platform_driver = {
	.probe = pcd_platform_driver_probe,
	.remove = pcd_platform_driver_remove,
	.id_table = pcdevs_ids,
	.driver = {
		.name = "pseudo-char-device",
		.of_match_table = org_pcdev_dt_match
	}
};

#define MAX_DEVICES 10

static int __init pcd_platform_driver_init(void)
{
	int ret;
	/*1. Dynamically allocate a device number for MAX_DEVICES*/
	ret = alloc_chrdev_region(&pcdrv_data.device_num_base, 0, MAX_DEVICES, "pcdevs");
	if (ret < 0) {
		pr_err("Alloc chrdev failed\n");
		return ret;
	}

	/*2. Create device class under /sys/class*/
	pcdrv_data.class_pcd = class_create(THIS_MODULE, "pcd_class");
	if (IS_ERR(pcdrv_data.class_pcd)) {
		pr_err("class creation failed\n");
		ret = PTR_ERR(pcdrv_data.class_pcd);
		unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);
		return ret;
	}
	/*3. Register a platform device*/
	platform_driver_register(&pcd_platform_driver);
	pr_info("Device setup module loaded\n");
	return 0;
}

static void __exit pcd_platform_driver_cleanup(void)
{
	/*1. Unregister platform driver*/
	platform_driver_unregister(&pcd_platform_driver);
	/*2. class destroy*/
	class_destroy(pcdrv_data.class_pcd);
	/*3. unregister device numbers for max_devices*/
	unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);

	pr_info("Device setup module unloaded\n");
}

module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FURKY");
MODULE_DESCRIPTION("A pseudo character platform driver which handles n platform pcdevs");
