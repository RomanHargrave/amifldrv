ifndef KERNEL
KERNEL = /lib/modules/$(shell uname -r)/build
endif
EXTRA_CFLAGS := -Wall -Wstrict-prototypes -O2 -fno-strict-aliasing
obj-m			+=	amifldrv_mod.o
amifldrv_mod-objs	:=	amifldrv.o amiwrap.o
default:
	make -C $(KERNEL) SUBDIRS=$(PWD) modules
	rm -f amifldrv.o_shipped
	mv amifldrv.o amifldrv.o_shipped
	rm -f amifldrv_mod.o
	mv amifldrv_mod.ko ../amifldrv_mod
.PHONY : clean
clean:
	rm -f *.*~
	rm -f test
	rm -f .*cmd
	rm -f *.o
	rm -f *mod*
	rm -f *.ko
	rm -f amifldrv.o_shipped
	rm -rf .tmp_versions/
