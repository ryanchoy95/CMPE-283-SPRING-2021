/*  
 *  cmpe283-1.c - Kernel module for CMPE283 assignment 1
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <asm/msr.h>

#define MAX_MSG 80

/*
 * Model specific registers (MSRs) by the module.
 * See SDM volume 4, section 2.1
 */
#define IA32_VMX_PINBASED_CTLS	0x00000481
#define IA32_VMX_PROCBASED_CTLS	0x00000482
#define IA32_VMX_PROCBASED_CTLS2	0x0000048B
#define IA32_VMX_EXIT_CTLS		0x00000483
#define IA32_VMX_ENTRY_CTLS		0x00000484



/*
 * struct caapability_info
 *
 * Represents a single capability (bit number and description).
 * Used by report_capability to output VMX capabilities.
 */
struct capability_info {
	uint8_t bit;
	const char *name;
};


/*
 * Pinbased capabilities
 * See SDM volume 3, section 24.6.1
 */
struct capability_info pinbased[5] =
{
	{ 0, "External Interrupt Exiting" },
	{ 3, "NMI Exiting" },
	{ 5, "Virtual NMIs" },
	{ 6, "Activate VMX Preemption Timer" },
	{ 7, "Process Posted Interrupts" }
};

struct capability_info procbased1[21] =
{
	{ 2, "INTERRUPT-WINDOW EXITING" },
	{ 3, "USE TSC OFFSETTING" },
	{ 7, "HLT EXITING" },
	{ 9, "INVLPG EXITING" },
	{ 10, "MWAIT EXITING" },
	{ 11, "RDPMC EXITING" },
	{ 12, "RDTSC EXITING" },
	{ 15, "CR3-LOAD EXITING" },
	{ 16, "CR3-STORE EXITING" },
	{ 19, "CR8-LOAD EXITING" },
	{ 20, "CR8-STORE EXITING" },
	{ 21, "USE TPR SHADOW" },
	{ 22, "NMI-WINDOW EXITING" },
	{ 23, "MOV-DR EXITING" },
	{ 24, "UNCONDITIONAL I/O EXITING" },
	{ 25, "USE I/O BITMAPS" },
	{ 27, "MONITOR TRAP FLAG" },
	{ 28, "USE MSR BITMAPS" },
	{ 29, "MONITOR EXITING" },
	{ 30, "PAUSE EXITING" },
	{ 31, "ACTIVATE SECONDARY CONTROLS" }
	
};

struct capability_info procbased2[23] =
{
    { 0, "VIRTUALIZE APIC ACCESSES" },
    { 1, "ENABLE EPT" },
    { 2, "DESCRIPTOR-TABLE EXITING" },
    { 3, "ENABLE RDTSCP" },
    { 4, "VIRTUALIZE x2APIC MODE" },
    { 5, "ENABLE VPID" },
    { 6, "WBINVD EXITING" },
    { 7, "UNRESTRICTED GUEST" },
    { 8, "APIC-REGISTER VIRTUALIZATION" },
    { 9, "VIRTUAL-INTERRUPT DELIVERY" },
    { 10, "PAUSE-LOOP EXITING" },
    { 11, "RDRAND EXITING" },
    { 12, "ENABLE INVPCID" },
    { 13, "ENABLE VM FUNCTIONS" },
    { 14, "VMCS SHADOWING" },
    { 15, "ENABLE ENCLS EXITING" },
    { 16, "RDSEED EXITING" },
    { 17, "ENABLE PML" },
    { 18, "EPT-VIOLATION #VE" },
    { 19, "CONCEAL VMX FROM PT" },
    { 20, "ENABLE XSAVES/XRSTORS" },
    { 22, "MODE-BASED EXECUTE CONTROL FOR EPT" },
    { 25, "USE TSC SCALING" }
};

struct capability_info exitbased[11] =
{
    { 2, "SAVE DEBUG CONTROLS" },
    { 9, "HOST ADDRESS-SPACE SIZE" },
    { 12, "LAOD IA32_PERF_GLOBAL_CTRL" },
    { 15, "ACKNOWLEDGE INTERRUPT ON EXIT" },
    { 18, "SAVE IA32_PAT" },
    { 19, "LOAD IA32_PAT" },
    { 20, "SAVE IA32_EFER" },
    { 21, "LOAD IA32_EFER" },
    { 22, "SAVE VMX-PREEMPTION TIMER VALUE" },
    { 23, "CLEAR IA32_BNDCFGS" },
    { 24, "CONCEAL VM EXITS FROM INTEL PT" }
};

struct capability_info entrybased[9] =
{
	{ 2, "LOAD DEBUG CONTROLS" },
	{ 9, "IA-32e MODE GUEST" },
	{ 10, "ENTRY TO SMM" },
	{ 11, "DEACTIVATE DUAL-MONITOR TREATMENT" },
	{ 13, "LOAD IA32_PERF_GLOBAL_CTRL" },
	{ 14, "LOAD IA32_PAT" },
	{ 15, "LOAD IA32_EFER" },
	{ 16, "LOAD IA32_BNDCFGS" },
	{ 17, "CONCEAL VM ENTRIES FROM INTEL PT" }
};
/*
 * report_capability
 *
 * Reports capabilities present in 'cap' using the corresponding MSR values
 * provided in 'lo' and 'hi'.
 *
 * Parameters:
 *  cap: capability_info structure for this feature
 *  len: number of entries in 'cap'
 *  lo: low 32 bits of capability MSR value describing this feature
 *  hi: high 32 bits of capability MSR value describing this feature
 */
void
report_capability(struct capability_info *cap, uint8_t len, uint32_t lo,
    uint32_t hi)
{
	uint8_t i;
	struct capability_info *c;
	char msg[MAX_MSG];

	memset(msg, 0, sizeof(msg));

	for (i = 0; i < len; i++) {
		c = &cap[i];
		snprintf(msg, 79, "  %s: Can set=%s, Can clear=%s\n",
		    c->name,
		    (hi & (1 << c->bit)) ? "Yes" : "No",
		    !(lo & (1 << c->bit)) ? "Yes" : "No");
		printk(msg);
	}
}

/*
 * detect_vmx_features
 *
 * Detects and prints VMX capabilities of this host's CPU.
 */
void
detect_vmx_features(void)
{
	uint32_t lo, hi;

	/* Pinbased controls */
	rdmsr(IA32_VMX_PINBASED_CTLS, lo, hi);
	pr_info("Pinbased Controls MSR: 0x%llx\n",
		(uint64_t)(lo | (uint64_t)hi << 32));
	report_capability(pinbased, 5, lo, hi);
	/* Procbased 1 controls */
	rdmsr(IA32_VMX_PROCBASED_CTLS, lo, hi);
	pr_info("Proc based Controls 1 MSR: 0x%llx\n",
		(uint64_t)(lo | (uint64_t)hi << 32));
	report_capability(procbased1, 21, lo, hi);
	/* Procbased 2 controls */
	rdmsr(IA32_VMX_PROCBASED_CTLS2, lo, hi);
	pr_info("Proc based Controls 23 MSR: 0x%llx\n",
		(uint64_t)(lo | (uint64_t)hi << 32));
	report_capability(procbased2, 5, lo, hi);
	/* Exit controls */
	rdmsr(IA32_VMX_EXIT_CTLS, lo, hi);
	pr_info("Exit Controls MSR: 0x%llx\n",
		(uint64_t)(lo | (uint64_t)hi << 32));
	report_capability(exitbased, 11, lo, hi);
	/* Entry controls */
	rdmsr(IA32_VMX_ENTRY_CTLS, lo, hi);
	pr_info("Entry Controls MSR: 0x%llx\n",
		(uint64_t)(lo | (uint64_t)hi << 32));
	report_capability(entrybased, 9, lo, hi);
}

/*
 * init_module
 *
 * Module entry point
 *
 * Return Values:
 *  Always 0
 */
int
init_module(void)
{
	printk(KERN_INFO "CMPE 283 Assignment 1 Module Start\n");

	detect_vmx_features();

	/* 
	 * A non 0 return means init_module failed; module can't be loaded. 
	 */
	return 0;
}

/*
 * cleanup_module
 *
 * Function called on module unload
 */
void
cleanup_module(void)
{
	printk(KERN_INFO "CMPE 283 Assignment 1 Module Exits\n");
}
