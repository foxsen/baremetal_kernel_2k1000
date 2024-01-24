OBJS := hello_kernel.o
TOOLPREFIX = loongarch64-linux-gnu-

CC = $(TOOLPREFIX)gcc
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy

CFLAGS = -Wall -O2 -g3 -march=loongarch64 -mabi=lp64s -ffreestanding -fno-common -nostdlib -I. -fno-stack-protector -fno-pie -no-pie 
LDFLAGS = -z max-page-size=16384

kernel: $(OBJS) ld.script
	$(LD) $(LDFLAGS) -T ld.script -o kernel $(OBJS)
	$(OBJCOPY) -O binary kernel kernel.bin

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: kernel 

clean: 
	rm -f *.o ./kernel ./kernel.bin
