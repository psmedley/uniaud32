/* $Id: malloc.h,v 1.1.1.1 2003/07/02 13:56:58 eleph Exp $ */
/*
 * Header file for simple ring 0 OS/2 heap
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

#ifndef MALLOC_INCLUDED
#define MALLOC_INCLUDED

// Standard malloc.h functions

//NOTE: enabling this in the non-KEE driver causes problems (file name strings
//      put in seperate private segments)
#ifdef DEBUGHEAP
void NEAR *malloc(unsigned size, const char *filename, int lineno);
void       free(void *NEAR ptr, const char *filename, int lineno);
void NEAR *realloc(void *NEAR ptr, unsigned newsize, const char *filename, int lineno);
#else
void NEAR *malloc(unsigned);
void       free(void NEAR *);
void NEAR *realloc(void NEAR *, unsigned);
#endif

unsigned _msize(void NEAR *);


// // Traditional 'C' library memset() -- don't need, intrinsic function !!!
// PVOID memset ( PVOID p, int c, USHORT len );

// Some extensions
unsigned _memfree(void);            // returns available space

// Specialized routines
unsigned HeapInit(unsigned);        // initializes the heap manager
void dumpheap(void);

BOOL IsHeapAddr(ULONG addr);

#endif
