/* $Id: irqos2.h,v 1.1.1.1 2003/07/02 13:56:58 eleph Exp $ */
/*
 * Header for IRQ definitions & structures
 *
 * (C) 2000-2002 InnoTek Systemberatung GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
 */

#ifndef __IRQOS2_H__
#define __IRQOS2_H__

#define MAX_SHAREDIRQS		8
#define MAX_IRQ_SLOTS		8


typedef int irqreturn_t;
typedef irqreturn_t (*snd_irq_handler_t)(int, void *);
#define irq_handler_t snd_irq_handler_t
#undef irq_handler_t
#define irq_handler_t snd_irq_handler_t

typedef struct {
  irq_handler_t handler;
  ULONG x0;
  char *x1;
  void *x2;
} IRQHANDLER_INFO;

typedef struct
{
  unsigned              irqNo;
  unsigned              fEOI;
  unsigned              flHandlers;
  IRQHANDLER_INFO       irqHandlers[MAX_SHAREDIRQS];
} IRQ_SLOT;


#ifdef __cplusplus
extern "C" {
#endif

BOOL ALSA_SetIrq(ULONG ulIrq, ULONG ulSlotNo, BOOL fShared);
BOOL ALSA_FreeIrq(ULONG ulIrq);

BOOL process_interrupt(ULONG ulSlotNo, ULONG *pulIrq);

ULONG os2gettimemsec();
ULONG os2gettimesec();

#ifdef __cplusplus
}
#endif

#endif //__IRQOS2_H__

