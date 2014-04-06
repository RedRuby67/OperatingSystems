// Defining _KERNEL_ and MODULE allows us to access kernel-level code
// not usually available to userspace programs.
#undef __KERNEL__
#define __KERNEL__

#undef __MODULE__
#define __MODULE__

// Linux Kernel/LKM headers: module.h is needed by all modules and 
// kernel.h is needed for KERN_INFO.
#include <linux/module.h>	// included for all kernel modules
#include <linux/kernel.h>	// included for KERN_INFO
#include <linux/init.h>		// included for __init and __exit macros

static int __init hello_init(void)
{
	printk(KERN_INFO "Hello world!\n");
	return 0; // non-zero return means that module couldn't be loaded
}

static void __exit hello_cleanup(void)
{
	printk(KERN_INFO "Cleaning up module.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);
