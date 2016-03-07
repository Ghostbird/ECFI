#include "ringbuffer.h"
int main()
{
    regval_t data[WRITE_DATACOUNT];
    ringbuffer_t *rb = rb_attach(3);
    for (uint32_t i; i < rb->size * 5; i++)
    {
        for (uint32_t j; j < WRITE_DATACOUNT; j++)
        {
            data[j] = i * j;
        }
        rb_write(data, rb);
    }
}
