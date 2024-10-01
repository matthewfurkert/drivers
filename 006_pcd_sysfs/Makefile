obj-m := pcd_sysfs.o
pcd_sysfs-objs += pcd_platform_driver_dt_sysfs.o pcd_syscalls.o

KERN_DIR=/home/furky/workspace/ldd/source/linux/

all:
	make -C $(KERN_DIR) M=$(PWD) modules
clean:
	make -C $(KERN_DIR) M=$(PWD) clean
help:
	make -C $(KERN_DIR) M=$(PWD) help
copy-dtb:
	scp /home/furky/workspace/ldd/source/linux/arch/arm/boot/dts/am335x-boneblack.dtb debian@192.168.2.102:/home/debian/drivers
copy-drv:
	scp *.ko debian@192.168.2.102:/home/debian/drivers