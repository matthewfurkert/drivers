// SPDX-License-Identifier: GPL-2.0
#include<linux/module.h>
#include<linux/platform_device.h>
#include"platform.h"

#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt, __func__

void pcdev_release(struct device *dev)
{
	pr_info("device_released\n");
}

/*creating platform data*/
struct pcdev_platform_data pcdev_pdata[4] = {
	[0] = {
		.size = 512,
		.perm = RDWR,
		.serial_number = "pcdev1ABC121"
	},
	[1] = {
		.size = 1024,
		.perm = RDWR,
		.serial_number = "pcdev2ABC122"
	},
	[2] = {
		.size = 1024,
		.perm = RDWR,
		.serial_number = "pcdev3ABC123"
	},
	[3] = {
		.size = 1024,
		.perm = RDWR,
		.serial_number = "pcdev4ABC124"
	}
};

/*creating platform devices*/
struct platform_device pcdev1 = {
	.name = "pseudo-char-device",
	.id = 0,
	.dev = {
		.platform_data = &pcdev_pdata[0],
		.release = pcdev_release
	}
};

struct platform_device pcdev2 = {
	.name = "pseudo-char-device",
	.id = 1,
	.dev = {
		.platform_data = &pcdev_pdata[1],
		.release = pcdev_release
	}
};

struct platform_device pcdev3 = {
	.name = "pseudo-char-device",
	.id = 2,
	.dev = {
		.platform_data = &pcdev_pdata[2],
		.release = pcdev_release
	}
};

struct platform_device pcdev4 = {
	.name = "pseudo-char-device",
	.id = 3,
	.dev = {
		.platform_data = &pcdev_pdata[3],
		.release = pcdev_release
	}
};

static int __init pcdev_platform_init(void)
{
	platform_device_register(&pcdev1);
	platform_device_register(&pcdev2);
	platform_device_register(&pcdev3);
	platform_device_register(&pcdev4);
	return 0;
}

static void __exit pcdev_platform_exit(void)
{
	platform_device_unregister(&pcdev1);
	platform_device_unregister(&pcdev2);
	platform_device_unregister(&pcdev3);
	platform_device_unregister(&pcdev4);
	pr_info("leaving device setup\n");
}


module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);

MODULE_LICENSE("GPL");
