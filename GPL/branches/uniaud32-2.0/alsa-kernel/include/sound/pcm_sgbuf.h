#ifndef __SOUND_PCM_SGBUF_H
#define __SOUND_PCM_SGBUF_H

/*
 * Scatter-Gather PCM access
 *
 *  Copyright (c) by Takashi Iwai <tiwai@suse.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
/*
 * buffer device info
 */
struct snd_dma_device {
	int type;			/* SNDRV_MEM_TYPE_XXX */
	union {
		struct pci_dev *pci;	/* for PCI and PCI-SG types */
		unsigned int flags;	/* GFP_XXX for continous and ISA types */
#ifdef CONFIG_SBUS
		struct sbus_dev *sbus;	/* for SBUS type */
#endif
	} dev;
	unsigned int id;		/* a unique ID */
};

/*
 * buffer types
 */
#define SNDRV_DMA_TYPE_UNKNOWN		0	/* not defined */
#define SNDRV_DMA_TYPE_CONTINUOUS	1	/* continuous no-DMA memory */
#define SNDRV_DMA_TYPE_ISA		2	/* ISA continuous */
#define SNDRV_DMA_TYPE_PCI		3	/* PCI continuous */
#define SNDRV_DMA_TYPE_SBUS		4	/* SBUS continuous */
#define SNDRV_DMA_TYPE_PCI_SG		5	/* PCI SG-buffer */

#ifdef CONFIG_PCI
/*
 * compose a snd_dma_device struct for the PCI device
 */
static inline void snd_dma_device_pci(struct snd_dma_device *dev, struct pci_dev *pci, unsigned int id)
{
	memset(dev, 0, sizeof(*dev));
	dev->type = SNDRV_DMA_TYPE_PCI;
	dev->dev.pci = pci;
	dev->id = id;
}
#endif


/*
 * info for buffer allocation
 */
struct snd_dma_buffer {
	unsigned char *area;	/* virtual pointer */
	dma_addr_t addr;	/* physical address */
	size_t bytes;		/* buffer size in bytes */
	void *private_data;	/* private for allocator; don't touch */
};

struct snd_sg_page {
	void *buf;
	dma_addr_t addr;
};

struct snd_sg_buf {
	int size;	/* allocated byte size (= runtime->dma_bytes) */
	int pages;	/* allocated pages */
	int tblsize;	/* allocated table size */
	struct snd_sg_page *table;
	struct pci_dev *pci;
};

typedef struct snd_sg_buf snd_pcm_sgbuf_t; /* for magic cast */

/*
 * return the pages matching with the given byte size
 */
static inline unsigned int snd_pcm_sgbuf_pages(size_t size)
{
	return (size + PAGE_SIZE - 1) >> PAGE_SHIFT;
}

/*
 * return the physical address at the corresponding offset
 */
static inline dma_addr_t snd_pcm_sgbuf_get_addr(struct snd_sg_buf *sgbuf, size_t offset)
{
	return sgbuf->table[offset >> PAGE_SHIFT].addr + offset % PAGE_SIZE;
}

int snd_pcm_sgbuf_init(snd_pcm_substream_t *substream, struct pci_dev *pci, int tblsize);
int snd_pcm_sgbuf_delete(snd_pcm_substream_t *substream);
int snd_pcm_sgbuf_alloc(snd_pcm_substream_t *substream, size_t size);
int snd_pcm_sgbuf_free(snd_pcm_substream_t *substream);

int snd_pcm_sgbuf_ops_copy_playback(snd_pcm_substream_t *substream, int channel, snd_pcm_uframes_t hwoff, void *buf, snd_pcm_uframes_t count);
int snd_pcm_sgbuf_ops_copy_capture(snd_pcm_substream_t *substream, int channel, snd_pcm_uframes_t hwoff, void *buf, snd_pcm_uframes_t count);
int snd_pcm_sgbuf_ops_silence(snd_pcm_substream_t *substream, int channel, snd_pcm_uframes_t hwoff, snd_pcm_uframes_t count);
void *snd_pcm_sgbuf_ops_page(snd_pcm_substream_t *substream, unsigned long offset);


#endif /* __SOUND_PCM_SGBUF_H */
