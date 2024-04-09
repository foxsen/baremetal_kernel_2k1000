TOOLPREFIX = loongarch64-linux-gnu-

CC = $(TOOLPREFIX)gcc
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy

CFLAGS = -Wall -O2 -g3 -march=loongarch64 -mabi=lp64s -ffreestanding -fno-common -nostdlib -I. -fno-stack-protector -fno-pie -no-pie 
AFLAGS = -Wall -O2 -g3 -march=loongarch64 -mabi=lp64s -ffreestanding -fno-common -nostdlib -I. -fno-stack-protector -fno-pie -no-pie 
LDFLAGS = -z max-page-size=16384

all: hello_kernel timer_kernel uart_kernel

hello_kernel: hello_kernel.o serial.o ld.script
	$(LD) $(LDFLAGS) -T ld.script -o hello_kernel hello_kernel.o serial.o
	$(OBJCOPY) -O binary hello_kernel hello_kernel.bin

timer_kernel: timer_kernel.o serial.o printf.o trap_entry.o timer_trap.o ld.script
	$(LD) $(LDFLAGS) -T ld.script -o timer_kernel timer_kernel.o serial.o printf.o trap_entry.o timer_trap.o
	$(OBJCOPY) -O binary timer_kernel timer_kernel.bin

uart_kernel: uart_kernel.o serial.o printf.o trap_entry.o uart_trap.o ld.script
	$(LD) $(LDFLAGS) -T ld.script -o uart_kernel uart_kernel.o serial.o printf.o trap_entry.o uart_trap.o
	$(OBJCOPY) -O binary uart_kernel uart_kernel.bin

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o : %.S
	$(CC) $(AFLAGS) -c -o $@ $<


clean: 
	rm -f *.o ./*kernel ./*kernel.bin
