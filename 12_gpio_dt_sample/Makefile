obj-m += gpio_dt_module.o
dts-file = gpio_dt_module.dt

KVERSION = $(shell uname -r)

all: module dt
	@printf "\033[38;5;42mBuilt the kernel and device tree binary\n\033[00m"

module:
	@make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

dt: $(dts-file)s
	@dtc -@ -I dts -O dtb -o $(dts-file)bo $(dts-file)s

clean:
	rm -rf \.*.cmd Module.symvers *.mod* *.o*

cleanall:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
	rm -rf $(dts-file)bo
