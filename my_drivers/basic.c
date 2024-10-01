// SPDX-License-Identifier: GPL-2.0
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>

#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt, __func__

#define NO_OF_DEVICES 4

#define mem_size_max_pcdev1 512
#define mem_size_max_pcdev2 512
#define mem_size_max_pcdev3 512
#define mem_size_max_pcdev4 512
char dev_buff_pcdev1[mem_size_max_pcdev1];
char dev_buff_pcdev2[mem_size_max_pcdev2];
char dev_buff_pcdev3[mem_size_max_pcdev3];
char dev_buff_pcdev4[mem_size_max_pcdev4];

#define RDONLY	0x01
#define WRONLY	0x10
#define RDWR	0x11

struct pcdev_private_data {
	char *buffer;
	unsigned int size;
	const char *serial_number;
	int perm;
	struct cdev cdev;
};

struct pcdrv_private_data {
	int total_devices;
	struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
	dev_t dev_num;
	struct class *class_pcd;
	struct device *device_pcd;
};

struct pcdrv_private_data pcdrv_data = {
	.total_devices = NO_OF_DEVICES,
	.pcdev_data = {
		[0] = {
			.buffer = dev_buff_pcdev1,
			.size = mem_size_max_pcdev1,
			.serial_number = "PCDEV1ABC123",
			.perm = RDONLY
		},
		[1] = {
			.buffer = dev_buff_pcdev2,
			.size = mem_size_max_pcdev2,
			.serial_number = "PCDEV2ABC123",
			.perm = WRONLY
		},
		[2] = {
			.buffer = dev_buff_pcdev3,
			.size = mem_size_max_pcdev3,
			.serial_number = "PCDEV3ABC123",
			.perm = RDWR
		},
		[3] = {
			.buffer = dev_buff_pcdev4,
			.size = mem_size_max_pcdev4,
			.serial_number = "PCDEV4ABC123",
			.perm = RDWR
		}
	}
};

int check_perm(int dev_perm, int acc_mode)
{
	if (dev_perm == RDWR)
		return 0;
	/*Ensure read only access*/
	if ((dev_perm == RDONLY) && ((acc_mode & FMODE_READ) && !(acc_mode & FMODE_WRITE)))
		return 0;
	/*Ensures write only access*/
	if ((dev_perm == WRONLY) && ((acc_mode & FMODE_WRITE) && !(acc_mode & FMODE_READ)))
		return 0;
	return -EPERM;
}

int pcd_open(struct inode *inode, struct file *filp)
{
	struct pcdev_private_data *pcdev_data;
	int minor_n;
	int ret;

	minor_n = MINOR(inode->i_rdev);
	pr_info("Acessing device: %d\n", minor_n);

	pcdev_data = container_of(inode->i_cdev, struct pcdev_private_data, cdev);
	filp->private_data = pcdev_data;

	ret = check_perm(pcdev_data->perm, filp->f_mode);
	(!ret)?pr_info("open was successful\n"):pr_info("open failed\n");

	return ret;
}

loff_t pcd_llseek(struct file *filp, loff_t off, int whence)
{
	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
	int mem_size = pcdev_data->size;
	loff_t temp;

	switch (whence) {
	case SEEK_SET:
		if ((off > mem_size) || (off < 0))
			return -EINVAL;
		filp->f_pos = off;
		break;
	case SEEK_CUR:
		temp = filp->f_pos + off;
		if ((temp > mem_size) || (temp < 0))
			return -EINVAL;
		filp->f_pos = temp;
		break;
	case SEEK_END:
		temp = mem_size + off;
		if ((temp > mem_size) || (temp < 0))
			return -EINVAL;
		filp->f_pos = temp;
		break;
	default:
		break;
	}

	pr_info("llseek successful\n");
	return filp->f_pos;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
	int mem_size = pcdev_data->size;

	if ((*f_pos + count) > mem_size)
		count = mem_size - *f_pos;

	if (copy_to_user(buff, pcdev_data->buffer + *f_pos, count))
		return -EFAULT;

	*f_pos += count;

	pr_info("read call successful\n");
	return count;

}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
	int mem_size = pcdev_data->size;

	if ((*f_pos + count) > mem_size)
		count = mem_size - *f_pos;
	if (!count) {
		pr_err("No memory available\n");
		return -ENOMEM;
	}
	if (copy_from_user(pcdev_data->buffer + *f_pos, buff, count))
		return -EFAULT;
	*f_pos += count;

	pr_info("write call successful\n");
	return count;
}

int pcd_release(struct inode *inode, struct file *filp)
{
	pr_info("release successful\n");
	return 0;
}


const struct file_operations fops = {
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
	/*dynamically allocate device numbers*/
	if (alloc_chrdev_region(&pcdrv_data.dev_num, 0, NO_OF_DEVICES, "my_devices"))
		return -EFAULT;

	pcdrv_data.class_pcd = class_create(THIS_MODULE, "pcd_practice");
	if (IS_ERR(pcdrv_data.class_pcd)) {
		unregister_chrdev_region(pcdrv_data.dev_num, NO_OF_DEVICES);
		return PTR_ERR(pcdrv_data.class_pcd);
	}

	for (i = 0; i < NO_OF_DEVICES; i++) {
		cdev_init(&pcdrv_data.pcdev_data[i].cdev, &fops);
		pcdrv_data.pcdev_data[i].cdev.owner = THIS_MODULE;

		if (cdev_add(&pcdrv_data.pcdev_data[i].cdev, pcdrv_data.dev_num + i, 1)) {
			for (; i >= 0; i--)
				cdev_del(&pcdrv_data.pcdev_data[i].cdev);
			class_destroy(pcdrv_data.class_pcd);
			unregister_chrdev_region(pcdrv_data.dev_num, NO_OF_DEVICES);
			return -EFAULT;
		}

		pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, NULL, pcdrv_data.dev_num + i, NULL, "pcd_dev%d", i+1);
		if (IS_ERR(pcdrv_data.device_pcd)) {
			for (; i >= 0; i--) {
				device_destroy(pcdrv_data.class_pcd, pcdrv_data.dev_num + i);
				cdev_del(&pcdrv_data.pcdev_data[i].cdev);
			}
			class_destroy(pcdrv_data.class_pcd);
			unregister_chrdev_region(pcdrv_data.dev_num, NO_OF_DEVICES);
			return PTR_ERR(pcdrv_data.device_pcd);
		}
	}

	pr_info("driver sucessfully loaded\n");
	return 0;
}

static void __exit pcd_driver_exit(void)
{
	int i;

	for (i = 0; i < NO_OF_DEVICES; i++) {
		device_destroy(pcdrv_data.class_pcd, pcdrv_data.dev_num + i);
		cdev_del(&pcdrv_data.pcdev_data[i].cdev);
	}
	class_destroy(pcdrv_data.class_pcd);
	unregister_chrdev_region(pcdrv_data.dev_num, NO_OF_DEVICES);

	pr_info("driver removed\n");
}

module_init(pcd_driver_init);
module_exit(pcd_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FURKY");
MODULE_DESCRIPTION("Attempt 3 at writing a pseudo character driver");
MODULE_VERSION("1.2");
