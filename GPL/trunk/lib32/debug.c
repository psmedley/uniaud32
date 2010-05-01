/* $Id: debug.c,v 1.1.1.1 2003/07/02 13:57:02 eleph Exp $ */
/*
 * COM port debugging functions
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
#define INCL_DOSERRORS           // for ERROR_INVALID_FUNCTION
#include <os2.h>

#include <string.h>

#define CR 0x0d
#define LF 0x0a

#define LEADING_ZEROES          0x8000
#define SIGNIFICANT_FIELD       0x0007

//#define COMM_DEBUG

BOOL  fLineTerminate=TRUE;
int   DebugLevel = 1;

extern int wrOffset;
extern char *szprintBuf;
extern int max_buf_size;

char hextab[]="0123456789ABCDEF";

                                        //-------------------- DecLongToASCII -
char * DecLongToASCII(char *StrPtr, ULONG lDecVal,USHORT Option)
{
   BOOL  fNonZero=FALSE;
   ULONG Digit;
   ULONG Power=1000000000;                      // 1 billion
   LONG  lVal = (LONG)lDecVal;

   if(lVal < 0) {
       *StrPtr = '-';
       StrPtr++;
       lDecVal = -lDecVal;
   }

   while (Power)
      {
      Digit=0;                                                                        // Digit=lDecVal/Power
      while (lDecVal >=Power)                   // replaced with while loop
         {
         Digit++;
         lDecVal-=Power;
         }

      if (Digit)
         fNonZero=TRUE;

      if (Digit ||
          fNonZero ||
          (Option & LEADING_ZEROES) ||
          ((Power==1) && (fNonZero==FALSE)))
         {
         *StrPtr=(char)('0'+Digit);
         StrPtr++;
         }

      if (Power==1000000000)                    // 1 billion
         Power=100000000;
      else if (Power==100000000)
         Power=10000000;
      else if (Power==10000000)
         Power=1000000;
      else if (Power==1000000)
         Power=100000;
      else if (Power==100000)
         Power=10000;
      else if (Power==10000)
         Power=1000;
      else if (Power==1000)
         Power=100;
      else if (Power==100)
         Power=10;
      else if (Power==10)
         Power=1;
      else
         Power=0;
      }
   return (StrPtr);
}
                                        //-------------------- HexWordToASCII -
                                        //-------------------- HexLongToASCII -
char  * HexLongToASCII(char  *StrPtr, ULONG wHexVal, USHORT Option)
{
   BOOL  fNonZero=FALSE;
   ULONG Digit;
   ULONG Power=0xF0000000;
   ULONG ShiftVal=28;

   while (Power)
      {
      Digit=(wHexVal & Power)>>ShiftVal;
      if (Digit)
         fNonZero=TRUE;

      if (Digit ||
          fNonZero ||
          (Option & LEADING_ZEROES) ||
          ((Power==0x0F) && (fNonZero==FALSE)))
          *StrPtr++=hextab[Digit];

      if (Power==0xF0000000)                  // 1 billion
         Power=0xF000000;
      else if (Power==0xF000000)
         Power=0xF00000;
      else if (Power==0xF00000)
         Power=0xF0000;
      else if (Power==0xF0000)
         Power=0xF000;
      else if (Power==0xF000)
         Power=0xF00;
      else if (Power==0xF00)
         Power=0xF0;
      else if (Power==0xF0)
         Power=0xF;
      else Power=0;

      ShiftVal-=4;
      } // end while

   return (StrPtr);
}

#ifdef DEBUG
//------------------------- StringOut --------------------------//

#define VMDHA_FIXED             0x0002

extern APIRET VMAlloc(ULONG size, ULONG flags, char NEAR* *pAddr);

/**
 * Finds the length of a string up to cchMax.
 * @returns   Length.
 * @param     psz     Pointer to string.
 * @param     cchMax  Max length.
 */
static unsigned _strnlen(const char *psz, unsigned cchMax)
{
    const char *pszC = psz;

    while (cchMax-- > 0 &&  *psz != '\0')
        psz++;

    return psz - pszC;
}

#ifdef COMM_DEBUG
short int MAGIC_COMM_PORT =  0; // pulled from word ptr 40:0


#define UART_DATA               0x00            // UART Data port
#define UART_INT_ENAB           0x01            // UART Interrupt enable
#define UART_INT_ID             0x02            // interrupt ID
#define UART_LINE_CTRL          0x03            // line control registers
#define UART_MODEM_CTRL         0x04            // modem control register
#define UART_LINE_STAT          0x05            // line status register
#define UART_MODEM_STAT         0x06            // modem status regiser
#define UART_DIVISOR_LO         0x00            // divisor latch least sig
#define UART_DIVISOR_HI         0x01h           // divisor latch most sig

#define DELAY   nop

void CharOut(char c)
{
    if( MAGIC_COMM_PORT )
    {

        _asm    {

        mov     dx, MAGIC_COMM_PORT     // address of PS/2's first COM port
        add     dx, UART_LINE_STAT

ReadyCheck:
        in      al, dx                                                          // wait for comm port ready signal

        DELAY
        DELAY
        DELAY

        test    al, 020h
        jz      ReadyCheck

        // Send the character

        add     dx, UART_DATA - UART_LINE_STAT
        mov     al,c
        out     dx, al

        DELAY
        DELAY
        DELAY
        }
    }
}
#endif

void StringOut(char *DbgStr)
{
   int len;
#ifdef COMM_DEBUG
   int i;
#endif /* DEBUG */

   len= _strnlen( DbgStr, 1024 );
/*
   while (*DbgStr)
      CharOut(*DbgStr++);
      */
#ifdef COMM_DEBUG
   if (MAGIC_COMM_PORT)   //PS+++ If have comport - out to it
   {
       for( i= 0; i < len; i++ )
           CharOut( DbgStr[i] );

       if (fLineTerminate)
       {
           CharOut(CR);                              // append carriage return,
           CharOut(LF);                              // linefeed
       }
   }
#endif
   if( szprintBuf == 0 )
   {
      VMAlloc( max_buf_size, VMDHA_FIXED, &szprintBuf );
      if( szprintBuf )
         memset( szprintBuf, 0, max_buf_size );
      wrOffset= 0;
   }
   if( szprintBuf )
   {
       if( (len + wrOffset) > max_buf_size )
       {
          int cntr;
          cntr= max_buf_size - wrOffset;
          memcpy( szprintBuf +  wrOffset, DbgStr, cntr );
          DbgStr+= cntr;
          len= len - cntr;
          wrOffset= 0;
       }
       if( len )
       {
          memcpy( szprintBuf + wrOffset, DbgStr, len );
          wrOffset= wrOffset + len;
          if( wrOffset >= max_buf_size )
              wrOffset= 0;
       }
       if (fLineTerminate)
       {
//           if( (wrOffset+1) >= max_buf_size )
//               wrOffset= 0;
           szprintBuf[wrOffset]= CR;
           if( ++wrOffset >= max_buf_size )
               wrOffset= 0;
           szprintBuf[wrOffset]= LF;
           if( ++wrOffset >= max_buf_size )
               wrOffset= 0;
       }
   }
}
#endif

#ifdef  DEBUG
char BuildString[1024];
#endif          // DEBUG

//------------------------- PrintfOut -
void _cdecl DPD(int level, char *DbgStr, ...)
{
#ifdef DEBUG
   char *BuildPtr=BuildString;
   char *pStr=(char *) DbgStr;
   char *SubStr;
   union {
         void   *VoidPtr;
         USHORT *WordPtr;
         ULONG  *LongPtr;
         ULONG  *StringPtr;
         } Parm;
   USHORT wBuildOption;

   Parm.VoidPtr=(void *) &DbgStr;
   Parm.StringPtr++;                            // skip size of string pointer

   while (*pStr)
      {
      // don't overflow target
      if (BuildPtr >= (char *) &BuildString[1024-2])
         break;

      switch (*pStr)
         {
         case '%':
            wBuildOption=0;
            pStr++;
            if (*pStr=='0')
               {
               wBuildOption|=LEADING_ZEROES;
               pStr++;
               }
            if (*pStr=='u')                                                         // always unsigned
               pStr++;
            if (*pStr=='#')
               pStr++;

            switch(*pStr)
               {
               case 'x':
	       case 'X':
	       case 'p':
               case 'P':
                  BuildPtr=HexLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                  pStr++;
                  continue;

               case 'd':
                  BuildPtr=DecLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                  pStr++;
                  continue;

               case 's':
                  SubStr=(char *)*Parm.StringPtr;
                  while (*BuildPtr++ = *SubStr++);
                  Parm.StringPtr++;
                  BuildPtr--;                      // remove the \0
                  pStr++;
                  continue;

               case 'l':
                  pStr++;
                  switch (*pStr)
                  {
                  case 'x':
                  case 'X':
                  BuildPtr=HexLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                  pStr++;
                  continue;

                  case 'd':
                     BuildPtr=DecLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                     pStr++;
                     continue;
                  } // end switch
                  continue;                        // dunno what he wants

               case 0:
                  continue;
               } // end switch
            break;

      case '\\':
         pStr++;
         switch (*pStr)
            {
            case 'n':
            *BuildPtr++=LF;
            pStr++;
            continue;

            case 'r':
            *BuildPtr++=CR;
            pStr++;
            continue;

            case 0:
            continue;
            break;
            } // end switch

         break;
         } // end switch

      *BuildPtr++=*pStr++;
      } // end while

   *BuildPtr=0;                                 // cauterize the string
   StringOut((char *) BuildString);
#endif                            //DEBUG
}


void _cdecl DPE(char *DbgStr, ...)
{
#ifdef DEBUG
   char *BuildPtr=BuildString;
   char *pStr = (char *) DbgStr;
   char *SubStr;
   union {
         void   *VoidPtr;
         USHORT *WordPtr;
         ULONG  *LongPtr;
         ULONG  *StringPtr;
         } Parm;
   USHORT wBuildOption;

   Parm.VoidPtr=(void *) &DbgStr;
   Parm.StringPtr++;                            // skip size of string pointer

   while (*pStr)
      {
      // don't overflow target
      if (BuildPtr >= (char *) &BuildString[1024-2])
         break;

      switch (*pStr)
         {
         case '%':
            wBuildOption=0;
            pStr++;
            if (*pStr=='0')
               {
               wBuildOption|=LEADING_ZEROES;
               pStr++;
               }
//            if (*pStr=='u')                                                         // always unsigned
//               pStr++;

            switch(*pStr)
               {
               case 'x':
	       case 'X':
	       case 'p':
               case 'P':
                  BuildPtr=HexLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                  pStr++;
                  continue;

               case 'd':
               case 'u':
                  BuildPtr=DecLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                  pStr++;
                  continue;

               case 's':
                  SubStr=(char *)*Parm.StringPtr;
                  while (*BuildPtr++ = *SubStr++);
                  Parm.StringPtr++;
                  BuildPtr--;                      // remove the \0
                  pStr++;
                  continue;

               case 'c':
                  *BuildPtr++ = (char)*Parm.LongPtr;
                  Parm.LongPtr++;
                  pStr++;
                  continue;

               case 'l':
                  pStr++;
                  switch (*pStr)
                  {
                  case 'x':
                  case 'X':
                  BuildPtr=HexLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                  pStr++;
                  continue;

                  case 'd':
                     BuildPtr=DecLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                     pStr++;
                     continue;
                  } // end switch
                  continue;                        // dunno what he wants

               case 0:
                  continue;
               } // end switch
            break;

      case '\\':
         pStr++;
         switch (*pStr)
            {
            case 'n':
            *BuildPtr++=LF;
            pStr++;
            continue;

            case 'r':
            *BuildPtr++=CR;
            pStr++;
            continue;

            case 0:
            continue;
            break;
            } // end switch

         break;
         } // end switch

      *BuildPtr++=*pStr++;
      } // end while

   *BuildPtr=0;                                 // cauterize the string
   StringOut((char *) BuildString);
#endif                            //DEBUG
}

struct snd_info_buffer {
	char *buffer;		/* pointer to begin of buffer */
	char *curr;		/* current position in buffer */
	unsigned long size;	/* current size */
	unsigned long len;	/* total length of buffer */
	int stop;		/* stop flag */
	int error;		/* error code */
};

typedef struct snd_info_buffer snd_info_buffer_t;

int snd_iprintf(snd_info_buffer_t * buffer, char *fmt,...)
{
   char *BuildPtr=buffer->curr;
   char *pStr=(char *) fmt;
   char *SubStr;
   int   res;
   union {
         void   *VoidPtr;
         USHORT *WordPtr;
         ULONG  *LongPtr;
         ULONG  *StringPtr;
         } Parm;
   USHORT wBuildOption;

   Parm.VoidPtr=(void *) &fmt;
   Parm.StringPtr++;                            // skip size of string pointer

   if (buffer->stop || buffer->error)
       return 0;

   while (*pStr)
      {
      // don't overflow target
      if (BuildPtr >= (char *) &buffer->curr[buffer->len - 4])
         break;

      switch (*pStr)
         {
         case '%':
            wBuildOption=0;
            pStr++;
            if (*pStr=='0')
               {
               wBuildOption|=LEADING_ZEROES;
               pStr++;
               }
//            if (*pStr=='u')                                                         // always unsigned
//               pStr++;
            if (*pStr=='#')
               pStr++;

            switch(*pStr)
               {
               case 'x':
	       case 'X':
	       case 'p':
               case 'P':
                  BuildPtr=HexLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                  pStr++;
                  continue;

               case 'd':
               case 'u':
                  BuildPtr=DecLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                  pStr++;
                  continue;

               case 's':
                  SubStr=(char *)*Parm.StringPtr;
                  while (*BuildPtr++ = *SubStr++);
                  Parm.StringPtr++;
                  BuildPtr--;                      // remove the \0
                  pStr++;
                  continue;

               case 'c':
                  *BuildPtr++ = (char)*Parm.LongPtr;
                  Parm.LongPtr++;
                  pStr++;
                  continue;

               case 'l':
                  pStr++;
                  switch (*pStr)
                  {
                  case 'x':
                  case 'X':
                  BuildPtr=HexLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                  pStr++;
                  continue;

                  case 'd':
                     BuildPtr=DecLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                     pStr++;
                     continue;
                  } // end switch
                  continue;                        // dunno what he wants

               case 0:
                  continue;
               } // end switch
            break;

      case '\\':
         pStr++;
         switch (*pStr)
            {
            case 'n':
            *BuildPtr++=LF;
            pStr++;
            continue;

            case 'r':
            *BuildPtr++=CR;
            pStr++;
            continue;

            case 0:
            continue;
            break;
            } // end switch

         break;
         } // end switch

      *BuildPtr++=*pStr++;
      } // end while

   *BuildPtr=0;                                 // cauterize the string

   res = strlen(buffer->curr);
   if (buffer->size + res >= buffer->len) {
       buffer->stop = 1;
       return 0;
   }
   buffer->curr += res;
   buffer->size += res;
   return res;
}

