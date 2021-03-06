#ifndef RINGBUFFER_INCLUDEONCE //Guard against double includes
#define RINGBUFFER_INCLUDEONCE
#include <inttypes.h>

/*! Amount of regval_t entries in a buffer field. */
#define WRITE_DATACOUNT 2

/*! Size in memory of a ring buffer with WRITE_DATACOUNT×size entries.
Take care that size is not allowed to be so big that it overflows the uint32_t maximum. */
#define RB_MEMSIZE(size) ((uint32_t)(sizeof(ringbuffer_t) + size))

/*! This bit indicates that an override has happened. */
// Unused. #define RB_FLAG_OVERRIDE 0x01

/*! Modulus calculation for positive divisors.
    - \a a The dividend
    - \a b The divisor, cannot be zero
*/
#define MOD(a,b) ((((a) % (b)) + (b)) % (b))

/*! The type of a register value is defined as an integer. */
typedef unsigned int regval_t;

/*! This is a ring buffer structure

    Instead of a read pointer and write pointer it uses indices.
    NOTE: If the type of the size, read, and write is changed,
    The arithmetic in \file ../src/ringbuffer.c must be checked,
    and casting must be reviewed to verify no wraps can occur.
*/
struct ringbuffer
{
    /*! The number of regval_t objects that fit in the buffer. */
    uint32_t size;
    /*! The current read index */
    uint32_t read;
    /*! The current write index */
    uint32_t write;
    /*! The flags */
    uint32_t flags;
};

/*! The ringbuffer_t typedef */
typedef struct ringbuffer ringbuffer_t;

/*! This is a ring buffer information structure.

    It is created by the rb_create function and should be passed to the rb_destroy() function.
    These functions will take care of the memory management.
*/
struct ringbuffer_info
{
    /*! Pointer to the location of the ringbuffer in memory. */
    ringbuffer_t *rb;
    /*! Filedescriptor to the open (anonymous) file associated with the ringbuffer. */
    int fd;
    /*! Name of the file associated with the ringbuffer. */
    char* name;
};

/*! The ringbuffer_info_t typedef */
typedef struct ringbuffer_info ringbuffer_info_t;

/*! Global pointer to read index used to optimise the writer */
uint32_t *rb_writer_read;
/*! Global pointer to write index used to optimise the writer */
uint32_t *rb_writer_write;
/*! Global pointer to buffer start used to optimise the writer */
regval_t *rb_writer_buffer;
/*! Global pointer to buffer end used to optimise the writer */
regval_t *rb_writer_end;
/*! Global variable to store the offset between the main function
    in the CFG and the main function in reality. */
int64_t cfg_offset;


/*! Create a new ringbuffer
    \param  bufsize  The size of the ring buffer in bytes.
    \param  bufname  Name of the ring buffer. Must conform to shm_open(3).
    \return          A pointer to a ringbuffer_info struct. NULL on failure.
*/
ringbuffer_info_t *rb_create(uint32_t bufsize, const char *bufname);

/*! Destroy a ringbuffer nicely.

    This function structurally calls complementary functions to those in
    rb_create to clean up a ringbuffer.
    Similar to free() the pointer passed to this function should not be used afterwards.
    \param  rb_info  A pointer to a valid ring buffer information struct.
*/
void rb_destroy(ringbuffer_info_t *rb_info);

/*! Read \a count register values from \a rbptr into \a data.
    The read will fail if it would overtake the write pointer.
    A successful read will update the read pointer.
    The caller is responsible for memory management of the data pointer.
    \param  rbptr  A pointer to the ring buffer struct to read from.
    \param  data   A pointer to the location where the data should be copied.
    \param  count  Non-zero number of regval_t values to read from the buffer.
    \return        The \a data pointer. Useful in e.g. if (rb_read(..) != NULL)
*/
regval_t *rb_read(ringbuffer_t *rbptr, regval_t *data, uint32_t count);

/*! Write an array of WRITE_DATACOUNT items to ring buffer \a rbptr.
    Behaviour is undefined if rbptr == NULL.
    This operation relies on the fact that the size of the ringbuffer is a multiple of the WRITE_DATACOUNT.
    This is guaranteed by using RB_MEMSIZE to calculate the size of the buffer at creation time.
    If this operation overwrites the read pointer it will increment the read pointer by WRITE_DATACOUNT.
    \param  data an array of data to write of size WRITE_DATACOUNT
    \param  rbptr  A pointer to the ring buffer to write to. CANNOT BE NULL.
    \param  count  The number of regval_t * WRITE_DATACOUNT entries to read. Must be greater than zero.
    \return The pointer to the data array, NULL if the read failed.
*/
void rb_write(ringbuffer_t *rbptr, const regval_t *data);

/*! Attach to an existing ringbuffer from a file descriptor.
    \param  fd The file descriptor of the open shared memory file.
    \return    A pointer to the ringbuffer_t object stored in the file.
*/
ringbuffer_t *rb_attach(int fd);

/*! Initialisation function for the writer.
    Attaches to ringbuffer on file descriptor 3
    Sets the global variables rb_writer_... */
void rb_init_writer();

/*! Write to buffer after rb_init_writer() has been called once. 
    This is a C function that can be branched to from assembly
    to write to the ringbuffer directly */
void rb_write_attached(int arg0, int arg1);

/*! Calculate the difference between the main function in the CFG
    and the main function in reality.

    Store this value in cfg_offset.
    Call once during set-up of the program that writes to the
    ringbuffer.
    \param cfg_address Address of main in the CFG.
    \param real_address Real address of main. */
void cfg_setup(void *cfg_address, void *real_address);
#endif
