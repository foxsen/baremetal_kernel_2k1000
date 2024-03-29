/* minimal kernel for loongarch64
 * it print out 'hello kernel' in the serial port
 */

//static const unsigned long base = 0x900000001fe001e0ULL;
//2k1000
static unsigned long uart_base = 0x800000001fe20000ULL;
// 2k500
//static unsigned long uart_base = 0x800000001ff40800ULL;

#define UART0_THR  (uart_base + 0)
#define UART0_LSR  (uart_base + 5)
#define LSR_TX_IDLE  (1 << 5)

static char io_readb(unsigned long addr)
{
    return *(volatile char*)addr;
}

static void io_writeb(unsigned long addr, char c)
{
    *(char*)addr = c;
}

static void putc(char c)
{
    // wait for Transmit Holding Empty to be set in LSR.
    while((io_readb(UART0_LSR) & LSR_TX_IDLE) == 0);
    io_writeb(UART0_THR, c);
}

static void puts(char *str)
{
    while (*str != 0) {
        putc(*str);
        str++;
    }
}

void kernel_entry(void *a0, void *a1, void *a2)
{
    puts("hello kernel!\n");

    while(1);
}
