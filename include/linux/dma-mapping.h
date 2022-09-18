#ifndef _ASM_LINUX_DMA_MAPPING_H
#define _ASM_LINUX_DMA_MAPPING_H
#include <linux/pci.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/err.h>

/*
 * A dma_addr_t can hold any valid DMA or bus address for the platform.
 * It can be given to a device to use as a DMA source or target.  A CPU cannot
 * reference a dma_addr_t directly because there may be translation between
 * its physical address space and the bus address space.
 */
struct dma_map_ops {
#if 0
	void* (*alloc)(struct device *dev, size_t size,
				dma_addr_t *dma_handle, gfp_t gfp,
				struct dma_attrs *attrs);
	void (*free)(struct device *dev, size_t size,
			      void *vaddr, dma_addr_t dma_handle,
			      struct dma_attrs *attrs);
	int (*mmap)(struct device *, struct vm_area_struct *,
			  void *, dma_addr_t, size_t, struct dma_attrs *attrs);

	int (*get_sgtable)(struct device *dev, struct sg_table *sgt, void *,
			   dma_addr_t, size_t, struct dma_attrs *attrs);

	dma_addr_t (*map_page)(struct device *dev, struct page *page,
			       unsigned long offset, size_t size,
			       enum dma_data_direction dir,
			       struct dma_attrs *attrs);
	void (*unmap_page)(struct device *dev, dma_addr_t dma_handle,
			   size_t size, enum dma_data_direction dir,
			   struct dma_attrs *attrs);
	/*
	 * map_sg returns 0 on error and a value > 0 on success.
	 * It should never return a value < 0.
	 */
	int (*map_sg)(struct device *dev, struct scatterlist *sg,
		      int nents, enum dma_data_direction dir,
		      struct dma_attrs *attrs);
	void (*unmap_sg)(struct device *dev,
			 struct scatterlist *sg, int nents,
			 enum dma_data_direction dir,
			 struct dma_attrs *attrs);
	void (*sync_single_for_cpu)(struct device *dev,
				    dma_addr_t dma_handle, size_t size,
				    enum dma_data_direction dir);
	void (*sync_single_for_device)(struct device *dev,
				       dma_addr_t dma_handle, size_t size,
				       enum dma_data_direction dir);
	void (*sync_sg_for_cpu)(struct device *dev,
				struct scatterlist *sg, int nents,
				enum dma_data_direction dir);
	void (*sync_sg_for_device)(struct device *dev,
				   struct scatterlist *sg, int nents,
				   enum dma_data_direction dir);
	int (*mapping_error)(struct device *dev, dma_addr_t dma_addr);
#endif
	int (*dma_supported)(struct device *dev, u64 mask);
	int (*set_dma_mask)(struct device *dev, u64 mask);
#ifdef ARCH_HAS_DMA_GET_REQUIRED_MASK
	u64 (*get_required_mask)(struct device *dev);
#endif
	int is_phys;
};


#define DMA_BIT_MASK(n)	(((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))

#define DMA_MASK_NONE	0x0ULL

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

//extern struct dma_map_ops *dma_ops;

static inline struct dma_map_ops *get_dma_ops(struct device *dev)
{
	if (dev->dma_ops)
		return dev->dma_ops;
	return NULL;
}

static inline void set_dma_ops(struct device *dev,
			       struct dma_map_ops *dma_ops)
{
	dev->dma_ops = dma_ops;
}

int dma_supported(struct device *dev, u64 mask);
int dma_set_mask(struct device *dev, u64 mask);
int dma_set_coherent_mask(struct device *dev, u64 mask);

/*
 * Set both the DMA mask and the coherent DMA mask to the same thing.
 * Note that we don't check the return value from dma_set_coherent_mask()
 * as the DMA API guarantees that the coherent DMA mask can be set to
 * the same or smaller than the streaming DMA mask.
 */
static inline int dma_set_mask_and_coherent(struct device *dev, u64 mask)
{
	int rc = dma_set_mask(dev, mask);
	if (rc == 0)
		dma_set_coherent_mask(dev, mask);
	return rc;
}

#if 1
#define dma_alloc_coherent(dev,size,addr,flags) pci_alloc_consistent((struct pci_dev *)(dev),size,addr)
#else
extern void *dma_alloc_coherent(struct device *dev, size_t size, dma_addr_t *handle, gfp_t gfp);
#endif
#define dma_free_coherent(dev,size,ptr,addr) pci_free_consistent((struct pci_dev *)(dev),size,ptr,addr)
static inline bool dma_can_mmap(struct device *dev)
{
	return false;
}
#define pci_set_consistent_dma_mask(p,x) pci_set_dma_mask(p,x)
#endif
