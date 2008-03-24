#ifndef __MIXER_OSS_H
#define __MIXER_OSS_H

/*
 *  OSS MIXER API
 *  Copyright (c) by Jaroslav Kysela <perex@perex.cz>
 *
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
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef CONFIG_SND_OSSEMUL

#define SNDRV_OSS_MAX_MIXERS	32

struct snd_mixer_oss_file;

struct snd_mixer_oss_slot {
	int number;
	unsigned int stereo: 1;
	int (*get_volume)(struct snd_mixer_oss_file *fmixer,
			  struct snd_mixer_oss_slot *chn,
			  int *left, int *right);
	int (*put_volume)(struct snd_mixer_oss_file *fmixer,
			  struct snd_mixer_oss_slot *chn,
			  int left, int right);
	int (*get_recsrc)(struct snd_mixer_oss_file *fmixer,
			  struct snd_mixer_oss_slot *chn,
			  int *active);
	int (*put_recsrc)(struct snd_mixer_oss_file *fmixer,
			  struct snd_mixer_oss_slot *chn,
			  int active);
	unsigned long private_value;
	void *private_data;
	void (*private_free)(struct snd_mixer_oss_slot *slot);
	int volume[2];
};

struct snd_mixer_oss {
	struct snd_card *card;
	char id[16];
	char name[32];
	struct snd_mixer_oss_slot slots[SNDRV_OSS_MAX_MIXERS]; /* OSS mixer slots */
	unsigned int mask_recsrc;		/* exclusive recsrc mask */
	int (*get_recsrc)(struct snd_mixer_oss_file *fmixer,
			  unsigned int *active_index);
	int (*put_recsrc)(struct snd_mixer_oss_file *fmixer,
			  unsigned int active_index);
	void *private_data_recsrc;
	void (*private_free_recsrc)(struct snd_mixer_oss *mixer);
	struct mutex reg_mutex;
	struct snd_info_entry *proc_entry;
	int oss_dev_alloc;
	/* --- */
	int oss_recsrc;
};

struct snd_mixer_oss_file {
	struct snd_card *card;
	struct snd_mixer_oss *mixer;
};

#endif /* CONFIG_SND_MIXER_OSS */

#endif /* __SOUND_MIXER_OSS_H */
