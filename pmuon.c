#include <linux/module.h>
#include <linux/kernel.h>

#define INST_RETIRED 0x08
MODULE_LICENSE("GPL");
MODULE_AUTHOR("CHENGCHEN_ZHANG");
MODULE_DESCRIPTION("PMUON");

void set_pmu(void* dummy) {
    unsigned int v;
	uint32_t ptype;
	uint32_t counter;
    printk("Turn PMU on Core %d\n", smp_processor_id());

    // 1. Enable "User Enable Register"
    asm volatile("mcr p15, 0, %0, c9, c14, 0\n\t" :: "r" (0x00000001)); 

    // 2. Reset i) Performance Monitor Control Register(PMCR),
    // ii) Count Enable Set Register, and
    // iii) Overflow Flag Status Register
    asm volatile ("mcr p15, 0, %0, c9, c12, 0\n\t" :: "r"(0x00000017));
    asm volatile ("mcr p15, 0, %0, c9, c12, 1\n\t" :: "r"(0x8000003f));
    asm volatile ("mcr p15, 0, %0, c9, c12, 3\n\t" :: "r"(0x8000003f));

    // 3. Disable Interrupt Enable Clear Register
    asm volatile("mcr p15, 0, %0, c9, c14, 2\n\t" :: "r" (~0));

    // 4. Read how many event counters exist 
    asm volatile("mrc p15, 0, %0, c9, c12, 0\n\t" : "=r" (v)); // Read PMCR
    printk("We have %d configurable event counters on Core %d\n",
            (v >> 11) & 0x1f, smp_processor_id());

    // 5. Set six event counter registers (Project Assignment you need to IMPLEMENT)
	asm volatile("mrc p15, 0, %0, c9, c12, 5\n\t" ::"r"(0x00000000));
	ptype = INST_RETIRED;
	asm volatile("mcr p15, 0, %0, c9, c13, 1\n\t" ::"r"(ptype));	

	asm volatile("mrc p15, 0, %0, c9, c13, 2\n\t" : "=r"(counter));

	printk("Instructions executed in counter: %d\n",counter);

	
}


int init_module(void) {
    // Set Performance Monitoring Unit (aka Performance Counter) across all cores 
    on_each_cpu(set_pmu, NULL, 1);
    printk("Ready to use PMU\n");
    return 0;
}

void cleanup_module(void) {
    printk("PMU Kernel Module Off\n");
}


