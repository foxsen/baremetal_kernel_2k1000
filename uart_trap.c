#include "printf.h"
#include "loongarch.h"

extern void trap_entry(void);

#define UART0_RBR 0x800000001fe20000
#define UART0_IER 0x800000001fe20001
#define UART0_IIR 0x800000001fe20002
#define UART0_LSR 0x800000001fe20005

#define INTISR0 0x800000001fe01420
#define INTEN0  0x800000001fe01424
#define INTENSET0 0x800000001fe01428
#define INTENCLR0 0x800000001fe0142c
#define ENTRY0 0x800000001fe01400

static char recv_buf[128] = {};
static int recv_index = 0;

static int int_status[1024][4];
static int int_index = 0;

void timer_interrupt(void)
{
    printf("timer %d\n", int_index);
    for (int i = 0; i < int_index; i++) {
        printf("isr=%x, en=%x, iir=%x, lsr=%x\n", int_status[i][0], int_status[i][1], int_status[i][2], int_status[i][3]);
    }

    recv_buf[recv_index] = 0;
    printf(recv_buf);

    int_index = 0;
    recv_index = 0;

    /* ack */
    w_csr_ticlr(r_csr_ticlr() | CSR_TICLR_CLR);
}

void uart_interrupt(void)
{
   unsigned int isr, en;
   unsigned char iir, lsr;
   volatile char c;

   isr = *(volatile unsigned int*)INTISR0;
   en = *(volatile unsigned int*)INTEN0;
   iir = *(volatile unsigned char*)UART0_IIR;
   lsr = *(volatile unsigned char*)UART0_LSR;

   int_status[int_index][0] = isr;
   int_status[int_index][1] = en;
   int_status[int_index][2] = iir;
   int_status[int_index][3] = lsr;
   if (int_index < 1023)
       int_index ++;

   c = *(volatile char*)UART0_RBR;
   recv_buf[recv_index++] = c;
   if (recv_index == 127) 
       recv_index = 0;

}

void trap_handler(void)
{
  unsigned long era = r_csr_era();
  unsigned long prmd = r_csr_prmd();
  unsigned int estat = r_csr_estat();
  unsigned int ecfg = r_csr_ecfg();

  if((prmd & PRMD_PPLV) != 0)
    printf("kerneltrap: not from privilege0");
  if(intr_get() != 0)
    printf("kerneltrap: interrupts enabled");

  if (estat & ecfg & TI_VEC) {
      timer_interrupt();
  } else if (estat & ecfg & (1 << 2)) {
      uart_interrupt();
  } else if (estat & ecfg) {
      printf("unexpected IRQ estat %x, ecfg %x\n", estat, ecfg);
      printf("era=%p eentry=%p\n", r_csr_era(), r_csr_eentry());
      while(1);
  }

  w_csr_era(era);
  w_csr_prmd(prmd);
}


void uart_trap_init(void)
{
  unsigned int ecfg = ( 0U << CSR_ECFG_VS_SHIFT ) | (1 << 2) | TI_VEC;
  unsigned long tcfg = 0x10000000UL | CSR_TCFG_EN | CSR_TCFG_PER;
  unsigned int isr, en;
  unsigned char iir, ier, lsr;

  /* configure uart interrupts */
  isr = *(volatile unsigned int*)INTISR0;
  en = *(volatile unsigned int*)INTEN0;
  iir = *(volatile unsigned char*)UART0_IIR;
  ier = *(volatile unsigned char*)UART0_IER;
  lsr = *(volatile unsigned char*)UART0_LSR;

  printf("before init isr=%x, en=%x, iir=%x, ier=%x, lsr=%x\n", isr, en, iir, ier, lsr);

  // enable uart0 RX interrupt in uart device 
  *(volatile unsigned char*)UART0_IER = 0x1;

  // enable uart0 interrupt on 2k1000 irqc
  *(volatile unsigned int*)INTENSET0 = 1;

  // 2k1000 irq0 route to core0 int0, which is related to bit2 of ECFG/ESTAT
  *(volatile unsigned int*)ENTRY0 = 0x10;

  // default to level trigger/fixed assignment mode

  w_csr_ecfg(ecfg);
  w_csr_tcfg(tcfg);
  w_csr_eentry((unsigned long)trap_entry);
  intr_on();
}
