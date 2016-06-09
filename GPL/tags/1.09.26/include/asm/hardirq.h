#ifndef __ASM_HARDIRQ_H
#define __ASM_HARDIRQ_H

extern unsigned int local_irq_count[1];

/*
 * Are we in an interrupt context? Either doing bottom half
 * or hardware interrupt processing?
 */
int in_interrupt();

#define hardirq_trylock(cpu)	(local_irq_count[cpu] == 0)
#define hardirq_endlock(cpu)	do { } while (0)

#define hardirq_enter(cpu)	(local_irq_count[cpu]++)
#define hardirq_exit(cpu)	(local_irq_count[cpu]--)

#define synchronize_irq()	

#endif /* __ASM_HARDIRQ_H */
