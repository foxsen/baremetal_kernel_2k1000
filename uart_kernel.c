/* minimal kernel for loongarch64
 * it demo the usage of timer interrupt
 */

#include "serial.h"

extern void uart_trap_init(void);

void kernel_entry(void *a0, void *a1, void *a2)
{
    puts("uart interrupt kernel!\n");

    uart_trap_init();

    while(1);
}
