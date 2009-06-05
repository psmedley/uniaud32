/* interrupt.h */
#ifndef _LINUX_INTERRUPT_H
#define _LINUX_INTERRUPT_H

#include <linux/kernel.h>
//#include <asm/bitops.h>
#include <asm/atomic.h>

/*
 * For 2.4.x compatibility, 2.4.x can use
 *
 *	typedef void irqreturn_t;
 *	#define IRQ_NONE
 *	#define IRQ_HANDLED
 *	#define IRQ_RETVAL(x)
 *
 * To mix old-style and new-style irq handler returns.
 *
 * IRQ_NONE means we didn't handle it.
 * IRQ_HANDLED means that we did have a valid interrupt and handled it.
 * IRQ_RETVAL(x) selects on the two depending on x being non-zero (for handled)
 */
typedef int irqreturn_t;

#define IRQ_NONE	(0)
#define IRQ_HANDLED	(1)
#define IRQ_RETVAL(x)	((x) != 0)

struct irqaction {
	void (*handler)(int, void *, struct pt_regs *);
	unsigned long flags;
	unsigned long mask;
	const char *name;
	void *dev_id;
	struct irqaction *next;
};

extern volatile unsigned char bh_running;

extern atomic_t bh_mask_count[32];
extern unsigned long bh_active;
extern unsigned long bh_mask;
extern void (*bh_base[32])(void);

void do_bottom_half(void);

/* Who gets which entry in bh_base.  Things which will occur most often
   should come first - in which case NET should be up the top with SERIAL/TQUEUE! */
   
enum {
	TIMER_BH = 0,
	CONSOLE_BH,
	TQUEUE_BH,
	DIGI_BH,
	SERIAL_BH,
	RISCOM8_BH,
	SPECIALIX_BH,
	AURORA_BH,
	ESP_BH,
	NET_BH,
	SCSI_BH,
	IMMEDIATE_BH,
	KEYBOARD_BH,
	CYCLADES_BH,
	CM206_BH,
	JS_BH,
	MACSERIAL_BH,
	ISICOM_BH
};
/* Tasklets --- multithreaded analogue of BHs.

   Main feature differing them of generic softirqs: tasklet
   is running only on one CPU simultaneously.

   Main feature differing them of BHs: different tasklets
   may be run simultaneously on different CPUs.

   Properties:
   * If tasklet_schedule() is called, then tasklet is guaranteed
     to be executed on some cpu at least once after this.
   * If the tasklet is already scheduled, but its excecution is still not
     started, it will be executed only once.
   * If this tasklet is already running on another CPU (or schedule is called
     from tasklet itself), it is rescheduled for later.
   * Tasklet is strictly serialized wrt itself, but not
     wrt another tasklets. If client needs some intertask synchronization,
     he makes it with spinlocks.
 */

struct tasklet_struct {
	struct tasklet_struct *next;	/* linked list of active bh's */
	unsigned long sync;		/* must be initialized to zero */
	void (*func)(void *);		/* function to call */
	void *data;			/* argument to function */
};

extern void tasklet_hi_schedule(struct tasklet_struct *t);

#define tasklet_schedule tasklet_hi_schedule

extern void tasklet_init(struct tasklet_struct *t,
			 void (*func)(unsigned long), unsigned long data);

/*
 * Autoprobing for irqs:
 *
 * probe_irq_on() and probe_irq_off() provide robust primitives
 * for accurate IRQ probing during kernel initialization.  They are
 * reasonably simple to use, are not "fooled" by spurious interrupts,
 * and, unlike other attempts at IRQ probing, they do not get hung on
 * stuck interrupts (such as unused PS2 mouse interfaces on ASUS boards).
 *
 * For reasonably foolproof probing, use them as follows:
 *
 * 1. clear and/or mask the device's internal interrupt.
 * 2. sti();
 * 3. irqs = probe_irq_on();      // "take over" all unassigned idle IRQs
 * 4. enable the device and cause it to trigger an interrupt.
 * 5. wait for the device to interrupt, using non-intrusive polling or a delay.
 * 6. irq = probe_irq_off(irqs);  // get IRQ number, 0=none, negative=multiple
 * 7. service the device to clear its pending interrupt.
 * 8. loop again if paranoia is required.
 *
 * probe_irq_on() returns a mask of allocated irq's.
 *
 * probe_irq_off() takes the mask as a parameter,
 * and returns the irq number which occurred,
 * or zero if none occurred, or a negative irq number
 * if more than one irq occurred.
 */
extern unsigned long probe_irq_on(void);	/* returns 0 on failure */
extern int probe_irq_off(unsigned long);	/* returns 0 or negative on failure */

#endif
