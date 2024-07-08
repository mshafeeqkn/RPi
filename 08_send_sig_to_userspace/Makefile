obj-m = sig_module.o
app-src = sig_app.c

KVERSION = $(shell uname -r)

all:
	gcc -o sig_app $(app-src)
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

clean:
	rm -rf sig_app
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
