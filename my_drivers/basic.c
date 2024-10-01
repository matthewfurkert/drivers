// SPDX-License-Identifier: GPL-2.0
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>

#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt, __func__

/*declaring the memory size*/
#define NO_DEVICES 4
#define mem_size_pcdev1 512
#define mem_size_pcdev2 512
#define mem_size_pcdev3 512
#define mem_size_pcdev4 512

/*allocting the device buffer space*/
char dev_buff_pcdev1[mem_size_pcdev1];
char dev_buff_pcdev2[mem_size_pcdev2];
char dev_buff_pcdev3[mem_size_pcdev3];
char dev_buff_pcdev4[mem_size_pcdev4];

struct pcdev_private_data {
	char *buffer;
	unsigned int size;
	const char *serial_number;
	int perm;
	struct cdev cdev;
};

struct pcdrv_private_data {
	int total_devices;
	struct pcdev_private_data pcdev_data[NO_DEVICES];
	dev_t dev_num;
	struct class *class_pcd;
	struct device *device_pcd;
};

#define RDONLY 0x01
#define WRONLY 0x10
#define RDWR 0x11

struct pcdrv_private_data pcdrv_data = {
	.total_devices = NO_DEVICES,
	.pcdev_data = {
		[0] = {
			.buffer = dev_buff_pcdev1,
			.size = mem_size_pcdev1,
			.serial_number = "pcdev1ABC121",
			.perm = RDONLY},
		[1] = {
			.buffer = dev_buff_pcdev2,
			.size = mem_size_pcdev2,
			.serial_number = "pcdev2ABC122",
			.perm = WRONLY},
		[2] = {
			.buffer = dev_buff_pcdev3,
			.size = mem_size_pcdev3,
			.serial_number = "pcdev3ABC123",
			.perm = RDWR},
		[3] = {
			.buffer = dev_buff_pcdev3,
			.size = mem_size_pcdev3,
			.serial_number = "pcdev4ABC124",
			.perm = RDWR},

	}
};

int error_check(int perm, int access)
{
	if (perm == RDWR)
		return 0;
	if ((perm == RDONLY) && (access & FMODE_READ) && !(access & FMODE_WRITE))
		return 0;
	if ((perm == WRONLY) && (access & FMODE_WRITE) && !(access & FMODE_READ))
		return 0;
	return -EPERM;
}

int pcd_open(struct inode *inode, struct file *filp)
{
	int ret;
	struct pcdev_private_data *pcdev_data;

	pr_info("Opening Device: %d", MINOR(inode->i_rdev));

	/*adding the device private data to the file object*/
	pcdev_data = container_of(inode->i_cdev, struct pcdev_private_data, cdev);
	filp->private_data = pcdev_data;

	ret = error_check(pcdev_data->perm, filp->f_mode);

	pr_info("file opened\n");
	return ret;
}
loff_t pcd_llseek(struct file *filp, loff_t off, int whence)
{
	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)(filp->private_data);
	int max_size = pcdev_data->size;
	loff_t temp;

	switch (whence) {
	case SEEK_SET:
		if ((off > max_size) || (off < 0))
			return -EINVAL;
		filp->f_pos = off;
		break;
	case SEEK_CUR:
		temp = filp->f_pos + off;
		if ((temp > max_size) || (temp < 0))
			return -EINVAL;
		filp->f_pos = temp;
		break;
	case SEEK_END:
		temp = max_size + off;
		if ((temp > max_size) || (temp < 0))
			return -EINVAL;
		filp->f_pos = temp;
		break;
	default:
		return -EINVAL;
	}

	pr_info("file_position_changed\n");
	return filp->f_pos;
}
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)(filp->private_data);
	int max_size = pcdev_data->size;

	if ((*f_pos + count) > max_size)
		count = max_size - *f_pos;
	if (copy_to_user(buff, pcdev_data->buffer, count))
		return -EFAULT;
	*f_pos += count;


	pr_info("read complete\n");
	return count;
}
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)(filp->private_data);
	int max_size = pcdev_data->size;

	if ((*f_pos + count) > max_size)
		count = max_size - *f_pos;
	if (!count)
		return -ENOMEM;

	if (copy_from_user(pcdev_data->buffer, __user buff, count))
		return -EFAULT;
	*f_pos += count;

	pr_info("write_complete\n");
	return count;
}
int pcd_release(struct inode *inode, struct file *filp)
{
	pr_info("file released\n");
	return 0;
}

const struct file_operations f_ops = {
	.open = pcd_open,
	.llseek = pcd_llseek,
	.read = pcd_read,
	.write = pcd_write,
	.release = pcd_release,
	.owner = THIS_MODULE
};

static int __init pcd_driver_init(void)
{
	int i;

	if (alloc_chrdev_region(&pcdrv_data.dev_num, 0, NO_DEVICES, "pcd_devices"))
		return -EINVAL;

	pcdrv_data.class_pcd = class_create(THIS_MODULE, "pcd_devs");
	if (IS_ERR(pcdrv_data.class_pcd)) {
		unregister_chrdev_region(pcdrv_data.dev_num, NO_DEVICES);
		return PTR_ERR(pcdrv_data.class_pcd);
	}

	for (i = 0; i < NO_DEVICES; i++) {
		cdev_init(&pcdrv_data.pcdev_data[i].cdev, &f_ops);

		pcdrv_data.pcdev_data[i].cdev.owner = THIS_MODULE;
		if (cdev_add(&pcdrv_data.pcdev_data[i].cdev, pcdrv_data.dev_num + i, 1)) {
			class_destroy(pcdrv_data.class_pcd);
			unregister_chrdev_region(pcdrv_data.dev_num, NO_DEVICES);
			return -EINVAL;
		}

		pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, NULL, pcdrv_data.dev_num + i, NULL, "pcd-dev%d", i + 1);
		if (IS_ERR(pcdrv_data.device_pcd)) {
			for (; i > 0; i--)
				cdev_del(&pcdrv_data.pcdev_data[i].cdev);
			class_destroy(pcdrv_data.class_pcd);
			unregister_chrdev_region(pcdrv_data.dev_num, NO_DEVICES);
			return PTR_ERR(pcdrv_data.device_pcd);
		}
	}

	pr_info("module successfully installed\n");
	return 0;
}

static void __exit pcd_driver_exit(void)
{
	int i;

	for (i = 0; i < NO_DEVICES; i++) {
		device_destroy(pcdrv_data.class_pcd, pcdrv_data.dev_num + i);
		cdev_del(&pcdrv_data.pcdev_data[i].cdev);
	}

	class_destroy(pcdrv_data.class_pcd);
	unregister_chrdev_region(pcdrv_data.dev_num, NO_DEVICES);

	pr_info("module successfully removed\n");
}

module_init(pcd_driver_init);
module_exit(pcd_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FURKY");
MODULE_DESCRIPTION("Attempt 4 at writing a pseudo character driver");
MODULE_VERSION("1.2");
