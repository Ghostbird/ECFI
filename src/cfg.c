#include "ringbuffer.h" /* global cfg_offset */
#include "errors.h"
#include "cfg.h"
#include <errno.h> /* errno */
#include <stdio.h> /* fprintf and stderr */
#include <stdlib.h> /* malloc and free */

cfg_int hotsite_type_bitmask = 0x07;

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
    fprintf(stderr, "CFG size is %i nodes.\n", cfg->size);
    // Allocate data for pointer array to CFG blocks.
    cfg->data = malloc(sizeof(cfg_int*) * cfg->size);
    // Read the blocks in the CFG.
    for (cfg_int i = 0; i < cfg->size; i++)
    {
        fprintf(stderr, "Reading node %i\n", i);
        // Allocate memory for CFGblock.
        cfg->data[i] = malloc(sizeof(cfgblock_t));
        // Read next value out of file into hotsiteid for this block.
        fread((void*)&(cfg->data[i]->hotsiteid), sizeof(cfg_int), 1, file);
        fprintf(stderr, "HotsiteID: %i\n", cfg->data[i]->hotsiteid);
        // Read next value out of file into pre_size for this block.
        fread((void*)&(cfg->data[i]->pre_size), sizeof(cfg_int), 1 , file);
        fprintf(stderr, "Number of pre-nodes: %i\n", cfg->data[i]->pre_size);
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
                fprintf(stderr, "Pre-target %i: %i\n", j, cfg->data[i]->pre_data[j]);
            }
        }
        // Read next value out of file into post_size for this block.
        fread((void*)&(cfg->data[i]->post_size), sizeof(cfg_int), 1, file);
        fprintf(stderr, "Number of post-nodes: %i\n", cfg->data[i]->post_size);
        // Check for presence of post_data in file
        if (cfg->data[i]->post_size > 0)
        {
            // Allocate memory for pre_data.
            cfg->data[i]->post_data = malloc(sizeof(cfg_int) * cfg->data[i]->post_size);
            // Iterate over pre_data
            for (cfg_int j = 0; j < cfg->data[i]->post_size; j++)
            {
                // Read next value from file into post_data[j]
                fread((void*)(cfg->data[i]->post_data + j), sizeof(cfg_int), 1, file);
                fprintf(stderr, "Post-target %i: %i\n", j, cfg->data[i]->post_data[j]);
            }
        }
    }
    fprintf(stderr, "Finished reading CFG.\n");
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
    if (hotsite == 0)
    {
        fprintf(stderr, "Program hit invalid hotsite!\n");
        fflush(stderr);
        return 0;
    }
    // Iterate over bblocks.
    for (cfg_int i = 0; i < cfg->size; i++)
    {
        // Reverse mask the hotsiteid to keep only the blockid.
        cfg_int pure_hotsite = hotsite & (~hotsite_type_bitmask);
        // Check whether the current block is the relevant one.
        if (cfg->data[i]->hotsiteid == pure_hotsite)
        {
            fprintf(stderr, "Found matching hotsite id in CFG for hotsite %i\n", hotsite);
            cfg_int hotsite_type = hotsite & hotsite_type_bitmask;
            switch (hotsite_type)
            {
                case 0:
                fprintf(stderr, "Control flow transfer is a BLX\n");
                break;
                case 1:
                fprintf(stderr, "Control flow transfer is a function epilogue\n");
                break;
                default:
                fprintf(stderr, "Hotsite type is unknown!\n");
            }
            fprintf(stderr, "Validation of the jump target is not yet supported!\n");
            return 1;
        }
    }
    // The loop has ended, so no matching edge has been found.
    fprintf(stderr, "Could not validate edge hotsite %08x -> address %08x\n", hotsite, target);
    fflush(stderr);
    return 0;
}
