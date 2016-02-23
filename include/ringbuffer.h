#include <inttypes.h>

/*! Define macro for ringbuffer name.
TODO: Invent naming scheme so multiple ringbuffers can be used concurrently.*/
#define SHMNAME ("/ringbuffer")

/* Amount of entries in the write operation. */
#define WRITE_DATACOUNT 8

/* Size in memory of a ring buffer with x entries. */
#define RB_MEMSIZE(size) (sizeof(ringbuffer_t) + (sizeof(regval_t)*size)) 

/*! The type of a register value is defined as a 32 bit integer. */
typedef uint32_t regval_t;

/*! This is a ring buffer structure

    Instead of a read pointer and write pointer it uses indices.
    NOTE: If the type of the size, read, and write is changed,
    The arithmetic in \file ../src/ringbuffer.c must be checked,
    and casting must be reviewed to verify no wraps can occur.
*/
struct ringbuffer
{
    /*! The memory location where the ring buffer starts.
    This implies that the buffer ends before memory location:
      start + size * sizeof(regval_t) */
    regval_t *start;
    /*! The number of regval_t objects that fit in the buffer. */
    uint32_t size;
    /*! The current read index */
    uint32_t read;
    /*! The current write index */
    uint32_t write;
};

/*! The ringbuffer_t typedef */
typedef struct ringbuffer ringbuffer_t;

/*! Create a new ringbuffer
    \param  bufsize  The size of the ring buffer in bytes.
    \return          A pointer to a ringbuffer. NULL on failure.
*/
ringbuffer_t *rb_create(uint32_t bufsize);

/*! Destroy a ringbuffer nicely
    \param  rbptr  A pointer to a valid fully initialised ring buffer.
*/
void rb_destroy(ringbuffer_t *rbptr);

/*! Read \a count register values from \a rbptr into \a data.
    The read will fail if it would overtake the write pointer.
    A successful read will update the read pointer.
    The caller is responsible for memory management of the data pointer.
    \param  rbptr  A pointer to the ring buffer struct to read from.
    \param  data   A pointer to the location where the data should be copied.
    \param  count  Non-zero number of register values to read from the buffer.
    \return        The \a data pointer. Useful in e.g. if (rb_read(..) != NULL)
*/
regval_t *rb_read(ringbuffer_t *rbptr, regval_t *data, uint32_t count);

/*! Write \a r0, \a r1, \a r2, \a r3, \a lr, \a pc, \a fp, and \a sp to ring buffer \a rbptr.
    Write fails silently if the read pointer would be overwritten.
    This is not desirable and a better solution needs to be found.
    \param  r0     The value of R0
    \param  r1     The value of R1
    \param  r2     The value of R2
    \param  r3     The value of R3
    \param  pc     The value of program counter
    \param  lr     The value of the link register
    \param  fp     The value of the frame pointer
    \param  sp     The value of the stack pointer
    \param  rbptr  A pointer to the ring buffer to write to.
*/
void rb_write(regval_t r0, regval_t r1, regval_t r2, regval_t r3, regval_t pc, regval_t lr, regval_t fp, regval_t sp, ringbuffer_t *rbptr);
