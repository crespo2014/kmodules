
#use make file=module1

obj-m:=${file}.o

all:
	make -C /usr/src/linux-headers-`uname -r` M=$(CURDIR) modules
