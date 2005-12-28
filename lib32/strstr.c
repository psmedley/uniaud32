/* strstr.c (emx+gcc) -- Copyright (c) 1990-1995 by Eberhard Mattes */

#include <string.h>
#if 1
char *strstr1 (const char *string1, const char *string2)
{
  int len1, len2, i;
  char first;

  if (*string2 == 0)
    return (char *)string1;
  len1 = 0;
  while (string1[len1] != 0) ++len1;
  len2 = 0;
  while (string2[len2] != 0) ++len2;
  if (len2 == 0)
    return (char *)(string1+len1);
  first = *string2;
  while (len1 >= len2)
    {
      if (*string1 == first)
        {
          for (i = 1; i < len2; ++i)
            if (string1[i] != string2[i])
              break;
          if (i >= len2)
            return (char *)string1;
        }
      ++string1; --len1;
    }
  return NULL;
}
#endif
#if 0
size_t strlcpy(char *dst, const char *src, size_t size)
{
        if(size) {
                strncpy(dst, src, size-1);
                dst[size-1] = '\0';
        } else {
                dst[0] = '\0';
        }
        return strlen(src);
}
#endif
