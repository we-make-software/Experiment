MODULES := System/System SecondMemoryTimeout/SecondMemoryTimeout MinuteMemoryTimeout/MinuteMemoryTimeout Run/Run
obj-m := $(addsuffix .o, $(MODULES))
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

load:
	insmod System/System.ko
	insmod SecondMemoryTimeout/SecondMemoryTimeout.ko
	insmod MinuteMemoryTimeout/MinuteMemoryTimeout.ko
	insmod Run/Run.ko

unload:
	@echo "Unloading modules..."
	@rmmod Run
	@rmmod SecondMemoryTimeout
	@rmmod MinuteMemoryTimeout
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
