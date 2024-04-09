/* minimal kernel for loongarch64
 * it demo the usage of timer interrupt
 */

#include "serial.h"

extern void timer_trap_init(void);

void kernel_entry(void *a0, void *a1, void *a2)
{
    puts("timer interrupt kernel!\n");

    timer_trap_init();

    while(1);
}
