#ifndef CFG_INCLUDEONCE
#define CFG_INCLUDEONCE
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

cfg_t *cfg_create(char *path);

void cfg_destroy(cfg_t *cfg);

char cfg_validate_jump(const cfg_t *cfg, const cfg_int hotsite, const cfg_int target);
#endif
