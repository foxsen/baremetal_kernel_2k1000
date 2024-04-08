/* minimal kernel for loongarch64
 * it print out 'hello kernel' in the serial port
 */

#include "serial.h"

void kernel_entry(void *a0, void *a1, void *a2)
{
    puts("hello kernel!\n");

    while(1);
}
