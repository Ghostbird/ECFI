/* Guard against including twice. */
#ifndef WRITE_DATACOUNT
   #include "ringbuffer.h"
#endif
#ifndef CFG_INCLUDE_ONCE
    #include "cfg.h"
#endif

/*! This is a testing heuristic. It just prints the data read.
    /param data An array of one WRITE_DATACOUNT of regval_t units.
*/
int cfi_print(const regval_t *data);

/*! This heuristic writes the buffer contents to a binary file. */
int cfi_record(const regval_t *data);

/*! This heuristic checks the buffer contents against a previously
    written record file from cfi_record(). */
int cfi_check_record(const regval_t *data);

/*! This heuristic checks the buffer contents against the cfg. */
int cfi_validate_forward_edge(const regval_t *data, const cfg_t *cfg);
