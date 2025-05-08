
obj-m += hello-world.o

KERNEL_VERSION=$(shell uname -r)

all:
	make -C /lib/modules/${KERNEL_VERSION}/build M=$(PWD) modules

clean:
	make -C /lib/modules/${KERNEL_VERSION}/build M=$(PWD) clean

re: clean all