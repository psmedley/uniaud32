#ifndef _ASM_IO_H
#define _ASM_IO_H

#include <asm/pgtable.h>
/*
 * This file contains the definitions for the x86 IO instructions
 * inb/inw/inl/outb/outw/outl and the "string versions" of the same
 * (insb/insw/insl/outsb/outsw/outsl). You can also use "pausing"
 * versions of the single-IO instructions (inb_p/inw_p/..).
 *
 * This file is not meant to be obfuscating: it's just complicated
 * to (a) handle it all in a way that makes gcc able to optimize it
 * as well as possible and (b) trying to avoid writing the same thing
 * over and over again with slight variations and possibly making a
 * mistake somewhere.
 */

extern void * __ioremap(unsigned long offset, unsigned long size, unsigned long flags);

#define ioremap(offset, size) __ioremap(offset, size, 0)

/*
 * This one maps high address device memory and turns off caching for that area.
 * it's useful if some control registers are in such an area and write combining
 * or read caching is not desirable:
 */
#define ioremap_nocache(offset, size) __ioremap(offset, size, _PAGE_PCD)

extern void iounmap(void *addr);

/*
 * readX/writeX() are used to access memory mapped devices. On some
 * architectures the memory mapped IO stuff needs to be accessed
 * differently. On the x86 architecture, we just read/write the
 * memory location directly.
 */

#define readb(addr) (*(volatile unsigned char *) __io_virt(addr))
#define readw(addr) (*(volatile unsigned short *) __io_virt(addr))
#define readl(addr) (*(volatile unsigned int *) __io_virt(addr))
#define __raw_readb readb
#define __raw_readw readw
#define __raw_readl readl

#define writeb(b,addr) (*(volatile unsigned char *) __io_virt(addr) = (b))
#define writew(b,addr) (*(volatile unsigned short *) __io_virt(addr) = (b))
#define writel(b,addr) (*(volatile unsigned int *) __io_virt(addr) = (b))
#define __raw_writeb writeb
#define __raw_writew writew
#define __raw_writel writel

#define __io_virt(x) ((void *)(x))
#define memset_io(a,b,c)	memset(__io_virt(a),(b),(c))
#define memcpy_fromio(a,b,c)	memcpy((a),__io_virt(b),(c))
#define memcpy_toio(a,b,c)	memcpy(__io_virt(a),(b),(c))

unsigned int ioread8(void __iomem *addr);
unsigned int ioread16(void __iomem *addr);
unsigned int ioread32(void __iomem *addr);
void iowrite8(u8 val, void __iomem *addr);
void iowrite16(u16 val, void __iomem *addr);
void iowrite32(u32 val, void __iomem *addr);
void __iomem *ioport_map(unsigned long port, unsigned int nr);
void ioport_unmap(void __iomem *addr);
void __iomem *pci_iomap(struct pci_dev *dev, int bar, unsigned long maxlen);
void pci_iounmap(struct pci_dev *dev, void __iomem * addr);

/*
 * Thanks to James van Artsdalen for a better timing-fix than
 * the two short jumps: using outb's to a nonexistent port seems
 * to guarantee better timings even on fast machines.
 *
 * On the other hand, I'd like to be sure of a non-existent port:
 * I feel a bit unsafe about using 0x80 (should be safe, though)
 *
 *		Linus
 */

 /*
  *  Bit simplified and optimized by Jan Hubicka
  *  Support of BIGMEM added by Gerhard Wichert, Siemens AG, July 1999.
  *
  *  isa_memset_io, isa_memcpy_fromio, isa_memcpy_toio added,
  *  isa_read[wl] and isa_write[wl] fixed
  *  - Arnaldo Carvalho de Melo <acme@conectiva.com.br>
  */

#ifdef SLOW_IO_BY_JUMPING
#define __SLOW_DOWN_IO "\njmp 1f\n1:\tjmp 1f\n1:"
#else
#define __SLOW_DOWN_IO "\noutb %%al,$0x80"
#endif

#ifdef REALLY_SLOW_IO
#define __FULL_SLOW_DOWN_IO __SLOW_DOWN_IO __SLOW_DOWN_IO __SLOW_DOWN_IO __SLOW_DOWN_IO
#else
#define __FULL_SLOW_DOWN_IO __SLOW_DOWN_IO
#endif

/*
 * Talk about misusing macros..
 */
#define __OUT1(s,x) \
extern inline void out##s(unsigned x value, unsigned short port) {


#define __IN1(s) \
extern inline RETURN_TYPE in##s(unsigned short port) { RETURN_TYPE _v;


void outb(unsigned char data, unsigned short port);
#pragma aux outb =       \
  "out dx, al"                  \
  parm [al] [dx];

void outsb(unsigned short port, char *buffer, int size);
#pragma aux outsb =       \
  "outsb"                  \
  parm [dx] [esi] [ecx];

#if 1
unsigned char inb(unsigned short port);
#pragma aux inb =       \
  "in al,dx"            \
  parm [dx]             \
  value [al];
#endif

void insb(unsigned short port, char *buffer, int size);
#pragma aux insb =       \
  "insb"                  \
  parm [dx] [esi] [ecx];

void outw(unsigned short data, unsigned short port);
#pragma aux outw =       \
  "out dx, ax"                  \
  parm [ax] [dx];

void outsw(unsigned short port, void *buffer, int size);
#pragma aux outsw =       \
  "outsw"                  \
  parm [dx] [edi] [ecx];

unsigned short inw(unsigned short port);
#pragma aux inw =       \
  "in ax,dx"            \
  parm [dx]             \
  value [ax];

void outl(unsigned long data, unsigned short port);
#pragma aux outl =       \
  "out dx, eax"                  \
  parm [eax] [dx];

unsigned long inl(unsigned short port);
#pragma aux inl =       \
  "in eax,dx"            \
  parm [dx]             \
  value [eax];

#endif
