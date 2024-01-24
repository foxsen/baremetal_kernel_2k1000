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

### 其他渠道

可以把内核放到U盘或者ssd硬盘等，然后灵活应用uboot的load*命令和go命令装载运行。也可以尝试用u-boot提供的mkimage工具把elf文件打包成u-boot认识的uImage镜像。

## u-boot和内核的交互

BIOS与内核的交互接口是计算机系统中最重要的接口之一，它约定了bios如何与内核进行信息交互，传递内核命令行参数以及当前硬件的各种配置（内存范围、硬件设备类型、中断、资源分配等），详细情况可以参见龙芯BIOS和内核接口规范。

u-boot缺省设置了两个直接内存映射窗口(即不通过TLB翻译，虚拟地址与物理地址线性映射)，虚地址0x800000xxxxxxxxxx将映射到物理地址xxxxxxxxxx，而且采用uncache访问，虚地址0x900000xxxxxxxxxx映射到物理地址xxxxxxxxxx，用cached访问。内核可以用前者访问IO，后者用来访问常规内存。





