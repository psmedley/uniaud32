#define INCL_NOPMAPI
#include <os2.h>
#include <dbgos2.h>
#include <string.h> // memcpy
#include "strategy.h"

int   rdOffset= 0;
int   wrOffset= 0;
char  *szprintBuf= 0;

void * __ioremap(unsigned long physaddr, unsigned long size, unsigned long flags);
void iounmap(void *addr);

ULONG StratRead(REQPACKET __far* rp)
{
  char *lin;
  int   transferCount= rp->io.usCount;

  if( szprintBuf )
  {
    lin= (char *)__ioremap( rp->io.ulAddress, transferCount, 0 );
    if( lin )
    {
      int diffCount;

      if( rdOffset > wrOffset )
      {
        diffCount= DBG_MAX_BUF_SIZE - rdOffset + wrOffset;
      } else
      {
        diffCount= wrOffset - rdOffset;
      }
      if( transferCount > diffCount )
        transferCount= diffCount;

      rp->io.usCount= transferCount;
      if( (rdOffset + transferCount) > DBG_MAX_BUF_SIZE )
      {
        diffCount= DBG_MAX_BUF_SIZE - rdOffset;
        memcpy( lin, szprintBuf + rdOffset, diffCount );
        transferCount= transferCount - diffCount;
        rdOffset= 0;
      } else
        diffCount= 0;

      if( transferCount )
      {
        memcpy( lin + diffCount, szprintBuf + rdOffset, transferCount );
        rdOffset= rdOffset + transferCount;
      }
      iounmap( (void *)lin );
      return RPDONE;
    } else
      return RPDONE | RPERR;
  } else
  {
    rp->io.usCount= 0;
    return RPDONE;
  }
}
