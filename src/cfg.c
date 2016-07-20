#include "errors.h"
#include "cfg.h"
#include <errno.h> /* errno */
#include <stdio.h> /* fprintf and stderr */
#include <stdlib.h> /* malloc and free */

cfg_t *cfg_create(char* path)
{
    // Open file
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Failed to open file: %s\n", path);
        return NULL;
    }
    // TODO malloc NULL checks. fread checks.
    // Allocate memory for the CFG.
    cfg_t *cfg = malloc(sizeof(cfg_t));
    // Read the amount of blocks in the CFG from the start of the file.
    fread((void*)&(cfg->size), sizeof(cfg_int), 1, file);
    // Allocate data for pointer array to CFG blocks.
    cfg->data = malloc(sizeof(cfg_int*) * cfg->size);
    // Read the blocks in the CFG.
    for (cfg_int i = 0; i < cfg->size; i++)
    {
        // Allocate memory for CFGblock.
        cfg->data[i] = malloc(sizeof(cfgblock_t));
        // Read next value out of file into address for this block.
        fread((void*)&(cfg->data[i]->address), sizeof(cfg_int), 1, file);
        // Read next value out of file into pre_size for this block.
        fread((void*)&(cfg->data[i]->pre_size), sizeof(cfg_int), 1 , file);
        // Check for presence of pre_data in file
        if (cfg->data[i]->pre_size > 0)
        {
            // Allocate memory for pre_data.
            cfg->data[i]->pre_data = malloc(sizeof(cfg_int) * cfg->data[i]->pre_size);
            // Iterate over pre_data
            for (cfg_int j = 0; j < cfg->data[i]->pre_size; j++)
            {
                // Read next value from file into pre_data[j]
                fread((void*)(cfg->data[i]->pre_data + j), sizeof(cfg_int), 1, file);
            }
        }
        // Read next value out of file into post_size for this block.
        fread((void*)&(cfg->data[i]->post_size), sizeof(cfg_int), 1, file);
        // Check for presence of post_data in file
        if (cfg->data[i]->post_size > 0)
        {
            // Allocate memory for pre_data.
            cfg->data[i]->post_data = malloc(sizeof(cfg_int) * cfg->data[i]->post_size);
            // Iterate over pre_data
            for (cfg_int j = 0; j < cfg->data[i]->post_size; j++)
            {
                // Read next value from file into pre_data[j]
                fread((void*)(cfg->data[i]->post_data + j), sizeof(cfg_int), 1, file);
            }
        }
    }
    // TODO Verify that read has reached EOF.
    return cfg;
}

void cfg_destroy(cfg_t *cfg)
{
    for (cfg_int i = 0; i < cfg->size; i++)
    {
        free(cfg->data[i]->pre_data);
        free(cfg->data[i]->post_data);
    }
    free(cfg->data);
    free(cfg);
    return;
}

char cfg_validate_jump(const cfg_t *cfg, const cfg_int hotsite, const cfg_int target)
{
    // Store cfg block index for hotsite.
    int bblock = 0;
    // Store cfg bblock_addres for bblock search.
    cfg_int bblock_address = 0;
    // Iterate over bblocks.
    for (cfg_int i = 0; i < cfg->size; i++)
    {
        if (cfg->data[i]->address > bblock_address && cfg->data[i]->address <= hotsite)
        {
            bblock_address = cfg->data[i]->address;
            bblock = i;
            fprintf(stderr, "Found block %i:%x for hotsite %x.\n", bblock, bblock_address, hotsite);
        }
    }
    for (cfg_int i = 0; i < cfg->data[bblock]->post_size; i++)
    {
        if (cfg->data[bblock]->post_data[i] == target)
        {
            fprintf(stderr, "Found valid edge %x -> %x.", hotsite, target);
            return 1;
        }
    }
    fprintf(stderr, "Could not validate edge %x -> %x.", hotsite, target);
    return 0;
}
