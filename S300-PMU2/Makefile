ifeq ($(KERNELRELEASE),)

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

.PHONY: build clean

build:
        $(MAKE) -Wall -C $(KERNELDIR) M=$(PWD) modules

clean:
        rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c
else

$(info Building with KERNELRELEASE = ${KERNELRELEASE})
obj-m :=    crash1.o

endif
