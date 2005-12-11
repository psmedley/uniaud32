/*
 *  FM instrument loader
 *
 *  (C) 2002 InnoTek Systemberatung GmbH
 *  Copyright (c) 2000 Uros Bizjak <uros@kss-loka.si>
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

#include <sound/driver.h>
#include <sound/control.h>
#include <sound/info.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/minors.h>
#include <sound/asequencer.h>
#include <sound/asound_fm.h>
#include <sound/ainstr_fm.h>
#include <sound/seq_kernel.h>
#include <sound/instr.h>
#include <linux/file.h>
#include <linux/soundcard.h>

#define LINUX
#include <ossidc32.h>
#include <stacktoflat.h>
#include <stdlib.h>
#include "soundoss.h"
#include "instrfm.h"

#define assert(a)   

typedef struct sbi_header
{
  char key[4];
  char name[32];
}
sbi_header_t;

typedef struct sbi_inst
{
  sbi_header_t header;
#define DATA_LEN_2OP	16
#define DATA_LEN_4OP	24
  char data[DATA_LEN_4OP];
}
sbi_inst_t;

/* offsets for SBI params */
#define AM_VIB		0
#define KSL_LEVEL	2
#define ATTACK_DECAY	4
#define SUSTAIN_RELEASE	6
#define WAVE_SELECT	8

/* offset for SBI instrument */
#define CONNECTION	10
#define OFFSET_4OP	11

/* offsets in sbi_header.name for SBI extensions */
#define ECHO_DELAY	25
#define ECHO_ATTEN	26
#define CHORUS_SPREAD	27
#define TRNSPS		28
#define FIX_DUR		29
#define MODES		30
#define FIX_KEY		31

/* Number of elements in an array */
#define NELEM(a) ( sizeof(a)/sizeof((a)[0]) )

#define ADDR_PARTS 4		/* Number of part in a port description addr 1:2:3:4 */
#define SEP ", \t"		/* Separators for port description */

#define SBI_FILE_TYPE_2OP 0
#define SBI_FILE_TYPE_4OP 1

static BOOL fInstumentsLoaded = FALSE;
/* Default file type */
static int file_type = SBI_FILE_TYPE_2OP;


static void load_sb (midihandle *pHandle, char *pFile, int filesize, int bank);

//******************************************************************************
//******************************************************************************
OSSRET OSS32_FMMidiLoadInstruments(OSSSTREAMID streamid)
{
    midihandle  *pHandle = (midihandle *)streamid;
    
    if(pHandle == NULL || pHandle->magic != MAGIC_MIDI_ALSA32) {
        DebugInt3();
        return OSSERR_INVALID_STREAMID;
    }
    if(fInstumentsLoaded) return OSSERR_SUCCESS;
    fInstumentsLoaded = TRUE;

    file_type = SBI_FILE_TYPE_4OP;

    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    //load instruments
    load_sb(pHandle, stdopl3, sizeof(stdopl3), 0);

    //load drums
    load_sb(pHandle, drumsopl3, sizeof(drumsopl3), 128);
   
    return OSSERR_SUCCESS;
}
//******************************************************************************
//******************************************************************************

#ifdef DEBUG
/*
 * Show instrument FM operators
 */
static void
show_op (fm_instrument_t * fm_instr) {
  int i = 0;

  do {
      dprintf( ("  OP%d: flags: %s %s %s %s\011OP%d: flags: %s %s %s %s\n",
	      i,
	      fm_instr->op[i].am_vib & (1 << 7) ? "AM" : "  ",
	      fm_instr->op[i].am_vib & (1 << 6) ? "VIB" : "   ",
	      fm_instr->op[i].am_vib & (1 << 5) ? "EGT" : "   ",
	      fm_instr->op[i].am_vib & (1 << 4) ? "KSR" : "   ",
	      i + 1,
	      fm_instr->op[i + 1].am_vib & (1 << 7) ? "AM" : "  ",
	      fm_instr->op[i + 1].am_vib & (1 << 6) ? "VIB" : "   ",
	      fm_instr->op[i + 1].am_vib & (1 << 5) ? "EGT" : "   ",
	      fm_instr->op[i + 1].am_vib & (1 << 4) ? "KSR" : ""));
      dprintf(("  OP%d: MULT = 0x%x" "\011\011OP%d: MULT = 0x%x\n",
	      i, fm_instr->op[i].am_vib & 0x0f,
	      i + 1, fm_instr->op[i + 1].am_vib & 0x0f));
      dprintf(("  OP%d: KSL  = 0x%x  TL = 0x%x\011OP%d: KSL  = 0x%x  TL = 0x%x\n",
	      i, (fm_instr->op[i].ksl_level >> 6) & 0x03, fm_instr->op[i].ksl_level & 0x3f,
	      i + 1, (fm_instr->op[i + 1].ksl_level >> 6) & 0x03, fm_instr->op[i + 1].ksl_level & 0x3f));
      dprintf(("  OP%d: AR   = 0x%x  DL = 0x%x\011OP%d: AR   = 0x%x  DL = 0x%x\n",
	      i, (fm_instr->op[i].attack_decay >> 4) & 0x0f, fm_instr->op[i].attack_decay & 0x0f,
	      i + 1, (fm_instr->op[i + 1].attack_decay >> 4) & 0x0f, fm_instr->op[i + 1].attack_decay & 0x0f));
      dprintf(("  OP%d: SL   = 0x%x  RR = 0x%x\011OP%d: SL   = 0x%x  RR = 0x%x\n",
	      i, (fm_instr->op[i].sustain_release >> 4) & 0x0f, fm_instr->op[i].sustain_release & 0x0f,
	      i + 1, (fm_instr->op[i + 1].sustain_release >> 4) & 0x0f, fm_instr->op[i + 1].sustain_release & 0x0f));
      dprintf(("  OP%d: WS   = 0x%x\011\011OP%d: WS   = 0x%x\n",
	      i, fm_instr->op[i].wave_select & 0x07,
	      i + 1, fm_instr->op[i + 1].wave_select & 0x07));
      dprintf((" FB = 0x%x,  %s\n",
	      (fm_instr->feedback_connection[i / 2] >> 1) & 0x07,
	      fm_instr->feedback_connection[i / 2] & (1 << 0) ? "parallel" : "serial"));
      i += 2;
  }
  while (i == (fm_instr->type == FM_PATCH_OPL3) << 1);

  dprintf((" Extended data:\n"
	  "  ED = %d  EA = %d  CS = %d  TR = %d\n"
	  "  FD = %d  MO = %d  FK = %d\n",
	  fm_instr->echo_delay, fm_instr->echo_atten, fm_instr->chorus_spread, fm_instr->trnsps,
	  fm_instr->fix_dur, fm_instr->modes, fm_instr->fix_key));
}
#endif

/*
 * Send patch to destination port
 */
static int load_patch (midihandle *pHandle, struct fm_instrument * fm_instr, int bank, int prg, char *name)
{
    snd_seq_instr_header_t *put;
    snd_seq_instr_t id;
    snd_seq_event_t ev;

    size_t size;
    int err;

#ifdef DEBUG
//    show_op (fm_instr);
#endif
    if ((err = snd_instr_fm_convert_to_stream (fm_instr, name, &put, &size)) < 0) {
        dprintf(("Unable to convert instrument %.3i to stream", prg));
        return -1;
    }
    memset(&id, 0, sizeof(id));
    id.std = SND_SEQ_INSTR_TYPE2_OPL2_3;
    id.prg = prg;
    id.bank = bank;
    snd_instr_header_set_id(put, &id);

    /* build event */
    memset (&ev, 0, sizeof (ev));
    ev.source.client = pHandle->clientid;
    ev.source.port   = pHandle->clientport;
    ev.dest.client   = pHandle->destclient;
    ev.dest.port     = pHandle->destport;

    ev.flags = SNDRV_SEQ_EVENT_LENGTH_VARUSR;
    ev.queue = SNDRV_SEQ_QUEUE_DIRECT;

    ev.type = SNDRV_SEQ_EVENT_INSTR_PUT;
    ev.data.ext.len = size;
    ev.data.ext.ptr = put;

    err = pHandle->file.f_op->write(&pHandle->file, (char *)__Stack32ToFlat(&ev), sizeof(ev), &pHandle->file.f_pos);
    if(err < 0) {
        dprintf(("Unable to write an instrument %.3i put event: %x",prg, err));
        return -1;
    }
    return 0;
}

/*
 * Parse standard .sb or .o3 file
 */
static void load_sb (midihandle *pHandle, char *pFile, int filesize, int bank) 
{
    int len, i, offset = 0;
    int prg;

    sbi_inst_t sbi_instr;
    struct fm_instrument fm_instr;
    int fm_instr_type;

    len = (file_type == SBI_FILE_TYPE_4OP) ? DATA_LEN_4OP : DATA_LEN_2OP;
    for (prg = 0;; prg++) {
        if(offset + sizeof (sbi_header_t) > filesize) {
            break; //EOF
        }
        memcpy(&sbi_instr.header, pFile+offset, sizeof (sbi_header_t));
        offset += sizeof (sbi_header_t);

        if (!strncmp (sbi_instr.header.key, "SBI\032", 4) || !strncmp (sbi_instr.header.key, "2OP\032", 4)) {
            fm_instr_type = FM_PATCH_OPL2;
        } 
        else 
        if (!strncmp (sbi_instr.header.key, "4OP\032", 4)) {
            fm_instr_type = FM_PATCH_OPL3;
        } 
        else {
            fm_instr_type = 0;
        }
        if(offset + len > filesize) {
            break; //EOF
        }
        memcpy(&sbi_instr.data, pFile+offset, len);
        offset += len;

        if (fm_instr_type == 0)
            continue;

        memset (&fm_instr, 0, sizeof (fm_instr));
        fm_instr.type = fm_instr_type;

        for (i = 0; i < 2; i++) {
            fm_instr.op[i].am_vib = sbi_instr.data[AM_VIB + i];
            fm_instr.op[i].ksl_level = sbi_instr.data[KSL_LEVEL + i];
            fm_instr.op[i].attack_decay = sbi_instr.data[ATTACK_DECAY + i];
            fm_instr.op[i].sustain_release = sbi_instr.data[SUSTAIN_RELEASE + i];
            fm_instr.op[i].wave_select = sbi_instr.data[WAVE_SELECT + i];
        }
        fm_instr.feedback_connection[0] = sbi_instr.data[CONNECTION];

        if (fm_instr_type == FM_PATCH_OPL3) {
            for (i = 0; i < 2; i++) {
                fm_instr.op[i + 2].am_vib = sbi_instr.data[OFFSET_4OP + AM_VIB + i];
                fm_instr.op[i + 2].ksl_level = sbi_instr.data[OFFSET_4OP + KSL_LEVEL + i];
                fm_instr.op[i + 2].attack_decay = sbi_instr.data[OFFSET_4OP + ATTACK_DECAY + i];
                fm_instr.op[i + 2].sustain_release = sbi_instr.data[OFFSET_4OP + SUSTAIN_RELEASE + i];
                fm_instr.op[i + 2].wave_select = sbi_instr.data[OFFSET_4OP + WAVE_SELECT + i];
            }
            fm_instr.feedback_connection[1] = sbi_instr.data[OFFSET_4OP + CONNECTION];
        }

        fm_instr.echo_delay = sbi_instr.header.name[ECHO_DELAY];
        fm_instr.echo_atten = sbi_instr.header.name[ECHO_ATTEN];
        fm_instr.chorus_spread = sbi_instr.header.name[CHORUS_SPREAD];
        fm_instr.trnsps = sbi_instr.header.name[TRNSPS];
        fm_instr.fix_dur = sbi_instr.header.name[FIX_DUR];
        fm_instr.modes = sbi_instr.header.name[MODES];
        fm_instr.fix_key = sbi_instr.header.name[FIX_KEY];

        if (load_patch (pHandle, &fm_instr, bank, prg, sbi_instr.header.name) < 0)
            break;
    }
    return;
}
/**
 * \brief Convert the FM instrument to byte stream
 * \param fm FM instrument handle
 * \param name FM instrument name
 * \param __data Result - allocated byte stream
 * \param __size Result - size of allocated byte stream
 * \return 0 on success otherwise a negative error code
 */
int snd_instr_fm_convert_to_stream(snd_instr_fm_t *fm,
				   const char *name,
				   snd_instr_header_t **__data,
				   size_t *__size)
{
	snd_instr_header_t *put;
	struct fm_instrument *instr;
	fm_xinstrument_t *xinstr;
	int idx;

	if (fm == NULL || __data == NULL)
		return -EINVAL;
	instr = (struct fm_instrument *)fm;
	*__data = NULL;
	*__size = 0;
	if (snd_instr_header_malloc(&put, sizeof(fm_xinstrument_t)) < 0)
		return -ENOMEM;
	/* build header */
	if (name)
		snd_instr_header_set_name(put, name);
	snd_instr_header_set_type(put, SND_SEQ_INSTR_ATYPE_DATA);
	snd_instr_header_set_format(put, SND_SEQ_INSTR_ID_OPL2_3);
	/* build data section */
	xinstr = (fm_xinstrument_t *)snd_instr_header_get_data(put);
	xinstr->stype = FM_STRU_INSTR;
	xinstr->share_id[0] = __cpu_to_le32(instr->share_id[0]);
	xinstr->share_id[1] = __cpu_to_le32(instr->share_id[1]);
	xinstr->share_id[2] = __cpu_to_le32(instr->share_id[2]);
	xinstr->share_id[3] = __cpu_to_le32(instr->share_id[3]);
	xinstr->type = instr->type;
	for (idx = 0; idx < 4; idx++) {
		xinstr->op[idx].am_vib = instr->op[idx].am_vib;
		xinstr->op[idx].ksl_level = instr->op[idx].ksl_level;
		xinstr->op[idx].attack_decay = instr->op[idx].attack_decay;
		xinstr->op[idx].sustain_release = instr->op[idx].sustain_release;
		xinstr->op[idx].wave_select = instr->op[idx].wave_select;
	}
	for (idx = 0; idx < 2; idx++) {
		xinstr->feedback_connection[idx] = instr->feedback_connection[idx];
	}
	xinstr->echo_delay = instr->echo_delay;
	xinstr->echo_atten = instr->echo_atten;
	xinstr->chorus_spread = instr->chorus_spread;
	xinstr->trnsps = instr->trnsps;
	xinstr->fix_dur = instr->fix_dur;
	xinstr->modes = instr->modes;
	xinstr->fix_key = instr->fix_key;

	/* write result */
	*__data = put;
	*__size = sizeof(*put) + sizeof(fm_xinstrument_t);
	return 0;
}
/**
 * \brief allocate an empty #snd_instr_header_t using standard malloc
 * \param ptr returned pointer
 * \param len additional data length
 * \return 0 on success otherwise negative error code
 */
int snd_instr_header_malloc(snd_instr_header_t **ptr, size_t len)
{
	assert(ptr);
	*ptr = kmalloc(sizeof(snd_instr_header_t) + len, 0);
	if (!*ptr)
		return -ENOMEM;

    memset(*ptr, 0, sizeof(snd_instr_header_t) + len);
	(*ptr)->len = len;
          
	return 0;
}

/**
 * \brief frees a previously allocated #snd_instr_header_t
 * \param pointer to object to free
 */
void snd_instr_header_free(snd_instr_header_t *obj)
{
	kfree(obj);
}
/**
 * \brief Set the data type of an instr_header container
 * \param info instr_header container
 * \param type the data type
 */
void snd_instr_header_set_type(snd_instr_header_t *info, int type)
{
	assert(info);
	info->data.type = type;
}

/**
 * \brief Set the data format of an instr_header container
 * \param info instr_header container
 * \param format the data format string
 */
void snd_instr_header_set_format(snd_instr_header_t *info, const char *format)
{
	assert(info && format);
	strncpy(info->data.data.format, format, sizeof(info->data.data.format));
}

/**
 * \brief Set the instrument id of an instr_header container
 * \param info instr_header container
 * \param id instrument id pointer
 */
void snd_instr_header_set_id(snd_instr_header_t *info, const snd_seq_instr_t *id)
{
	assert(info && id);
	info->id.instr = *(struct snd_seq_instr *)id;
}

/**
 * \brief Set the data name of an instr_header container
 * \param info instr_header container
 * \param name the name string
 */
void snd_instr_header_set_name(snd_instr_header_t *info, const char *name)
{
	assert(info && name);
	strncpy(info->data.name, name, sizeof(info->data.name));
}

/**
 * \brief Get the extra data pointer of an instr_header container
 * \param info instr_header container
 * \return the extra data pointer
 */
void *snd_instr_header_get_data(const snd_instr_header_t *info)
{
	assert(info);
	return (void*)((char*)info + sizeof(*info));
}

//******************************************************************************
//******************************************************************************

