obj-m += dev_tree_probe.o
dts-file = device_tree.dt

KVERSION = $(shell uname -r)

all: module dt
	echo "Built the kernel and device tree binary"

module:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

dt: $(dts-file)s
	dtc -@ -I dts -O dtb -o $(dts-file)bo $(dts-file)s

clean:
	rm -rf \.*.cmd Module.symvers *.mod* *.o*

cleanall:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
	rm -rf $(dts-file)bo
