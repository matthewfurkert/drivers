// SPDX-License-Identifier: GPL-2.0
#include<linux/module.h>
#include<linux/device.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/mod_devicetable.h>
#include<linux/platform_device.h>
#include<linux/of.h>


#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt, __func__

#define MAX_DEVICES 10

struct device_platform_data {
	int size;
	int perm;
	const char *serial_num;
};
struct driver_device_data {
	struct device_platform_data pdata;
	dev_t dev_num;
	char *buffer;
	struct cdev cdev;
};
struct driver_private_data {
	int total_devices;
	dev_t dev_num_base;
	struct class *class_pcd;
	struct device *device_pcd;
};
struct driver_private_data drv_data;

int pcdopen(struct inode *inode, struct file *filp)
{
	return 0;
}
loff_t pcdllseek(struct file *filp, loff_t off, int whence)
{
	return 0;
}
ssize_t pcdread(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
	return 0;
}
ssize_t pcdwrite(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
	return 0;
}
int pcdrelease(struct inode *inode, struct file *filp)
{
	return 0;
}

const struct file_operations f_ops = {
	.open = pcdopen,
	.llseek = pcdllseek,
	.read = pcdread,
	.write = pcdwrite,
	.release = pcdrelease,
	.owner = THIS_MODULE
};

struct device_platform_data *get_platform_data(struct device *dev)
{
	struct device_node *dev_node = dev->of_node;
	struct device_platform_data *pdata;

	if (!dev_node)
		return NULL;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		dev_info(dev, "Unable to allocate memory for platform data\n");
		return ERR_PTR(-ENOMEM);
	}

	if (of_property_read_u32(dev_node, "org,size", &pdata->size)) {
		dev_info(dev, "Unable to extract size property\n");
		return ERR_PTR(-EINVAL);
	}
	if (of_property_read_string(dev_node, "org,device-serial-num", &pdata->serial_num)) {
		dev_info(dev, "Unable to extract serial number property\n");
		return ERR_PTR(-EINVAL);
	}
	if (of_property_read_u32(dev_node, "org,perm", &pdata->perm)) {
		dev_info(dev, "Unable to extract permission property\n");
		return ERR_PTR(-EINVAL);
	}

	return pdata;

}

int my_driver_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_platform_data *pdata;
	struct driver_device_data *dev_data;

	dev_info(dev, "A device match has been found\n");
	/*import the platform data*/
	pdata = get_platform_data(dev);

	/*allocate memory for the device data*/
	dev_data = devm_kzalloc(dev, sizeof(*dev_data), GFP_KERNEL);
	if (!dev) {
		dev_info(dev, "Unable to allocate memory for device data\n");
		return -ENOMEM;
	}

	/*adding the dev_data pointer to struct device*/
	dev_set_drvdata(dev, dev_data);

	/*add platform data to dev data*/
	dev_data->pdata.size = pdata->size;
	dev_data->pdata.serial_num = pdata->serial_num;
	dev_data->pdata.perm = pdata->perm;

	/*allocate memory for the device buffer*/
	dev_data->buffer = devm_kzalloc(dev, dev_data->pdata.size, GFP_KERNEL);
	if (!dev_data->buffer) {
		dev_info(dev, "Unable to allocate memory for device buffer");
		return -ENOMEM;
	}

	/*add the cdev data*/
	cdev_init(&dev_data->cdev, &f_ops);
	dev_data->cdev.owner = THIS_MODULE;

	dev_data->dev_num = drv_data.dev_num_base + drv_data.total_devices;
	if (cdev_add(&dev_data->cdev, dev_data->dev_num, 1))
		return -EINVAL;

	/*create the device*/
	drv_data.device_pcd = device_create(drv_data.class_pcd, dev, dev_data->dev_num, NULL, "my-dev%d", drv_data.total_devices + 1);
	if (IS_ERR(drv_data.device_pcd)) {
		cdev_del(&dev_data->cdev);
		return PTR_ERR(drv_data.device_pcd);
	}

	drv_data.total_devices++;

	return 0;
}

int my_driver_remove(struct platform_device *pdev)
{
	struct driver_device_data *dev_data = dev_get_drvdata(&pdev->dev);

	device_destroy(drv_data.class_pcd, dev_data->dev_num);
	cdev_del(&dev_data->cdev);

	drv_data.total_devices--;
	return 0;
}

const struct of_device_id device_tree_match[] = {
	{.compatible = "pcdev-A1X"},
	{.compatible = "pcdev-B1X"},
	{.compatible = "pcdev-C1X"},
	{.compatible = "pcdev-D1X"},
	{}
};


struct platform_driver pcd_driver = {
	.probe = my_driver_probe,
	.remove = my_driver_remove,
	.driver = {
		.name = "My Platform Driver",
		.of_match_table = device_tree_match
	}
};


static int __init platform_driver_init(void)
{
	if (alloc_chrdev_region(&drv_data.dev_num_base, 0, MAX_DEVICES, "my_devices"))
		return -EINVAL;

	drv_data.class_pcd = class_create(THIS_MODULE, "class_devs");
	if (IS_ERR(drv_data.class_pcd)) {
		unregister_chrdev_region(drv_data.dev_num_base, MAX_DEVICES);
		return PTR_ERR(drv_data.class_pcd);
	}

	platform_driver_register(&pcd_driver);

	pr_info("platform driver module has been installed\n");
	return 0;
}

static void __exit platform_driver_exit(void)
{
	platform_driver_unregister(&pcd_driver);
	class_destroy(drv_data.class_pcd);
	unregister_chrdev_region(drv_data.dev_num_base, MAX_DEVICES);
	pr_info("platorm driver module has been removed\n");
}

module_init(platform_driver_init);
module_exit(platform_driver_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Platform Driver Practice");
MODULE_AUTHOR("Matthew");
MODULE_VERSION("1.1");
