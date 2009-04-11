#define INCL_NOPMAPI
#include <os2.h>
#include <devhelp.h>
#include <devrp.h>
#include <dbgos2.h>
#include <string.h> // memcpy

extern "C" {

int   rdOffset= 0;
int   wrOffset= 0;
char  *szprintBuf= 0;
#ifdef DEBUG
int   max_buf_size= 0x100000;
#else
int   max_buf_size= 0x10000;
#endif

void * __ioremap(unsigned long physaddr, unsigned long size, unsigned long flags);
void iounmap(void *addr);

}

ULONG StratRead(RP __far* _rp)
{
  RPRWV __far* rp = (RPRWV __far*)_rp;
  
  char *lin;
  int   transferCount= rp->Count;
  
  if( szprintBuf )
  {
    lin= (char *)__ioremap( rp->Transfer, transferCount, 0 );
    if( lin )
    {
      int diffCount;
      
      if( rdOffset > wrOffset )
      {
        diffCount= max_buf_size - rdOffset + wrOffset;
      } else
      {
        diffCount= wrOffset - rdOffset;
      }
      if( transferCount > diffCount )
        transferCount= diffCount;
      
      rp->Count= transferCount;
      if( (rdOffset + transferCount) > max_buf_size )
      {
        diffCount= max_buf_size - rdOffset;
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
    rp->Count= 0;
    return RPDONE;
  }
}
