#include "cfi-heuristics.h"
#include <stdio.h>     /* print(), fwrite(), fprintf(), stderr, stdout */
#include <inttypes.h>  /* Print format specifier macros */
#include <errno.h>     /* errno */
#include <string.h>    /* memcmp */

#define RECORDNAME "record"

int cfi_print(const regval_t *data)
{
    printf("Read: \n");
    for (int i = 0; i < WRITE_DATACOUNT; i++)
    {
        printf("  0x%04x\n", data[i]);
    }
    /* Force write to go through, or data may be lost on termination. */
    fflush(NULL);
    return 0;
}


int cfi_record(const regval_t *data)
{
    FILE *record;
    static unsigned char reset_once = 1;
    if (reset_once)
    {
        /* Truncate file RECORDNAME and open it for writing. */
        record = fopen(RECORDNAME,"w");
        reset_once = 0;
    }
    else
    {
        /* Open file RECORDNAME for appending. */
        record = fopen(RECORDNAME,"a");
    }
    /* Check for failure to open file. */
    if (record == NULL)
    {
        fprintf(stderr, "Error: Failed to open record for writing! fopen() returned NULL with error code %d.", errno);
        return -1;
    }
    /* Write the data to the file. */
    size_t count = fwrite(data, sizeof(regval_t), WRITE_DATACOUNT, record);
    /* Check whether all the data was written. */
    if (count < WRITE_DATACOUNT)
    {
        fprintf(stderr, "Error: Not all data has been recorded! Recorded %lu of %lu values.\n", (unsigned long int)count, (unsigned long int)(WRITE_DATACOUNT)); 
    }
    if (fclose(record) != 0)
    {
        fprintf(stderr, "Error: Failed to close record! fclose() returned nonzero with error code %d.", errno);
        return -1;
    }
    return 0;
}

int cfi_check_record(const regval_t *data)
{
    /* Keep track of the number of times this function is called. */
    static uint32_t step = 0;
    /* Static buffer to store read data. No need to reallocate every time. (TODO: check safety) */
    static regval_t buffer[WRITE_DATACOUNT];
    /* Open file RECORDNAME for reading. */
    FILE *record = fopen(RECORDNAME,"rb");
    /* Check for failure to open file. */
    if (record == NULL)
    {
        fprintf(stderr, "Error: Failed to open record for reading! fopen() returned NULL with error code %d.", errno);
        return -1;
    }
    /* Move file position indicator to correct location in file. */
    fseek(record, WRITE_DATACOUNT * sizeof(regval_t) * step, SEEK_SET);
    /* Read data from file. */
    size_t count = fread(buffer, sizeof(regval_t), WRITE_DATACOUNT, record);
    /* Check whether the read from the record went well. */
    if (count < WRITE_DATACOUNT)
    {
        if (feof(record) != 0)
        {
            fprintf(stderr, "Error: Program has generated more data than the original record contains!");
        }
        else if (ferror(record))
        {
            fprintf(stderr, "Error: An unknown error occurred while reading from the original record!");
        }
    }
    else
    {
        /* Compare current data to original record. */
        if (memcmp(data, buffer, WRITE_DATACOUNT * sizeof(regval_t)) != 0)
        {
            fprintf(stdout, "Original record, and current program run are not identical in step %d:\n", step);
            fprintf(stdout, "        Original            Current\n");
            fprintf(stdout, "Hotsite: %016x    %016x\n", buffer[0], data[0]);
            fprintf(stdout, "Target:  %016x    %016x\n", buffer[1], data[1]);
        }
    }
    if (fclose(record) != 0)
    {
        fprintf(stderr, "Error: Failed to close record! fclose() returned nonzero with error code %d.", errno);
        return -1;
    }
    step += 1;
    return 0;
}

/* Return 0 on success, 1 on invalid edge, -1 on error. */
int cfi_validate_forward_edge(const regval_t *data, const cfg_t *cfg)
{
    if (cfg_validate_jump(cfg, data[0], data[1]) == 0)
    {
        return 0;
    }
    return 1;
}
