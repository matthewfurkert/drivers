obj-m := main.o

KERN_DIR=/home/furky/workspace/ldd/source/linux/

all:
	make -C $(KERN_DIR) M=$(PWD) modules
clean:
	make -C $(KERN_DIR) M=$(PWD) clean
help:
	make -C $(KERN_DIR) M=$(PWD) help