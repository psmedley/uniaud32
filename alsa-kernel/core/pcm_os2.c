#ifdef TARGET_OS2
static int snd_pcm_setvolume(snd_pcm_substream_t * substream, snd_pcm_volume_t * _volume)
{
	snd_pcm_volume_t volume;
	snd_pcm_runtime_t *runtime;
	int res;
	
	snd_assert(substream != NULL, return -ENXIO);
	if (copy_from_user(&volume, _volume, sizeof(volume)))
		return -EFAULT;
	runtime = substream->runtime;
	if (runtime->status->state == SNDRV_PCM_STATE_OPEN)
		return -EBADFD;
	res = substream->ops->ioctl(substream, SNDRV_PCM_IOCTL1_SETVOLUME, &volume);
	if (res < 0)
		return res;
	return 0;
}

static int snd_pcm_getvolume(snd_pcm_substream_t * substream, snd_pcm_volume_t * _volume)
{
	snd_pcm_volume_t volume;
	snd_pcm_runtime_t *runtime;
	int res;
	
	snd_assert(substream != NULL, return -ENXIO);
    memset(&volume, 0, sizeof(volume));
	
    runtime = substream->runtime;
	if (runtime->status->state == SNDRV_PCM_STATE_OPEN)
		return -EBADFD;
	res = substream->ops->ioctl(substream, SNDRV_PCM_IOCTL1_GETVOLUME, &volume);
	if (res < 0)
		return res;
	if (copy_to_user(_volume, &volume, sizeof(volume)))
		return -EFAULT;
	return 0;
}
#endif
