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
	{ CARD_ICH,	 &name_module_init(intel8x0_driver_init),	&name_module_exit(intel8x0_driver_exit)	  },
	{ CARD_VIA82XX,	 &name_module_init(via82xx_driver_init),	&name_module_exit(via82xx_driver_exit)	  },
	{ CARD_SBLIVE,	 &name_module_init(emu10k1_driver_init),	&name_module_exit(emu10k1_driver_exit)	  },
	{ CARD_CMEDIA,	 &name_module_init(cmipci_driver_init),		&name_module_exit(cmipci_driver_exit)	  },
	{ CARD_ALS4000,	 &name_module_init(als4000_driver_init),	&name_module_exit(als4000_driver_exit)	  },
	{ CARD_CS4281,	 &name_module_init(cs4281_driver_init),		&name_module_exit(cs4281_driver_exit)	  },
	{ CARD_CS46XX,	 &name_module_init(cs46xx_driver_init),		&name_module_exit(cs46xx_driver_exit)	  },
	{ CARD_CS5535,	 &name_module_init(cs5535audio_driver_init),	&name_module_exit(cs5535audio_driver_exit) },
	{ CARD_ESS1938,	 &name_module_init(es1938_driver_init),		&name_module_exit(es1938_driver_exit)	  },
	{ CARD_ENSONIQ0, &name_module_init(ens137x_driver_init),	&name_module_exit(ens137x_driver_exit)	  },
	{ CARD_ENSONIQ1, NULL,						NULL 					  },
	{ CARD_YAMAHA,	 &name_module_init(ymfpci_driver_init),		&name_module_exit(ymfpci_driver_exit)	  },
	{ CARD_MAESTRO,	 &name_module_init(es1968_driver_init),		&name_module_exit(es1968_driver_exit)	  },
	{ CARD_MAESTRO3, &name_module_init(m3_driver_init),		&name_module_exit(m3_driver_exit)	  },
	{ CARD_ALI5451,	 &name_module_init(ali5451_driver_init),	&name_module_exit(ali5451_driver_exit)	  },
	{ CARD_TRIDENT,	 &name_module_init(trident_driver_init),	&name_module_exit(trident_driver_exit)	  },
#ifdef VORTEX
	{ CARD_VORTEX,	 &name_module_init(vortex_driver_init),		&name_module_exit(vortex_driver_exit)	  },
#else
	{ CARD_VORTEX,	 NULL,						NULL					  },
#endif
	{ CARD_NEOMAGIC, &name_module_init(nm256_driver_init),		&name_module_exit(nm256_driver_exit)	  },
	{ CARD_FM801,	 &name_module_init(fm801_driver_init),		&name_module_exit(fm801_driver_exit)	  },
	{ CARD_ATIIXP,	 &name_module_init(atiixp_driver_init),		&name_module_exit(atiixp_driver_exit)	  },
	{ CARD_AUDIGYLS, &name_module_init(ca0106_driver_init),		&name_module_exit(ca0106_driver_exit)	  },
	{ CARD_AZX,	 &name_module_init(azx_driver_init),		&name_module_exit(azx_driver_exit)	  },
	{ CARD_BT87X,	 &name_module_init(alsa_card_bt87x_init),	&name_module_exit(alsa_card_bt87x_exit)	  },
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

	if(call_module_init(alsa_sound_init) != 0)		return OSSERR_INIT_FAILED;
	if(call_module_init(alsa_seq_device_init) != 0)		return OSSERR_INIT_FAILED;
	dprintf(("OSS32_Initialize1: Start address: %X", OffsetBeginCS32));
	if(call_module_init(alsa_pcm_init) != 0)		return OSSERR_INIT_FAILED;
	if(call_module_init(alsa_hwdep_init) != 0)		return OSSERR_INIT_FAILED;
	if(call_module_init(alsa_timer_init) != 0)		return OSSERR_INIT_FAILED;

	if(call_module_init(alsa_rawmidi_init) != 0)	 	return OSSERR_INIT_FAILED;
	if(call_module_init(alsa_seq_init) != 0)		return OSSERR_INIT_FAILED;
	call_module_init(opl3_seq_driver_init);	 
	call_module_init(regmap_initcall);

	/* Init functions for HDA audio */
	call_module_init(hda_bus_init);
	call_module_init(generic_driver_init);
	call_module_init(analog_driver_init);
	call_module_init(ca0110_driver_init);
	call_module_init(cirrus_driver_init);
	call_module_init(cmedia_driver_init);
	call_module_init(conexant_driver_init);
//	call_module_init(hdmi_driver_init);
	call_module_init(realtek_driver_init);
	call_module_init(sigmatel_driver_init);
	call_module_init(via_driver_init);

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

	call_module_exit(opl3_seq_driver_exit);
	call_module_exit(alsa_seq_exit);
	call_module_exit(alsa_rawmidi_exit);
	call_module_exit(alsa_timer_exit);
	call_module_exit(alsa_hwdep_exit);
	call_module_exit(alsa_pcm_exit);
	call_module_exit(alsa_seq_device_exit);
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
