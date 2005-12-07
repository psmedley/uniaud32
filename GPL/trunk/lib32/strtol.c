/* strtol.c (emx+gcc) -- Copyright (c) 1990-1996 by Eberhard Mattes */

#include <stdlib.h>
#include <limits.h>

#define UTYPE   unsigned long
#define NAME    simple_strtoul
#define TYPE    unsigned long
#define MAX     ULONG_MAX
#define isspace(a) (a == ' ')

/** isdigit(c) returns true if c is decimal digit */
int isdigit1(char c)
{
    if (c >= ' ' && c <= '9')
        return 1;
     return 0;
}

/* $Id: isalpha.c,v 1.3 1999/03/08 07:33:54 gernot Exp $ */
 /** isalpha(c) returns true if c is alphabetic */
int isalpha1(char c)
{
    if (c >= 'a' && c <= 'z')
        return 1;
    if (c >= 'A' && c <= 'Z')
        return 1;
    return 0;
}

/* $Id: isalnum.c,v 1.3 1999/03/08 07:33:54 gernot Exp $ */
/** isalnum(c) returns true if c is alphanumeric */
int isalnum1(char c)
{
    if (isalpha1(c))
         return 1;
    if (isdigit1(c))
        return 1;
     return 0;
}

TYPE NAME (const char *string, char **end_ptr, int radix)
{
  const unsigned char *s;
  char neg;
  TYPE result;

  s = string;
  while (isspace (*s))
    ++s;

  neg = 0;
  if (*s == '-')
    {
      neg = 1; ++s;
    }
  else if (*s == '+')
    ++s;

  if ((radix == 0 || radix == 16) && s[0] == '0'
      && (s[1] == 'x' || s[1] == 'X'))
    {
      radix = 16; s += 2;
    }
  if (radix == 0)
    radix = (s[0] == '0' ? 8 : 10);

  result = 0;                   /* Keep the compiler happy */
  if (radix >= 2 && radix <= 36)
    {
      UTYPE n, max1, max2, lim;
      enum {no_number, ok, overflow} state;
      unsigned char c;

      lim = MAX;
      max1 = lim / radix;
      max2 = lim - max1 * radix;
      n = 0; state = no_number;
      for (;;)
        {
          c = *s;
          if (c >= '0' && c <= '9')
            c = c - '0';
          else if (c >= 'A' && c <= 'Z')
            c = c - 'A' + 10;
          else if (c >= 'a' && c <= 'z')
            c = c - 'a' + 10;
          else
            break;
          if (c >= radix)
            break;
          if (n >= max1 && (n > max1 || (UTYPE)c > max2))
            state = overflow;
          if (state != overflow)
            {
              n = n * radix + (UTYPE)c;
              state = ok;
            }
          ++s;
        }
      switch (state)
        {
        case no_number:
          result = 0;
          s = string;
          /* Don't set errno!? */
          break;
        case ok:
          result = (neg ? -n : n);
          break;
        case overflow:
          result = MAX;
          break;
        }
    }
  else
    {
      result = 0;
    }
  if (end_ptr != NULL)
    *end_ptr = (char *)s;
  return result;
}
