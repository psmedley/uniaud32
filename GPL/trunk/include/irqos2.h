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

//#define MAX_SHAREDIRQS          16
//#define MAX_IRQS		16
#define MAX_SHAREDIRQS          256 // ACPI
#define MAX_IRQS                256 // ACPI

//typedef void (NEAR * IRQHANDLER)(int, void *, void *);
typedef int (NEAR * IRQHANDLER)(int, void *, void *);

typedef struct {
  IRQHANDLER handler;
  ULONG x0;
  char *x1;
  void *x2;
} IRQHANDLER_INFO;

typedef BOOL (*PFNIRQ)(int irq);

#ifdef __cplusplus
extern "C" {
#endif

BOOL RMSetIrq(ULONG ulIrq, BOOL fShared, PFNIRQ pfnIrqHandler);
BOOL RMFreeIrq(ULONG ulIrq);

BOOL oss_process_interrupt(int irq);

ULONG os2gettimemsec();
ULONG os2gettimesec();


#ifdef __cplusplus
}
#endif

#endif //__IRQOS2_H__

