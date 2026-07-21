#include <stdint.h>

volatile uint32_t counter = 7;   /* initialized  -> lands in .data */
volatile uint32_t flag;          /* zero-init    -> lands in .bss  */

int main(void)
{
    counter++;
    flag = 1;
    while (1) { }
}
