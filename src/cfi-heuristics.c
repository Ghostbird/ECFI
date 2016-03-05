#include "cfi-heuristics.h"
#include <stdio.h>     /* print() */
#include <inttypes.h>  /* Print format specifier macros */

int cfi_print(const regval_t data[WRITE_DATACOUNT])
{
    printf("Read: \n");
    for (int i; i < WRITE_DATACOUNT; i++)
    {
        printf("  0x%4" PRIx32 "\n", data[i]);
    }
    return 0;
}
