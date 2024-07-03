obj-m = poll_module.o
app-src = poll_app.c

KVERSION = $(shell uname -r)
all:
  gcc -o poll_app $(app-src)
  make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
  rm -rf poll_app
  make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
