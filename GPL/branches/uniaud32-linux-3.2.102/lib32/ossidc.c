/* $Id: ossidc.cpp,v 1.4 2003/08/08 15:09:03 vladest Exp $ */
/*
 * OS/2 IDC services (callback to 16 bits MMPM2 driver)
 *
 * (C) 2000-2002 InnoTek Systemberatung GmbH
 * (C) 2000-2001 Sander van Leeuwen (sandervl@xs4all.nl)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
 */

#define INCL_NOPMAPI
#define INCL_DOSERRORS			 // for ERROR_INVALID_FUNCTION
#include <os2.h>
#include <ossdefos2.h>
#include <ossidc32.h>
#include <dbgos2.h>
#include <devhelp.h>
#include <unicard.h>
#include <version.h>
#ifdef KEE
#include <kee.h>
#endif
#include "initcall.h"

extern DWORD TimerHandler16; 	//drv32\strategy.asm
extern int OSS32_ProcessMIDIIRQ(int bytes);
extern int OSS32_CloseUNI16(void);

//******************************************************************************
//******************************************************************************
BOOL CallOSS16(ULONG cmd, ULONG param1, ULONG param2)
{
	BOOL		 rc;

	if(idc16_PddHandler == 0) {
		return FALSE;
	}

	rc = CallPDD16(idc16_PddHandler, cmd, param1, param2);
	return rc;
}
//******************************************************************************
exitcall_t fnCardExitCall[OSS32_MAX_AUDIOCARDS] = {0};

/* this table should be discarded after init time */

typedef struct cardcalls_t {
	int card_id;
	initcall_t *cinitcall;
	exitcall_t *cexitcall;
}cardcalls_t;

cardcalls_t cardcalls[CARDS_NUM] = {
	{ CARD_ICH,		 &name_module_init(alsa_card_intel8x0_init),	&name_module_exit(alsa_card_intel8x0_exit)	  },
	{ CARD_VIA82XX,	 &name_module_init(alsa_card_via82xx_init),	 	&name_module_exit(alsa_card_via82xx_exit)	  },
	{ CARD_SBLIVE,	 &name_module_init(alsa_card_emu10k1_init),		&name_module_exit(alsa_card_emu10k1_exit)	  },
	{ CARD_CMEDIA,	 &name_module_init(alsa_card_cmipci_init),		&name_module_exit(alsa_card_cmipci_exit)	  },
	{ CARD_ALS4000,	 &name_module_init(alsa_card_als4000_init),		&name_module_exit(alsa_card_als4000_exit)	  },
	{ CARD_CS4281,	 &name_module_init(alsa_card_cs4281_init),		&name_module_exit(alsa_card_cs4281_exit)	  },
	{ CARD_CS46XX,	 &name_module_init(alsa_card_cs46xx_init),		&name_module_exit(alsa_card_cs46xx_exit)	  },
	{ CARD_CS5535,	 &name_module_init(alsa_card_cs5535audio_init),	&name_module_exit(alsa_card_cs5535audio_exit) },
	{ CARD_ESS1938,	 &name_module_init(alsa_card_es1938_init),		&name_module_exit(alsa_card_es1938_exit)	  },
	{ CARD_ENSONIQ0, &name_module_init(alsa_card_ens1370_init),		&name_module_exit(alsa_card_ens1370_exit)	  },
	{ CARD_ENSONIQ1, &name_module_init(alsa_card_ens1371_init),		&name_module_exit(alsa_card_ens1371_exit)	  },
	{ CARD_YAMAHA,	 &name_module_init(alsa_card_ymfpci_init),		&name_module_exit(alsa_card_ymfpci_exit)	  },
	{ CARD_MAESTRO,	 &name_module_init(alsa_card_es1968_init),		&name_module_exit(alsa_card_es1968_exit)	  },
	{ CARD_MAESTRO3, &name_module_init(alsa_card_m3_init),			&name_module_exit(alsa_card_m3_exit)		  },
	{ CARD_ALI5451,	 &name_module_init(alsa_card_ali_init),			&name_module_exit(alsa_card_ali_exit)		  },
	{ CARD_TRIDENT,	 &name_module_init(alsa_card_trident_init),		&name_module_exit(alsa_card_trident_exit)	  },
#ifdef VORTEX
	{ CARD_VORTEX,	 &name_module_init(alsa_card_vortex_init),		&name_module_exit(alsa_card_vortex_exit)	  },
#else
	{ CARD_VORTEX,	 NULL,		NULL },
#endif
	{ CARD_NEOMAGIC, &name_module_init(alsa_card_nm256_init),		&name_module_exit(alsa_card_nm256_exit)		  },
	{ CARD_FM801,	 &name_module_init(alsa_card_fm801_init),		&name_module_exit(alsa_card_fm801_exit)		  },
	{ CARD_ATIIXP,	 &name_module_init(alsa_card_atiixp_init),		&name_module_exit(alsa_card_atiixp_exit)	  },
	{ CARD_AUDIGYLS, &name_module_init(alsa_card_ca0106_init),		&name_module_exit(alsa_card_ca0106_exit)	  },
	{ CARD_AZX,		 &name_module_init(alsa_card_azx_init),			&name_module_exit(alsa_card_azx_exit)		  },
	{ CARD_BT87X,	 &name_module_init(alsa_card_bt87x_init),		&name_module_exit(alsa_card_bt87x_exit)		  },
};

int 	   nrCardsDetected = 0;
int 	   fStrategyInit = FALSE;
void pcm_info(void);
void FillCaps(ULONG deviceid);

//******************************************************************************
// This routine should be discarded after init time
OSSRET OSS32_Initialize(void)
{
	short sI, iTmp;

	fStrategyInit = TRUE;

	if(DevSetTimer(TimerHandler16) != 0) {
		rprintf(("DevSetTimer failed!!"));
		DebugInt3();
		return OSSERR_INIT_FAILED;
	}
	dprintf(("OSS32_Initialize: Start address: %X", OffsetBeginCS32));

	if(call_module_init(alsa_sound_init) != 0)		 return OSSERR_INIT_FAILED;
	dprintf(("OSS32_Initialize1: Start address: %X", OffsetBeginCS32));
	if(call_module_init(alsa_pcm_init) != 0)		 return OSSERR_INIT_FAILED;
	if(call_module_init(alsa_hwdep_init) != 0)		 return OSSERR_INIT_FAILED;
	if(call_module_init(alsa_timer_init) != 0)		 return OSSERR_INIT_FAILED;

	if(call_module_init(alsa_rawmidi_init) != 0)	 return OSSERR_INIT_FAILED;
	if(call_module_init(alsa_seq_init) != 0)		 return OSSERR_INIT_FAILED;
	if(call_module_init(alsa_opl3_init) != 0)		 return OSSERR_INIT_FAILED;
	if(call_module_init(alsa_opl3_seq_init) != 0)	 return OSSERR_INIT_FAILED;

	if(call_module_init(alsa_mpu401_uart_init) != 0) return OSSERR_INIT_FAILED;

	/* Init functions for HDA audio */
	call_module_init(patch_analog_init);
	//call_module_init(patch_atihdmi_init);
	call_module_init(patch_cmedia_init);
	call_module_init(patch_conexant_init);
	call_module_init(patch_realtek_init);
	call_module_init(patch_sigmatel_init);
	call_module_init(patch_via_init);

	//dprintf(("OSS32_Initialize: ForceCard=%d", ForceCard));

	for (sI=0; sI<CARDS_NUM; sI++) {
		//if ((ForceCard != CARD_NONE) && (ForceCard != cardcalls[sI].card_id)) continue;
		//dprintf(("calling: %x at %x", cardcalls[sI].card_id, cardcalls[sI].cinitcall));
		if (cardcalls[sI].cinitcall == NULL) continue;
		if (*cardcalls[sI].cinitcall == NULL) continue;
		iTmp = (*cardcalls[sI].cinitcall)();
		if (iTmp <= 0) continue;
		while (iTmp) {
			fnCardExitCall[nrCardsDetected++] = *cardcalls[sI].cexitcall;
			iTmp--;
			if (nrCardsDetected >= OSS32_MAX_AUDIOCARDS) break;
		}
		if (nrCardsDetected >= OSS32_MAX_AUDIOCARDS) break;
	}

	fStrategyInit = FALSE;

	if(nrCardsDetected != 0) {
		pcm_info();
		for(int i=0;i<nrCardsDetected;i++) {
			FillCaps(i);
		}
		rprintf(("OSS32_Initialize: SUCCESS. Cards=%d", nrCardsDetected));
		return OSSERR_SUCCESS;
	}
	rprintf(("OSS32_Initialize: FAILED"));
	return OSSERR_INIT_FAILED;
}
//******************************************************************************
//Called during OS/2 shutdown
//******************************************************************************
OSSRET OSS32_Shutdown(void)
{
	CallOSS16(IDC16_EXIT, 0, 0);

	for(int i=0;i<nrCardsDetected;i++) {
		if(fnCardExitCall[i]) fnCardExitCall[i]();
	}

	call_module_exit(alsa_mpu401_uart_exit);
	call_module_exit(alsa_opl3_seq_exit);
	call_module_exit(alsa_opl3_exit);
	call_module_exit(alsa_seq_exit);
	call_module_exit(alsa_rawmidi_exit);
	call_module_exit(alsa_timer_exit);
	call_module_exit(alsa_hwdep_exit);
	call_module_exit(alsa_pcm_exit);
	call_module_exit(alsa_sound_exit);

	return OSSERR_SUCCESS;
}

//******************************************************************************
int OSS32_ProcessIRQ(void)
{
	return CallOSS16(IDC16_PROCESS, 0, 0);
}

int OSS32_ProcessMIDIIRQ(int bytes)
{
	return CallOSS16(IDC16_MIDI_IRQ, bytes, 0);
}

int OSS32_CloseUNI16(void)
{
	return CallOSS16(IDC16_CLOSE_UNI16, 0, 0);
}

//******************************************************************************
int MyDevBlock(ULONG id, ULONG tout, char flag)
{
	return DevBlock(id,tout, flag);
}

int OSS32_DebugString(char *buffer, ULONG size)
{
	rprintf(("U16: %s", buffer));
	return size;
}
