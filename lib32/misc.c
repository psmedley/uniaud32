/* $Id: misc.c,v 1.1.1.1 2003/07/02 13:57:02 eleph Exp $ */
/*
 * OS/2 implementation of misc. Linux kernel services
 *
 * (C) 2000-2002 InnoTek Systemberatung GmbH
 * (C) 2000-2001 Sander van Leeuwen (sandervl@xs4all.nl)
 *
 * hweight32 based on Linux code (bitops.h)
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

#include "linux.h"
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/poll.h>
#define CONFIG_PROC_FS
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <asm/hardirq.h>
#include <linux\ioport.h>
#include <linux\utsname.h>
#include <linux\module.h>
#include <dbgos2.h>
#include <printfos2.h>
#include <limits.h>

struct new_utsname system_utsname = {0};
struct resource ioport_resource = {NULL, 0, 0, IORESOURCE_IO, NULL, NULL, NULL};
struct resource iomem_resource  = {NULL, 0, 0, IORESOURCE_MEM, NULL, NULL, NULL};
mem_map_t *mem_map = 0;
int this_module[64] = {0};
                
#include <stdarg.h>

char szLastALSAError1[128] = {0};
char szOverrunTest1        = 0xCC;
char szLastALSAError2[128] = {0};
char szOverrunTest2        = 0xCC;
int  iLastError            = 0;

//******************************************************************************
//Save error message in szLastALSAError; if card init failed, then we will
//print it in drv32\init.cpp
//******************************************************************************
int printk(const char * fmt, ...)
{
    va_list argptr;                /* -> variable argument list */

    char *pszLastALSAError;
    
    pszLastALSAError= iLastError ? szLastALSAError2 : szLastALSAError1;

    va_start(argptr, fmt);           /* get pointer to argument list */
    vsprintf(pszLastALSAError, fmt, argptr);
//    strcat(pszLastALSAError, "\r");
    va_end(argptr);                /* done with variable arguments */

    if(szOverrunTest1 != 0xCC || szOverrunTest2 != 0xCC) {
        DebugInt3();
    }
    
    dprintf( (pszLastALSAError) );
    if(++iLastError > 1) {
        iLastError = 0;
    }
    return 0;
}
//******************************************************************************
//******************************************************************************
void schedule(void)
{

}
//******************************************************************************
//******************************************************************************
void poll_wait(struct file * filp, wait_queue_head_t * wait_address, poll_table *p)
{

}
//******************************************************************************
//******************************************************************************
int __check_region(struct resource *a, unsigned long b, unsigned long c)
{
    DebugInt3();
    return 0;
}

/* --------------------------------------------------------------------- */
/*
 * hweightN: returns the hamming weight (i.e. the number
 * of bits set) of a N-bit word
 */

#ifdef hweight32
#undef hweight32
#endif

unsigned int hweight32(unsigned int w)
{
	unsigned int res = (w & 0x55555555) + ((w >> 1) & 0x55555555);
	res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
	res = (res & 0x0F0F0F0F) + ((res >> 4) & 0x0F0F0F0F);
	res = (res & 0x00FF00FF) + ((res >> 8) & 0x00FF00FF);
	return (res & 0x0000FFFF) + ((res >> 16) & 0x0000FFFF);
}
//******************************************************************************
//******************************************************************************
mem_map_t *virt_to_page(int x)
{
    static mem_map_t map = {0};
    return &map;
}
//******************************************************************************
//******************************************************************************
struct proc_dir_entry proc_root = {0};
//******************************************************************************
//******************************************************************************
struct proc_dir_entry *create_proc_entry(const char *name, mode_t mode,
				         struct proc_dir_entry *parent)
{
    struct proc_dir_entry *proc;

    proc = (struct proc_dir_entry *)kmalloc(sizeof(struct proc_dir_entry), 0);
    memset(proc, 0, sizeof(struct proc_dir_entry));

    proc->name   = name;
    proc->mode   = mode;
    proc->parent = parent;

    return proc;
}
//******************************************************************************
//******************************************************************************
void remove_proc_entry(const char *name, struct proc_dir_entry *parent)
{
    return; //memory leak
}
//******************************************************************************
//******************************************************************************
int proc_register(struct proc_dir_entry *parent, struct proc_dir_entry *proc)
{
    return 0;
}
//******************************************************************************
//******************************************************************************
int proc_unregister(struct proc_dir_entry *proc, int bla)
{
    return 0;
}
//******************************************************************************
//******************************************************************************
int fasync_helper(int a, struct file *b, int c, struct fasync_struct **d)
{
    return 0;
}
//******************************************************************************
//******************************************************************************
void kill_fasync(struct fasync_struct *a, int b, int c)
{
}
//******************************************************************************
//******************************************************************************
int request_dma(unsigned int dmanr, const char * device_id)	/* reserve a DMA channel */
{
    DebugInt3();
    return 0;
}
//******************************************************************************
//******************************************************************************
void free_dma(unsigned int dmanr)
{
    DebugInt3();
}
//******************************************************************************
/* enable/disable a specific DMA channel */
//******************************************************************************
void enable_dma(unsigned int dmanr)
{
    DebugInt3();
}
//******************************************************************************
//******************************************************************************
void disable_dma(unsigned int dmanr)
{
    DebugInt3();
}
//******************************************************************************
static struct notifier_block *reboot_notify_list = NULL;
// No need to implement this right now. The ESS Maestro 3 driver uses it
// to call pci_unregister_driver, which is always called from the shutdown
// notification sent by OS2.
// Same goes for es1968 & Yamaha's DS1/DS1E.
//******************************************************************************
int register_reboot_notifier(struct notifier_block *pnblock)
{
    return 0;
}
//******************************************************************************
//******************************************************************************
int unregister_reboot_notifier(struct notifier_block *pnblock)
{
    return 0;
}
//******************************************************************************
//******************************************************************************
void *snd_compat_kcalloc(size_t n, size_t size, int flags)
{
        void *ret = NULL;

        if (n != 0 && size > INT_MAX / n)
                return ret;

        ret = kmalloc(n * size, flags);
        if (ret)
                memset(ret, 0, n * size);
        return ret;
}

