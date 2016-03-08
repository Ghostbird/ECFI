#include "cfi-heuristics.h"
#include <stdio.h>     /* print() */
#include <inttypes.h>  /* Print format specifier macros */

int cfi_print(const regval_t data[WRITE_DATACOUNT])
{
    printf("Read: \n");
    for (int i = 0; i < WRITE_DATACOUNT; i++)
    {
        printf("  0x%04x\n", data[i]);
    }
    return 0;
}
