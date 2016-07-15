#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/workqueue.h>
#include <linux/time.h>
#include <asm/mutex.h>


//
// Read the PMU User Enable Register, PMUUSERENR, and return the value
//
static inline unsigned long
arm_useren_read(void)
{
    u32 val;
    asm volatile("mrc   p15, 0, %0, c9, c14, 0" : "=r"(val));
    return val;
}

//
// Write the PMU User Enable Register, PMUUSERENR, to the given value
//
static inline void
arm_useren_write(unsigned long val)
{
    asm volatile("mcr   p15, 0, %0, c9, c14, 0" : : "r"(val));
}

//
// Make PMU accessible from user space when module is loaded
//
static int __init enable_pmu_init(void)
{
    int st = 0;
    unsigned long value = 0;

    value = arm_useren_read();
    printk ("ARM PMU bit value is %lu \n", value);
    // Set the enable bit
    value |= 1;

    // Write new value to enable user-space access to the Performance Monitor counters
    arm_useren_write(value);

    printk ("PMU Enabled?: The PMU is now accesible in user mode\n");
    printk ("ARM PMU bit value is %lu \n", value);
//    value2 = arm_useren_write();
//    printk ("arm_useren_write value is $lu\n", value2);
    return(st);
}

//
// Disable PMU access from user space when module is unloaded
//
static void __exit enable_pmu_exit(void)
{
    unsigned long value;
//   unsigned long value2
    value = arm_useren_read();
    printk ("PMU exit ?: read %lu\n",value);

    // Clear the enable bit
    value &= 0xfffffffe;

    // Write new value to disable user-space access to the Performance Monitor counters
    arm_useren_write(value);

    printk ("PMU access in user mode disabled\n");
}

module_init(enable_pmu_init);
module_exit(enable_pmu_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ali Abbasi");
MODULE_DESCRIPTION("Enable Broadcom BCM2835 SoCs PMU from user space");
