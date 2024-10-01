obj-m := pcd_device_setup.o pcd_platform_driver.o

KERN_DIR=/home/furky/workspace/ldd/source/linux/

all:
	make -C $(KERN_DIR) M=$(PWD) modules
clean:
	make -C $(KERN_DIR) M=$(PWD) clean
help:
	make -C $(KERN_DIR) M=$(PWD) help