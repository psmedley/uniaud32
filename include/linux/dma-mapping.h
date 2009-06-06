#ifndef _ASM_LINUX_DMA_MAPPING_H
#define _ASM_LINUX_DMA_MAPPING_H

/* These definitions mirror those in pci.h, so they can be used
 * interchangeably with their PCI_ counterparts */
enum dma_data_direction {
	DMA_BIDIRECTIONAL = 0,
	DMA_TO_DEVICE = 1,
	DMA_FROM_DEVICE = 2,
	DMA_NONE = 3,
};

#define DMA_64BIT_MASK	0xffffffffffffffffULL
//#define DMA_32BIT_MASK	0x00000000ffffffffULL

#ifndef DMA_32BIT_MASK
#define DMA_32BIT_MASK 0xffffffff
#endif
#ifndef DMA_31BIT_MASK
#define DMA_31BIT_MASK	0x000000007fffffffULL
#endif
#ifndef DMA_30BIT_MASK
#define DMA_30BIT_MASK	0x000000003fffffffULL
#endif
#ifndef DMA_28BIT_MASK
#define DMA_28BIT_MASK	0x000000000fffffffULL
#endif
#ifndef DMA_24BIT_MASK
#define DMA_24BIT_MASK	0x0000000000ffffffULL
#endif
#define DMA_BIT_MASK(n)	(((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))
#endif


