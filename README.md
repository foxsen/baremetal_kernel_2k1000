# baremetal kernel

这是一个最小化的内核态运行的程序，它只是启动后打印一个'hello kernel'字符串。严格来说，它并不是启动就运行的真正意义baremetal，而是依赖u-boot装载运行的简单内核。但测试真正的baremetal内核需要有烧录flash的工具，不适合大部分同学。

它利用u-boot提供的文件装载和运行功能来实现内核启动，并没用任何汇编代码。

## 代码编译

内核的编译和常规程序有所不同，这里我们用到的编译选项如下：

    CFLAGS = -Wall -O0 -g3 -march=loongarch64 -mabi=lp64s -ffreestanding -fno-common -nostdlib -I. -fno-stack-protector -fno-pie -no-pie 

其中:

* -Wall -O0 -g3是常规的警告信息、优化级别和调试级别选项，可以根据需要修改
* -march=loongarch64 -mabi=lp64s是指定相应的目标架构和ABI(lp64s表示64位、不带浮点支持的LA ABI)
* -ffreestanding选项是告诉编译器不要假设有标准库存在，程序也不一定从main()函数开始，它隐含了-fno-builtin，编译器不会自动识别一些内置的函数(如memcpy)
* -fnostdlib，不链接一些系统启动文件和库(如/crt*.o/libc/libgcc等)
* -fno-common，告诉编译器不要把未初始化的全局变量放到一个common block，而是放到BSS中
* -I. 允许编译器在当前目标找头文件
* -fno-stack-protector，不要自动生成栈保护（检测栈是否溢出等的代码）
* -fno-pie -no-pie，不要生成位置无关的可执行代码。内核的位置一般不会动，这个选项可能提升性能。但如果你的内核需要支持位置无关以提高安全性，则不用这些选项。

根据2k1000星云板的用户手册，下载交叉编译工具链，安装到/opt后，用命令source set_env.sh设置路径，然后make即可编译：

```bash
foxsen@fx:~/software/qemu-loongarch-runenv/mini_kernel1/tmp/baremetal_kernel_2k1000$ ls /opt/toolchain-loongarch64-linux-gnu-gcc8-host-x86_64-2022-07-18/
bin  init-bash  lib  libexec  loongarch64-linux-gnu  share  sysroot  versions
foxsen@fx:~/software/qemu-loongarch-runenv/mini_kernel1/tmp/baremetal_kernel_2k1000$ source set_env.sh
====>setup env for LoongArch...
foxsen@fx:~/software/qemu-loongarch-runenv/mini_kernel1/tmp/baremetal_kernel_2k1000$ make clean
rm -f *.o ./kernel ./kernel.bin
foxsen@fx:~/software/qemu-loongarch-runenv/mini_kernel1/tmp/baremetal_kernel_2k1000$ make
loongarch64-linux-gnu-gcc -Wall -O2 -g3 -march=loongarch64 -mabi=lp64s -ffreestanding -fno-common -nostdlib -I. -fno-stack-protector -fno-pie -no-pie  -c -o hello_kernel.o hello_kernel.c
loongarch64-linux-gnu-ld -z max-page-size=16384 -T ld.script -o kernel hello_kernel.o
loongarch64-linux-gnu-objcopy -O binary kernel kernel.bin
foxsen@fx:~/software/qemu-loongarch-runenv/mini_kernel1/tmp/baremetal_kernel_2k1000$ sudo cp kernel.bin /srv/tftp/
```

## 代码链接

Makefile中使用了定制化的链接脚本来链接内核的目标文件：

	$(LD) $(LDFLAGS) -T ld.script -o kernel $(OBJS)

ld.script指定了目标elf文件的入口为kernel_entry，约定了链接的起始地址，以及如何把各个目标文件的section组合起来等等。

## 运行测试

### 通过tftp从网络获取内核运行

启动uboot时，按c键停到u-boot命令行，然后设置网络地址（setenv ipaddr <2k1000 ip>; setenv gatewayip <gw ip>; saveenv; reboot）; 设置好tftp服务器（可参考2k1000开发板用户手册），将生成的kernel.bin拷贝到tftp服务器的相应目录（使用tftpd-hpa时缺省为/srv/tftp/），然后参考如下样子启动：

```bash
=> tftpboot 0x9000000008000000 192.168.31.128:kernel.bin
ethernet@40040000 Waiting for PHY auto negotiation to complete......... TIMEOUT !
Could not initialize PHY ethernet@40040000
Speed: 1000, full duplex
Using ethernet@40050000 device
TFTP from server 192.168.31.128; our IP address is 192.168.31.123
Filename 'kernel.bin'.
Load address: 0x9000000008000000
Loading: #
         1000 Bytes/s
done
Bytes transferred = 144 (90 hex)
=> go 0x9000000008000000
## Starting application at 0x9000000008000000 ...
hello kernel!
```

如果用[2k1000 QEMU模拟器](https://github.com/LoongsonLab/2k1000-materials/releases/download/qemu-static-20240401/qemu-static-20240401.tar.xz)运行，则可以根据其说明，在runqemu运行uboot时，长按c键停到uboot命令行，然后设置网络，用tftpboot命令。runqemu缺省用用户级网络模拟，主机侧的ip是10.0.2.2，模拟器侧可以设置为10.0.2.15等，一个样例如下：

```bash
foxsen@fx:~/software/2k1000/qemu$ ./runqemu
WARNING: Image format was not specified for './2k1000/u-boot-with-spl.bin' and probing guessed raw.
         Automatically detecting the format is dangerous for raw images, write operations on block 0 will be restricted.
         Specify the 'raw' format explicitly to remove the restrictions.
WARNING: Image format was not specified for '/tmp/disk' and probing guessed raw.
         Automatically detecting the format is dangerous for raw images, write operations on block 0 will be restricted.
         Specify the 'raw' format explicitly to remove the restrictions.
hda-duplex: hda_audio_init: cad 0
ram=0x3020760
length=852992 must be 16777216 bytes,run command:
trucate -s 16777216 file
 to resize file
oobsize = 64




qemu-system-loongarch64: warning: nic pci-synopgmac.1 has no peer
qemu-system-loongarch64: warning: nic e1000e.0 has no peer
hda-duplex: hda_audio_reset
hda-duplex: hda_audio_reset
intel-hda: intel_hda_update_irq: level 0 [intx]

 _     __   __  _  _  ___  ___  __  _  _    /   ___  __  \
 |    |  | |  | |\ | | __ [__  |  | |\ |    |  | __ |  \ |
 |___ |__| |__| | \| |__] ___] |__| | \|    \  |__] |__/ /

Trying to boot from SPI


U-Boot 2022.04 (Jan 26 2024 - 15:42:00 +0800)

CPU:   LA264
Speed: Cpu @ 900 MHz/ Mem @ 400 MHz/ Bus @ 125 MHz
Model: loongson-2k1000
Board: LS2K1000-DP
DRAM:  1 GiB
Core:  74 devices, 20 uclasses, devicetree: board
cam_disable:1, vpu_disable:1, pcie0_enable:0, pcie1_enable:1
Loading Environment from SPIFlash... SF: Detected gd25q128 with page size 256 Bytes, erase size 4 KiB, total 16 MiB
*** Warning - bad CRC, using default environment

Cannot get ddc bus
In:    serial
Out:   serial
Err:   serial vidconsole

eth0: using random MAC address - 36:91:dc:95:40:85

eth1: using random MAC address - e6:46:9b:00:0c:ce
Net:   eth0: ethernet@40040000, eth1: ethernet@40050000
************************** Notice **************************
Press c to enter u-boot console, m to enter boot menu
************************************************************
Bus otg@40000000: dwc2_usb otg@40000000: Core Release: 0.000
dwc2_usb otg@40000000: SNPSID invalid (not DWC2 OTG device): 00000000
Port not available.
Bus ehci@40060000: USB EHCI 1.00
Bus ohci@40070000: USB OHCI 1.0
scanning bus ehci@40060000 for devices... 3 USB Device(s) found
scanning bus ohci@40070000 for devices... 2 USB Device(s) found
init ls_trigger_boot and set it default value
init ls_trigger_u_kernel and set it default value
init ls_trigger_u_rootfs and set it default value
init ls_trigger_u_uboot and set it default value
Saving Environment to SPIFlash... Erasing SPI flash...Writing to SPI flash...done
OK
Autoboot in 0 seconds
=> setenv ip 10.0.2.15
=> setenv ipaddr 10.0.2.15
=> setenv gatewayip 10.0.2.2
=> tftpboot 0x9000000008000000 10.0.2.2:hello_kernel.bin
Speed: 1000, full duplex
Using ethernet@40040000 device
TFTP from server 10.0.2.2; our IP address is 10.0.2.15; sending through gateway 10.0.2.2
Filename 'hello_kernel.bin'.
Load address: 0x9000000008000000
Loading: #
	 21.5 KiB/s
done
Bytes transferred = 272 (110 hex)
=> go 0x9000000008000000
## Starting application at 0x9000000008000000 ...
hello kernel!
```

### 其他渠道

可以把内核放到U盘或者ssd硬盘等，然后灵活应用uboot的load*命令和go命令装载运行。也可以尝试用u-boot提供的mkimage工具把elf文件打包成u-boot认识的uImage镜像。

## u-boot和内核的交互

BIOS与内核的交互接口是计算机系统中最重要的接口之一，它约定了bios如何与内核进行信息交互，传递内核命令行参数以及当前硬件的各种配置（内存范围、硬件设备类型、中断、资源分配等），详细情况可以参见龙芯BIOS和内核接口规范。

u-boot缺省设置了两个直接内存映射窗口(即不通过TLB翻译，虚拟地址与物理地址线性映射)，虚地址0x800000xxxxxxxxxx将映射到物理地址xxxxxxxxxx，而且采用uncache访问，虚地址0x900000xxxxxxxxxx映射到物理地址xxxxxxxxxx，用cached访问。内核可以用前者访问IO以及需要绕过cache访问内存的场合，用后者经过cache访问内存。





