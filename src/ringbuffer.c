#include "ringbuffer.h"
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>

/** Modulus calculation for positive divisors.
    - \a a The dividend
    - \a b The divisor, cannot be zero
*/
#define MOD(a,b) ((((a) % (b)) + (b)) % (b))

/** Amount of entries in the write operation. */
#define WRITE_DATACOUNT 8

/** Manually define MAP_ANONYMOUS for C99. NOTE: Is this valid? */
#define MAP_ANONYMOUS 0x20

ringbuffer_t *rb_create(uint32_t bufsize)
{
    /* Bail out in case of idiotic request */
    if (bufsize == 0) return NULL;

    /* Try to map memory for the data. */
    ringbuffer_t *rb = (ringbuffer_t *)mmap(NULL, (sizeof(ringbuffer_t) + (bufsize * sizeof(regval_t))), (PROT_READ | PROT_WRITE), (MAP_ANONYMOUS | MAP_SHARED), -1, 0);

    /* Check whether memory allocation succeeded. */
    if (rb == (ringbuffer_t *)-1)
    {
        fprintf(stderr,"rb_create(): Failed to map memory; mmap error code %d. Returning NULL\n", errno);
        return NULL;
    }
    else
    {
        /* Fill the struct with data */
        rb->size = bufsize;
        /* Calculate the start position of the buffer to lie directly after the “administrative” ringbuffer struct in memory. */
        rb->start = (regval_t *)(rb + sizeof(ringbuffer_t));
        rb->read = 0;
        rb->write = 0;
    }
    /* Return the pointer to the ringbuffer struct. */
    return rb;
}

void rb_destroy(ringbuffer_t *rbptr)
{
    /* Unmap the memory allocated to the buffer. */
    munmap(rbptr, rbptr->size + sizeof(ringbuffer_t));
}

regval_t *rb_read(ringbuffer_t *rbptr, regval_t *data, uint32_t count)
{
    /* Bail out on stupid input */
    if ((rbptr == NULL) || (data == NULL))
        return NULL;
    /* The read is only valid if it does not read beyond the write pointer */
    /* Upcast to avoid uint wrapping when rbptr->read - rbptr->write < 0 */
    int64_t upcast_write = (int64_t)rbptr->write;
    /* Upcast to avoid uint wrapping when rbptr->read + count > UINT32_MAX */
    int64_t upcast_read  = (int64_t)rbptr->read;
    if ((uint32_t)(MOD((upcast_write - upcast_read), rbptr->size)) >= count)
    {
        /* Copy register values one by one.
        Use remainder operation to wrap linear memory space.
        Can most likely be optimised. */
        for (uint32_t i = 0; i < count; i++)
        {
            data[i] = rbptr->start[(uint32_t)((upcast_read + i) % rbptr->size)];
        }
        /* Update read index. */
        rbptr->read = (uint32_t)((upcast_read + count) % rbptr->size);
    }
    else
    {
        return NULL;
    }
    return data;
}

void rb_write(regval_t r0, regval_t r1, regval_t r2, regval_t r3, regval_t pc, regval_t lr, regval_t fp, regval_t sp, ringbuffer_t *rbptr)
{
    regval_t data[WRITE_DATACOUNT] = {r0, r1, r2, r3, pc, lr, fp, sp};
    /* Bail out on stupid input */
    if (rbptr == NULL)
        return;
    /* Upcast to avoid uint wrapping when rbptr->read - rbptr->write < 0 */
    int64_t upcast_write = (int64_t)rbptr->write;
    /* Upcast to avoid uint wrapping when rbptr->read + count > UINT32_MAX */
    int64_t upcast_read  = (int64_t)rbptr->read;
    if ((uint32_t)(MOD((upcast_read - upcast_write), rbptr->size)) >= WRITE_DATACOUNT)
    {
        /* Copy register values one by one.
        Can maybe be optimised. */
        for (uint32_t i = 0; i < WRITE_DATACOUNT; i++)
        {
            rbptr->start[(uint32_t)((upcast_write + i) % rbptr->size)] = data[i];
        }
        /* Update write index. */
        rbptr->write = (uint32_t)((upcast_write + WRITE_DATACOUNT) % rbptr->size);
    }
    return;
}
