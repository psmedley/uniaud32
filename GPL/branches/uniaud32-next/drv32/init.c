/* $Id: init.c,v 1.1.1.1 2003/07/02 13:56:56 eleph Exp $ */
/*
 * Init strategy handler
 *
 * (C) 2000-2002 InnoTek Systemberatung GmbH
 * (C) 2000-2001 Sander van Leeuwen (sandervl@xs4all.nl)
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

#define INCL_NOPMAPI
#define INCL_DOSINFOSEG      // Need Global info seg in rm.cpp algorithms
#include <os2.h>
#include <string.h>

// Device support
#include <devtype.h>
#include <stacktoflat.h>
#include <version.h>
#include <ossidc32.h>
#include <dbgos2.h>
#include <osspci.h>
#include <kee.h>
#include <malloc.h>
#include "parse.h"
#include "sound/version.h"
#include "strategy.h"

const char ERR_ERROR[]   = "ERROR: ";
const char ERR_LOCK[]    = "Unable to lock 32 bit data & code segments, exiting...\r\r\n";
const char ERR_INIT[]    = "Initialization failed\r\r\n";
const char ERR_NAMES[]    = "Query names failed\r\r\n";
#ifdef DEBUG
const char szALSA[]      = "\r\n"PRODUCT_NAME" v"UNIAUD_VERSION"-DEBUG\r\nBased on Linux "ALSA_VERSION"\r\n";
#else
const char szALSA[]      = "\r\n"PRODUCT_NAME" v"UNIAUD_VERSION"\r\nBased on Linux "ALSA_VERSION"\r\n";
#endif

//const char szCopyRight1[]= "Copyright 2000-2002 InnoTek Systemberatung GmbH\r\n";
const char szCopyRight2[]= "Copyright 2000-2021 The ALSA Project\r\n\r\n";
const char szCopyRight3[]= "Copyright 2005-2021 Netlabs http://www.netlabs.org\r\n";

const char szCodeStartEnd[] = "Code 0x%0x - 0x%0x\r\n\r\n";
const char szMixerFound[]= "Detected Mixer: ";
const char szEOL[]       = "\r\n";

typedef struct {
    USHORT  MsgLength;
    WORD32  MsgPtr;
} MSG_TABLE;

extern FARPTR16 MSG_TABLE32;
extern char szLastALSAError1[];
extern char szLastALSAError2[];
extern int sprintf (char *buffer, const char *format, ...);

extern APIRET APIENTRY DOS16OPEN(PSZ pszFileName, PHFILE phf, PULONG pulAction, ULONG cbFile, ULONG ulAttribute, ULONG fsOpenFlags, ULONG fsOpenMode, PEAOP2 peaop2 );
extern APIRET APIENTRY DOS16CLOSE(HFILE hFile);
extern APIRET APIENTRY DOS16WRITE(HFILE hFile, PVOID pBuffer, ULONG cbWrite, PULONG pcbActual);
extern void SaveBuffer(void);

#define VMDHA_FIXED             0x0002

extern APIRET VMAlloc(ULONG size, ULONG flags, char NEAR* *pAddr);

//Print messages with DosWrite when init is done or has failed (see startup.asm)
void DevSaveMessage(char FAR48 *str, int length)
{
 MSG_TABLE FAR48 *msg = (MSG_TABLE FAR48 *)MAKE_FARPTR32(MSG_TABLE32);
 char FAR48 *str16 = (char FAR48 *)MAKE_FARPTR32(msg->MsgPtr);
 int i;

  for(i=0;i<length;i++) {
  str16[msg->MsgLength + i] = str[i];
  }
  str16[msg->MsgLength + length] = 0;
  msg->MsgLength += length;

  return;
}

//SvL: Lock our 32 bits data & code segments
int LockSegments(void)
{
    KEEVMLock lock;
    ULONG  segsize;

    /*
     * Locks DGROUP into physical memory
     */
    //NOTE: VMLock rounds base address down to nearest page
    //      So we MUST take this into account when calculating the
    //      size of our code/data
    segsize = OffsetFinalDS32 - ((OffsetBeginDS32) & ~0xFFF);
    if(segsize & 0xFFF) {
      segsize += PAGE_SIZE;
    }
    segsize &= ~0xFFF;

    if(KernVMLock(KEE_VML_LONGLOCK,
                  (PVOID)((OffsetBeginDS32) & ~0xFFF),
                  segsize,
                  &lock,
                  (KEEVMPageList*)-1,
                  0)) {
      return(1);
    }
    /*
     * Locks CODE32 into physical memory
     */
    segsize = OffsetFinalCS32 - ((OffsetBeginCS32) & ~0xFFF);
    if(segsize & 0xFFF) {
      segsize += PAGE_SIZE;
    }
    segsize &= ~0xFFF;

    if(KernVMLock(KEE_VML_LONGLOCK,
                  (PVOID)((OffsetBeginCS32) & ~0xFFF),
                  segsize,
                  &lock,
                  (KEEVMPageList*)-1,
                  0)) {
      return(1);
    }
    return 0;
}

// Write a string of specified length
static VOID WriteString(const char __far* str, int length)
{
    // Write the string
    DevSaveMessage((char __far *)str, length);
    return;
}

#if 0
void SaveBuffer(void)
{
    ULONG result;
    HFILE hFile;
    ULONG usAction;
    // write log to file at end of INIT strategy
    if(DOS16OPEN((PSZ)"uniaud.log", &hFile, &usAction, 0,
               FILE_NORMAL, FILE_OPEN | OPEN_ACTION_CREATE_IF_NEW,
               OPEN_ACCESS_READWRITE |
               OPEN_SHARE_DENYNONE | OPEN_FLAGS_WRITE_THROUGH,
               NULL ) == 0)
    {
        if (szprintBuf)
            DOS16WRITE(hFile, szprintBuf, wrOffset, &result);

        DOS16CLOSE(hFile);
    }

    return;
}
#endif
// Initialize device driver
WORD32 DiscardableInit(REQPACKET __far* rp)
{
  char debugmsg[64];
  char FAR48 *args;

  DebugLevel = 1;
  rp->init_out.usCodeEnd = 0;
  rp->init_out.usDataEnd = 0;

  if(LockSegments())
  {
    WriteString(ERR_ERROR, sizeof(ERR_ERROR)-1);
    WriteString(ERR_LOCK, sizeof(ERR_LOCK)-1);
    return RPDONE | RPERR_BADCOMMAND;
  }

  RMCreateDriverU32(); // register driver in  Resource Manager

  if ( szprintBuf == 0 )
  {
    VMAlloc( DBG_MAX_BUF_SIZE, VMDHA_FIXED, &szprintBuf );
    if (szprintBuf)
    {
      memset( szprintBuf, 0, DBG_MAX_BUF_SIZE );
      wrOffset= 0;
    }
  }
  if (!HeapInit(HEAP_SIZE))
  {
    rprintf(("HeapInit failed!"));
  }

  args = MAKE_FARPTR32(rp->init_in.szArgs);
  GetParms(args);

  #ifdef DEBUG
  rprintf(("Uniaud32 version %s-DEBUG",UNIAUD_VERSION));
  #else
  rprintf(("Uniaud32 version %s",UNIAUD_VERSION));
  #endif
  rprintf(("Based on linux %s",CONFIG_SND_VERSION));

  if(fVerbose)
  {
    WriteString(szALSA, sizeof(szALSA)-1);
    WriteString(szCopyRight3, sizeof(szCopyRight3)-1);
    WriteString(szCopyRight2, sizeof(szCopyRight2)-1);
  }

  if(fDebug)
  {
    sprintf(debugmsg, szCodeStartEnd, OffsetBeginCS32, OffsetFinalCS32);
    WriteString(debugmsg, strlen(debugmsg));
  }

  char szMixerName[64];
  char szDeviceName[128];

  if(OSS32_Initialize() != OSSERR_SUCCESS)
  {
    WriteString(ERR_ERROR, sizeof(ERR_ERROR)-1);
    WriteString(ERR_INIT, sizeof(ERR_INIT)-1);
      if(szLastALSAError1[0])
      {
        WriteString(szLastALSAError1, strlen(szLastALSAError1));
      }
      if(szLastALSAError2[0])
      {
        WriteString(szLastALSAError2, strlen(szLastALSAError2));
      }
    // !! dont exit when error !! return RPDONE | RPERR_COMMAND;
  }
  else if (OSS32_QueryNames(OSS32_DEFAULT_DEVICE, szDeviceName,
                              sizeof(szDeviceName),szMixerName,
                              sizeof(szMixerName), TRUE) != OSSERR_SUCCESS)
  {
    WriteString(ERR_ERROR, sizeof(ERR_ERROR)-1);
    WriteString(ERR_NAMES, sizeof(ERR_INIT)-1);
    if(szLastALSAError1[0])
    {
      WriteString(szLastALSAError1, strlen(szLastALSAError1));
    }
    if(szLastALSAError2[0])
    {
      WriteString(szLastALSAError2, strlen(szLastALSAError2));
    }

    // !! dont exit when error !! return RPDONE | RPERR_COMMAND;
  }
  else
  if(fVerbose)
  {
    WriteString(szDeviceName, strlen(szDeviceName));
    WriteString(szEOL, sizeof(szEOL)-1);
    WriteString(szMixerFound, sizeof(szMixerFound)-1);
    WriteString(szMixerName, strlen(szMixerName));
    WriteString(szEOL, sizeof(szEOL)-1);

    #if 0
    for(int i=1;i<OSS32_MAX_AUDIOCARDS;i++)
    {
      if(OSS32_QueryNames(i, szDeviceName, sizeof(szDeviceName), szMixerName, sizeof(szMixerName)) == OSSERR_SUCCESS)
      {
        WriteString(szDeviceName, strlen(szDeviceName));
        WriteString(szEOL, sizeof(szEOL)-1);
        WriteString(szMixerFound, sizeof(szMixerFound)-1);
        WriteString(szMixerName, strlen(szMixerName));
        WriteString(szEOL, sizeof(szEOL)-1);
      }
      else break;
    }
    #endif
    WriteString(szEOL, sizeof(szEOL)-1);
  }
  // Complete the installation
  rp->init_out.usCodeEnd = _OffsetFinalCS16;
  rp->init_out.usDataEnd = _OffsetFinalDS16;

  //SaveBuffer();

  // Confirm a successful installation
  return RPDONE;
}
