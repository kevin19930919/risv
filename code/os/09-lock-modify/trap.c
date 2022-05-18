#include "trap.h"
#include "os.h"

static const char *exception_type[EXCEPTION_TYPE_NUM] = {
	"instruction misaligned",
	"instruction fault",
	"illegal instruction",
	"breakpoint",
	"load misaligned",
	"load fault",
	"store/amo misaligned",
	"store/amo fault",
	"syscall from U mode",
	"syscall from S mode",
	"reserved",
	"syscall from M mode",
	"instruction page fault",
	"load page fault",
	"reserved for future",
	"store/amo page fault"};

extern void vector_table(void);
extern void uart_isr(void);
extern void timer_handler(void);
extern void schedule(void);

void trap_init()
{
	/*
	 * set the trap-vector base-address for machine-mode
	 */
	w_mtvec((reg_t)vector_table | 0x1);
}

void default_handler(void)
{
	printf("default_hanlder\n");
}

void software_interrupt_handler(void)
{
	uart_puts("software interruption!\n");
	/*
	 * acknowledge the software interrupt by clearing 			 
     * the MSIP bit in mip.
	 */
	int id = r_mhartid();
	*(uint32_t *)CLINT_MSIP(id) = 0;

	schedule();
	uart_puts("software interruption exit\n");
}

void external_interrupt_handler()
{
	int irq = plic_claim();
	uart_puts("external interruption!\n");
	if (irq == UART0_IRQ)
	{
		uart_isr();
	}
	else if (irq)
	{
		printf("unexpected interrupt irq = %d\n", irq);
	}

	if (irq)
	{
		plic_complete(irq);
	}
}

reg_t trap_handler(reg_t epc, reg_t cause, reg_t tval)
{
	reg_t return_pc = epc;
	reg_t cause_code = cause & 0xfff;

	/* Synchronous trap - exception */
	if (cause_code < EXCEPTION_TYPE_NUM)
		printf("exception type: %s\n", exception_type[cause_code]);
	printf("mepc = %p, mtval = %p\n", epc, tval);
	printf("Sync exceptions!, code = %d\n", cause_code);
	panic("OOPS! What can I do!");
	// return_pc += 4;

	return return_pc;
}

void trap_test()
{
	/*
	 * Synchronous exception code = 7
	 * Store/AMO access fault
	 */
	*(int *)0x00000000 = 100;

	/*
	 * Synchronous exception code = 5
	 * Load access fault
	 */
	//int a = *(int *)0x00000000;

	uart_puts("Yeah! I'm return back from trap!\n");
}
