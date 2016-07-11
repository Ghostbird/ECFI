/*

DummyModbusSVC server (slave) frontend daemon

Simple dummy modbus TCP service. Supports coil/bit/register writing/reading, writes state to files upon every new request.
Contains trivial stack buffer overflow vulnerability due to small query buffer.

Protections:
    XN
    Stack Cookies
    Full RELRO
    ASLR (host-enabled)

gcc -Bdynamic -lc -ldl -lrt -lm -lpthread -lgcc_s -lmodbus -fstack-protector-all -Wformat -Wformat-security -Wconversion -Wsign-conversion -Wl,-z,relro,-z,now -Ilibmodbus/src -o dummyserver dummyserver.c

*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>
#include <dlfcn.h>
#include <link.h>
#include <sys/file.h>

#define QUERY_BUFFER_SIZE 32

extern uint32_t __stack_chk_guard;

/*
    Set registers to infoleak values

        + stack pointer
        + stack cookie
        + libc base address
*/
void init_mapping(modbus_mapping_t *mb_mapping)
{    
    uint32_t sp;

    asm volatile("mov %0, sp"
                 : "=r" (sp)
                );

    struct link_map* lm = (struct link_map*)dlopen("libc.so.6", RTLD_NOW);

    mb_mapping->tab_registers[0] = ((sp & 0xFFFF0000) >> 16);
    mb_mapping->tab_registers[1] = (sp & 0x0000FFFF);
    mb_mapping->tab_registers[2] = ((__stack_chk_guard & 0xFFFF0000) >> 16);
    mb_mapping->tab_registers[3] = (__stack_chk_guard & 0x0000FFFF);

    if(lm != NULL)
    {
        mb_mapping->tab_registers[4] = ((lm->l_addr & 0xFFFF0000) >> 16);
        mb_mapping->tab_registers[5] = (lm->l_addr & 0x0000FFFF);
    }

    return;
}

/*
    Write coils, discrete inputs, input registers and holding registers to files.
*/
void save_modbus_state(modbus_mapping_t* mb_mapping)
{
    unsigned int i;
    char* fnames[4] = {"coils.save", "disc.save", "ireg.save", "hreg.save"};
    FILE* fp;

    for(i = 0; i < 4; i++)
    {
        fp = fopen(fnames[i], "wb");

        if(!fp)
        {
            printf("[-] Failed to open '%s' for saving...\n", fnames[i]);
            return;
        }

        // Place shared lock on file during writing
        flock(fileno(fp), LOCK_EX);

        switch(i)
        {
            case 0:
            {
                fwrite(mb_mapping->tab_bits, sizeof(uint8_t), mb_mapping->nb_bits, fp);
            }break;

            case 1:
            {
                fwrite(mb_mapping->tab_input_bits, sizeof(uint8_t), mb_mapping->nb_input_bits, fp);
            }break;

            case 2:
            {
                fwrite(mb_mapping->tab_input_registers, sizeof(uint16_t), mb_mapping->nb_input_registers, fp);
            }break;

            case 3:
            {
                fwrite(mb_mapping->tab_registers, sizeof(uint16_t), mb_mapping->nb_registers, fp);
            }break;
        }

        // Remove shared lock from file after writing
        flock(fileno(fp), LOCK_UN);

        fclose(fp);
    }
    return;
}

void modbus_task(modbus_t* ctx, modbus_mapping_t* mb_mapping)
{
    uint8_t query[QUERY_BUFFER_SIZE];
    int rc;

    for (;;) 
    {
        rc = modbus_receive(ctx, query);
        if (rc > 0) 
        {
            // save new state
            save_modbus_state(mb_mapping);
            // rc is the query size
            modbus_reply(ctx, query, rc, mb_mapping);
        } 
        else if (rc == -1) 
        {
            // Connection closed by the client or error
            break;
        }
    }
    return;
}

int modbus_routine()
{
    int s = -1;
    modbus_t *ctx;
    modbus_mapping_t *mb_mapping;

    ctx = modbus_new_tcp("0.0.0.0", 1502);
    modbus_set_debug(ctx, TRUE);

    mb_mapping = modbus_mapping_new(500, 500, 500, 500);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    init_mapping(mb_mapping);

    for(;;)
    {
        s = modbus_tcp_listen(ctx, 1);
        modbus_tcp_accept(ctx, &s);
        modbus_task(ctx, mb_mapping);

        printf("[-] modbus_task exit: %s\n", modbus_strerror(errno));

        if (s != -1) 
        {
            close(s);
        }
    }

    printf("[-] accept loop exit: %s\n", modbus_strerror(errno));
    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);
    return 1;
}

int main(void)
{
    modbus_routine();
    return 0;
}