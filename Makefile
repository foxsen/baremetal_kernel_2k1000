TOOLPREFIX = loongarch64-linux-gnu-

CC = $(TOOLPREFIX)gcc
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy

CFLAGS = -Wall -O2 -g3 -march=loongarch64 -mabi=lp64s -ffreestanding -fno-common -nostdlib -I. -fno-stack-protector -fno-pie -no-pie 
LDFLAGS = -z max-page-size=16384

hello_kernel: hello_kernel.o serial.o ld.script
	$(LD) $(LDFLAGS) -T ld.script -o hello_kernel hello_kernel.o serial.o
	$(OBJCOPY) -O binary hello_kernel hello_kernel.bin

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: kernel 

clean: 
	rm -f *.o ./kernel ./kernel.bin
