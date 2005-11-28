#ifndef __ASM_SYSTEM_H
#define __ASM_SYSTEM_H

#include <linux/kernel.h>
#include <asm/segment.h>

#ifdef __KERNEL__

struct task_struct;	/* one of the stranger aspects of C forward declarations.. */
extern void FASTCALL(__switch_to(struct task_struct *prev, struct task_struct *next));

/*
 * disable hlt during certain critical i/o operations
 */
#define HAVE_DISABLE_HLT
void disable_hlt(void);
void enable_hlt(void);

void save_flags(u32 *flags);
#pragma aux save_flags =       \
  "pushfd"   \
  "mov dword ptr [eax], edx" \
  modify [edx] \
  parm [eax];

void restore_flags(u32 flags);
#pragma aux restore_flags =       \
  "push eax"   \
  "popfd" \
  parm [eax];

#define cli() 			_asm cli;
#define sti()			_asm sti;

#endif
#endif
