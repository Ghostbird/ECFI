#ifndef CFI_CHECKER_INCLUDEONCE // Guard against double includes.
#define CFI_CHECKER_INCLUDEONCE
#include <inttypes.h>
#include "ringbuffer.h"
#include "cfg.h"

/*! Global variable to store the offset between the main function
    in the CFG and the main function in reality. */
int64_t cfg_offset;

/*! Checker function. Runs until interrupted.
    Reads from ringbuffer and passes the data through heuristics. */
void checker(ringbuffer_info_t *rb_info, cfg_t *cfg);

/*! Main program for cfi checker.

    Creates a ringbuffer.
    Forks the checker.
    Forks and spawns the program passed as argument.*/
int main(int argc, char** argv);

/*! Calculate the difference between the main function in the CFG
    and the main function in reality.

    Store this value in cfg_offset.
    Call once during set-up of the program that writes to the
    ringbuffer.
    \param cfg_address Address of main in the CFG.
    \param real_address Real address of main. */
void setup(void *cfg_address, void *real_address);
#endif
