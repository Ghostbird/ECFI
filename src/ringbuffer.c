#include "ringbuffer.h"
#include <stdlib.h>

/** Modulus calculation for positive divisors.
    a The dividend
    b The divisor, cannot be zero
*/
#define MOD(a,b) ((((a) % (b)) + (b)) % (b))

ringbuffer_t *rb_create(uint32_t bufsize)
{
    /* Try to get memory allocated for the ringbuffer struct. */
    ringbuffer_t *rb = malloc(sizeof(*rb));

    /* Try to get memory allocated for the buffer.
    Some idiot is going to try a zero size buffer. In such unreasonable
    situations it is reasonable to assume that malloc(0) returns NULL. */
    void *buf = malloc(bufsize * sizeof(regval_t));

    /* Check whether memory allocation succeeded. */
    if (buf != NULL && rb != NULL)
    {
        /* Fill the struct with data */
        rb->size = bufsize;
        rb->start = buf;
        rb->read = 0;
        rb->write = 0;
    }
    else
    {
        /* In this case, something's wrong, attempt clean-up. */
        if (buf != NULL)
        {
            /* Free buf if it was allocated. */
            free(buf);
        }
        if (rb != NULL)
        {
            /* Free rb if it was allocated. */
            free(rb);
            /* Explicitly set it to NULL for the return. */
            rb = NULL;
        }
    }
    /* Return the pointer to the ringbuffer struct. */
    return rb;
}

void rb_destroy(ringbuffer_t *rbptr)
{
    /* Free the memory allocated to the buffer. */
    free(rbptr->start);
    /* Invalidate start pointer in the struct. */
    rbptr->start = NULL;
    /* Free the struct itself. */
    free(rbptr);
    return;
}

void rb_read(ringbuffer_t *rbptr, regval_t *data, uint32_t count)
{
    /* Upcast to avoid uint wrapping when rbptr->read - rbptr->write < 0 */
    int64_t upcast_write = (int64_t)rbptr->write;
    /* Upcast to avoid uint wrapping when rbptr->read + count > UINT32_MAX */
    int64_t upcast_read  = (int64_t)rbptr->read;
    /* The read is only valid if it does not read beyond the write pointer
    and the data pointer is valid */
    if ((data != NULL) && ((uint32_t)(MOD((upcast_write - upcast_read), rbptr->size)) <= count))
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
}
