MODULES := System/System DataLinkLayer/DataLinkLayer Run/Run
obj-m := $(addsuffix .o, $(MODULES))
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

load:
	insmod System/System.ko
	insmod DataLinkLayer/DataLinkLayer.ko
	insmod Run/Run.ko

unload:
	@echo "Unloading modules..."
	@rmmod Run
	@rmmod DataLinkLayer
	@rmmod System

spy:
	dmesg -w


push:
	@echo "Enter commit message:"; \
	read msg; \
	if [ -n "$$msg" ]; then \
		git add . && \
		git commit -m "$$msg" && \
		git push -u origin main; \
	else \
		echo "Commit cancelled: no message provided."; \
	fi
