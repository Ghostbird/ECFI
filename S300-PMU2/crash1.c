/* Module source file 'crash1.c'. */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

arm_write(unsigned long val)
{

//      asm volatile("mcr p15, 0, %0, c9, c14, 0" : : "r"(val));
//        asm volatile("mcr p15, 0, %0, c9, c14, 0" :: "r"(1));
        asm volatile("mrc p15, 0, %0, c9, c14, 0" :: "r"(1));
        asm volatile("mcr p15, 0, %0, c9, c14, 0" :: "r"(1));
//        asm volatile("mcr p15, 0, %0, c9, c12, 0" :: "r"(PERF_DEF_OPTS));
//        asm volatile("mcr p15, 0, %0, c9, c12, 1" :: "r"(0x8000000f));

}

static int hi(void)
{
     unsigned long value = 1;
     printk(KERN_INFO "Enabling PMU usermode.\n");
     arm_write(value);
     return 0;
}

static void bye(void)
{
     printk(KERN_INFO "crash1 module being unloaded.\n");
}

module_init(hi);
module_exit(bye);

MODULE_AUTHOR("Ali Abbasi, Gijsbert ter Horst");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Enabling PMU access from user mode");
