#ifndef CFI_CHECKER_INCLUDEONCE // Guard against double includes.
#define CFI_CHECKER_INCLUDEONCE
#include <inttypes.h>
#include "ringbuffer.h"
#include "cfg.h"

/*! Checker function. Runs until interrupted.
    Reads from ringbuffer and passes the data through heuristics. */
void checker(ringbuffer_info_t *rb_info, cfg_t *cfg);

/*! Main program for cfi checker.

    Creates a ringbuffer.
    Forks the checker.
    Forks and spawns the program passed as argument.*/
int main(int argc, char** argv);

#endif
