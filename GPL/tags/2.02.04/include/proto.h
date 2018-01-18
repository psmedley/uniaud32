extern void *memdup_user(void __user *src, size_t len);
extern void snd_free_dev_pages(struct device *dev, size_t size, void *ptr, dma_addr_t dma);
extern void *snd_malloc_dev_pages(struct device *dev, size_t size, dma_addr_t *dma);
extern int _snd_pcm_hw_param_set(struct snd_pcm_hw_params *params, snd_pcm_hw_param_t var, unsigned int val, int dir);
extern int in_interrupt(void);
extern int in_atomic(void);
/*
 * Uniaud API support
 */
extern void uniaud_set_interrupted_substream(struct snd_pcm_substream *substream);


