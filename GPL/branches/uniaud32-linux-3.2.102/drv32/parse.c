/* $Id: parse.c,v 1.3 2003/08/08 15:09:03 vladest Exp $ */

/*
 * Config.sys parameter parsing
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
 */

#ifdef __cplusplus
extern "C" {
#endif
#define INCL_NOPMAPI
#define INCL_DOSMISC
#include <os2.h>
#ifdef __cplusplus
}
#endif

#include <devhelp.h>
#include <devtype.h>
#include <unicard.h>
#include "parse.h"         // NUM_DEVICES
#include <string.h>

#define COMM_DEBUG

// True if the /V parameter was specified
int fVerbose  = FALSE;
int fDebug    = FALSE;
//int ForceCard = CARD_NONE;
int iAdapterNumber = -1;

extern short int midi_port;

#ifdef COMM_DEBUG
extern short int MAGIC_COMM_PORT;
#endif

//*****************************************************************************
//*****************************************************************************
int toupper(int c)
{
   return (unsigned) c - 'a' <= 'z' - 'a' ? c - ('a' - 'A') : c;
}
//*****************************************************************************
//*****************************************************************************
CHAR FAR48 *mymemchr(CHAR FAR48 *strP, CHAR c, USHORT size)
{
    USHORT i;
    // search for the character - return position if found
    i = 0;
    while (i <= size - 1) {
        if (*strP == c)
            return (strP);
        strP++;
        i++;
    }
    // character not found - return null
    return ((CHAR FAR48 *) 0);
}
//*****************************************************************************
//*****************************************************************************

USHORT sz2us(char FAR48 *sz, int base)
{
    static char digits[] = "0123456789ABCDEF";

    USHORT us=0;
    //   char *pc;
    CHAR FAR48 *pc;

    // skip leading spaces
    while (*sz == ' ') sz++;

    // skip leading zeros
    while (*sz == '0') sz++;

    // accumulate digits - return error if unexpected character encountered
    for (;;sz++) {
        pc = (CHAR FAR48 *) mymemchr(digits, toupper(*sz), base);
        if (!pc)
            return us;
        us = (us * base) + (pc - digits);
    }
}
//*****************************************************************************
//*****************************************************************************
int IsWhitespace(char ch)
{
   if ( ch > '9' && ch < 'A')
      return TRUE;
   if ( ch < '0' || ch > 'Z')
      return TRUE;

   return FALSE;
}
//*****************************************************************************
//*****************************************************************************
char FAR48 *SkipWhite(char FAR48 *psz)
{
   while (*psz) {
      if (!IsWhitespace((char) toupper(*psz))) return psz;
      psz++;
   }
   return NULL;
}
//*****************************************************************************

#if 0
//*****************************************************************************
void CheckCardName(char FAR48 *psz)
{
    char name[CARD_MAX_LEN+1];
    int i;

    for (i=0; i<CARD_MAX_LEN; i++) {
        name[i] = toupper(psz[i]);
        if(name[i] == ' ') {
            name[i] = 0;
            break;
        }
    }
    name[CARD_MAX_LEN] = 0;

    if(!strcmp(name, CARD_STRING_SBLIVE)) {
        ForceCard = CARD_SBLIVE;
    }
    else
    if(!strcmp(name, CARD_STRING_ALS4000)) {
        ForceCard = CARD_ALS4000;
    }
    else
    if(!strcmp(name, CARD_STRING_CMEDIA)) {
        ForceCard = CARD_CMEDIA;
    }
    else
    if(!strcmp(name, CARD_STRING_CS4281)) {
        ForceCard = CARD_CS4281;
    }
    else
    if(!strcmp(name, CARD_STRING_ICH)) {
        ForceCard = CARD_ICH;
    }
    else
    if(!strcmp(name, CARD_STRING_CS46XX)) {
        ForceCard = CARD_CS46XX;
    }
    else
    if(!strcmp(name, CARD_STRING_VIA82XX)) {
        ForceCard = CARD_VIA82XX;
    }
    else
    if(!strcmp(name, CARD_STRING_ESS1938)) {
        ForceCard = CARD_ESS1938;
    }
    else
    if(!strcmp(name, CARD_STRING_VORTEX)) {
        ForceCard = CARD_VORTEX;
    }
    else
    if(!strcmp(name, CARD_STRING_ENSONIQ)) {
        ForceCard = CARD_ENSONIQ;
    }
    else
    if(!strcmp(name, CARD_STRING_TRIDENT)) {
        ForceCard = CARD_TRIDENT;
    }
    else
    if(!strcmp(name, CARD_STRING_NEOMAGIC)) {
        ForceCard = CARD_NEOMAGIC;
    }
    else
    if(!strcmp(name, CARD_STRING_FM801)) {
        ForceCard = CARD_FM801;
    }
    else
    if(!strcmp(name, CARD_STRING_ATIIXP)) {
        ForceCard = CARD_ATIIXP;
    }
    else
    if(!strcmp(name, CARD_STRING_AUDIGYLS)) {
        ForceCard = CARD_AUDIGYLS;
    }
    else
    if(!strcmp(name, CARD_STRING_BT87X)) {
        ForceCard = CARD_BT87X;
    }
    else
    if(!strcmp(name, CARD_STRING_AZX)) {
        ForceCard = CARD_AZX;
    }

}
//*****************************************************************************
#endif

//*****************************************************************************
int DoParm(char cParm, char FAR48 *pszOption)
{
    switch (cParm) {
    case 'A':
        iAdapterNumber = *pszOption & 0x07;
        break;
    case 'V':                     // Verbose option
        fVerbose = TRUE;
        break;
    case 'D':
        fDebug = TRUE;
        break;
    //case 'C':
        //CheckCardName(pszOption);
        //break;
#if 1
    case 'M':
        midi_port = 0x300;
            //sz2us(pszOption, 16);
        break;
#endif
#ifdef COMM_DEBUG
    case 'P':
        if(*pszOption == '0') {
            MAGIC_COMM_PORT = 0x000;
        }
        else
        if(*pszOption == '1') {
            MAGIC_COMM_PORT = 0x3f8;
        }
        else
        if(*pszOption == '2') {
            MAGIC_COMM_PORT = 0x2f8;
        }
        if(*pszOption == '4') {
            MAGIC_COMM_PORT = 0x4000;
        }
        break;
#endif

    default:
         return FALSE;              // unknown parameter
    }
    return TRUE;
}

/* Function: ParseParm
   Input: pointer to the letter of the parameter (e.g. the 'P' in 'P1:330').
          length of this parameter, which must be at least 1
   Output: TRUE if the parameter was value
   Purpose: parses the string into three parts: the letter parameter, the port
            number, and the option string.  Calls DoParm with these values.
   Notes:
      the following describes the format of valid parameters.
         1. Parameters consist of a letter, an optional number, and an
            optional 'option'.  The format is x[n][:option], where 'x' is the
            letter, 'n' is the number, and 'option' is the option.
         2. Blanks are delimeters between parameters, therefore there are no
            blanks within a parameter.
         3. The option is preceded by a colon
      This gives us only four possibilities:
         P (length == 1)
         P1 (length == 2)
         P:option (length >= 3)
         P1:option (length >= 4)
*/
int ParseParm(char FAR48 *pszParm, int iLength)
{
   char ch,ch1=(char) toupper(*pszParm);       // get letter

   if (iLength == 1)                // only a letter?
      return DoParm(ch1,NULL);

   ch=pszParm[1];                   // should be either 1-9 or :
   if (ch < '1' || (ch > '9' && ch != ':'))
      return FALSE;

   if (iLength == 3) {
      if (ch != ':')
         return FALSE;
      return DoParm(ch1,pszParm+2);
   }

   if (ch == ':')
      return DoParm(ch1,pszParm+2);

   return DoParm(ch1, pszParm+3);
}
//*****************************************************************************
//*****************************************************************************
int GetParms(char FAR48 *pszCmdLine)
{
   int iLength;

   while (*pszCmdLine != ' ') {              // skip over filename
      if (!*pszCmdLine) return TRUE;         // no params?  then just exit
      pszCmdLine++;
   }

   while (TRUE) {
      pszCmdLine=SkipWhite(pszCmdLine);      // move to next param
      if (!pszCmdLine) return TRUE;          // exit if no more

      for (iLength=0; pszCmdLine[iLength]; iLength++)    // calculate length
         if (pszCmdLine[iLength] == ' ') break;          //  of parameter

      if (!ParseParm(pszCmdLine,iLength))    // found parameter, so parse it
         return FALSE;

      while (*pszCmdLine != ' ') {              // skip over parameter
         if (!*pszCmdLine) return TRUE;         // no params?  then just exit
         pszCmdLine++;
      }
   }
}
//*****************************************************************************
//*****************************************************************************

