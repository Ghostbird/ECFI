#include <inttypes.h>

typedef uint32_t cfg_int;

struct cfgblock
{
    cfg_int address;
    cfg_int pre_size;
    cfg_int *pre_data;
    cfg_int post_size;
    cfg_int *post_data;
};

typedef struct cfgblock cfgblock_t;


struct cfg
{
    cfg_int size;
    cfgblock_t **data;
};

typedef struct cfg cfg_t;
