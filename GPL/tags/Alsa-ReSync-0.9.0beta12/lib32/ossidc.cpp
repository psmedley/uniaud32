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
#define INCL_DOSERRORS           // for ERROR_INVALID_FUNCTION
#include <os2.h>
#include <ossdefos2.h>
#include <ossidc32.h>
#include <dbgos2.h>
#include <devhelp.h>
#include <unicard.h>
#ifdef KEE
#include <kee.h>
#endif
#include "initcall.h"

extern "C" DWORD TimerHandler16; 	//drv32\strategy.asm
extern "C" int OSS32_ProcessMIDIIRQ(int bytes);
extern "C" int OSS32_CloseUNI16(void);

//******************************************************************************
//******************************************************************************
BOOL CallOSS16(ULONG cmd, ULONG param1, ULONG param2)
{
    BOOL         rc;

    if(idc16_PddHandler == 0) {
	    return FALSE;
    }

    rc = CallPDD16(idc16_PddHandler, cmd, param1, param2);
    return rc;
}
//******************************************************************************
exitcall_t fnCardExitCall[OSS32_MAX_AUDIOCARDS] = {0};
extern "C" {
#if 0

    typedef int  (initcall_tt)(void);
    typedef void (exitcall_tt)(void);

typedef struct cardcalls_t {
    int card_id;
    initcall_tt *cinitcall;
    exitcall_tt *cexitcall;
}cardcalls_t;

cardcalls_t cardcalls[1] = {
    //    { CARD_SBLIVE,   name_module(alsa_card_emu10k1, _init, _exit) },
    { CARD_SBLIVE,   __initcall_alsa_card_emu10k1_init, __exitcall_alsa_card_emu10k1_exit },
/*
    { CARD_CMEDIA,   name_module(alsa_card_cmipci, _init, _exit) },
    { CARD_ALS4000,  name_module(alsa_card_als4000, _init, _exit) },
    { CARD_CS4281,   name_module(alsa_card_cs4281, _init, _exit) },
    { CARD_ICH,      name_module(alsa_card_intel8x0, _init, _exit) },
    { CARD_CS46XX,   name_module(alsa_card_cs46xx, _init, _exit) },
    { CARD_VIA82XX,  name_module(alsa_card_via82xx, _init, _exit) },
    { CARD_ESS1938,  name_module(alsa_card_es1938, _init, _exit) },
//    { CARD_VORTEX,   name_module(alsa_card_vortex, _init, _exit) },
    { CARD_ENSONIQ,  name_module(alsa_card_ens137x, _init, _exit) },
    { CARD_YAMAHA,   name_module(alsa_card_ymfpci, _init, _exit) },
    { CARD_MAESTRO,  name_module(alsa_card_es1968, _init, _exit) },
    { CARD_MAESTRO3, name_module(alsa_card_m3, _init, _exit) },
    { CARD_ALI5451,  name_module(alsa_card_ali, _init, _exit) },
    { CARD_TRIDENT,  name_module(alsa_card_trident, _init, _exit) },
    { CARD_NEOMAGIC, name_module(alsa_card_nm256, _init, _exit) },
    { CARD_ATIIXP,   name_module(alsa_card_atiixp, _init, _exit) },
    { CARD_FM801,    name_module(alsa_card_fm801, _init, _exit) },
    { CARD_AUDIGYLS, name_module(alsa_card_ca0106, _init, _exit) },
    { CARD_BT87X,    name_module(alsa_card_bt87x, _init, _exit) },
    { CARD_AZX,      name_module(alsa_card_azx, _init, _exit) }*/
};
#endif

int        nrCardsDetected = 0;
int        fStrategyInit = FALSE;
void pcm_info(void);
void FillCaps(ULONG deviceid);
};
//******************************************************************************
OSSRET OSS32_Initialize(void)
{
    int i;

    fStrategyInit = TRUE;

    if(DevSetTimer(TimerHandler16) != 0) {
        dprintf(("DevSetTimer failed!!"));
        DebugInt3();
        return OSSERR_INIT_FAILED;
    }

    dprintf(("OSS32_Initialize. Start addrees: %X", OffsetBeginCS32));
//    DebugInt3();

    if(call_module_init(alsa_sound_init) != 0)       return OSSERR_INIT_FAILED;
    dprintf(("OSS32_Initialize1"));
    if(call_module_init(alsa_pcm_init) != 0)         return OSSERR_INIT_FAILED;
    if(call_module_init(alsa_hwdep_init) != 0)       return OSSERR_INIT_FAILED;
    if(call_module_init(alsa_timer_init) != 0)       return OSSERR_INIT_FAILED;

    if(call_module_init(alsa_rawmidi_init) != 0)     return OSSERR_INIT_FAILED;
    if(call_module_init(alsa_seq_init) != 0)         return OSSERR_INIT_FAILED;
    if(call_module_init(alsa_opl3_init) != 0)        return OSSERR_INIT_FAILED;
    if(call_module_init(alsa_opl3_seq_init) != 0)    return OSSERR_INIT_FAILED;

    if(call_module_init(alsa_mpu401_uart_init) != 0) return OSSERR_INIT_FAILED;
#if 0
    if (ForceCard != CARD_NONE)
    {
        if (cardcalls[ForceCard].cinitcall() == 0)
            fnCardExitCall[nrCardsDetected] = cardcalls[ForceCard].cexitcall;
        for (i=0; i < CARDS_NUM; i++)
        {
            if (i==ForceCard) continue;
            if (cardcalls[i].cinitcall() == 0)
                fnCardExitCall[nrCardsDetected] = cardcalls[i].cexitcall;
            if (nrCardsDetected >= OSS32_MAX_AUDIOCARDS)
                break;
        }
    } else
    {
        for (i=0; i < CARDS_NUM; i++)
        {
            dprintf(("calling: %X at %X", i, cardcalls[i].cinitcall));
            if (cardcalls[i].cinitcall() == 0)
                fnCardExitCall[nrCardsDetected] = cardcalls[i].cexitcall;
            if (nrCardsDetected >= OSS32_MAX_AUDIOCARDS)
                break;
        }
    }
#else
    dprintf(("force card: %d",ForceCard));
    //Check for SoundBlaster Live!
    if((ForceCard == CARD_NONE || ForceCard == CARD_ICH) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_intel8x0_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_intel8x0_exit);
    }
    if((ForceCard == CARD_NONE || ForceCard == CARD_VIA82XX) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_via82xx_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_via82xx_exit);
    }

    if((ForceCard == CARD_NONE || ForceCard == CARD_SBLIVE) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_emu10k1_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_emu10k1_exit);
    }

    if((ForceCard == CARD_NONE || ForceCard == CARD_CMEDIA) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_cmipci_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_cmipci_exit);
    }
    if((ForceCard == CARD_NONE || ForceCard == CARD_ALS4000) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_als4000_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_als4000_exit);
    }
    if((ForceCard == CARD_NONE || ForceCard == CARD_CS4281) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_cs4281_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_cs4281_exit);
    }
    if((ForceCard == CARD_NONE || ForceCard == CARD_CS46XX) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_cs46xx_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_cs46xx_exit);
    }
    if((ForceCard == CARD_NONE || ForceCard == CARD_ESS1938) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_es1938_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_es1938_exit);
    }
    if((ForceCard == CARD_NONE || ForceCard == CARD_ENSONIQ) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_ens137x_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_ens137x_exit);
    }
    if((ForceCard == CARD_NONE || ForceCard == CARD_YAMAHA) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_ymfpci_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_ymfpci_exit);
    }
    if((ForceCard == CARD_NONE || ForceCard == CARD_MAESTRO) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_es1968_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_es1968_exit);
    }
    if((ForceCard == CARD_NONE || ForceCard == CARD_MAESTRO3) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_m3_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_m3_exit);
    }
    if((ForceCard == CARD_NONE || ForceCard == CARD_ALI5451) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_ali_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_ali_exit);
    }
    if((ForceCard == CARD_NONE || ForceCard == CARD_TRIDENT) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_trident_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_trident_exit);
    }

#ifdef VORTEX
    if((ForceCard == CARD_NONE || ForceCard == CARD_VORTEX) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_vortex_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_vortex_exit);
   }
#endif
    if((ForceCard == CARD_NONE || ForceCard == CARD_NEOMAGIC) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_nm256_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_nm256_exit);
    }
    if((ForceCard == CARD_NONE || ForceCard == CARD_FM801) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_fm801_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_fm801_exit);
    }
    if((ForceCard == CARD_NONE || ForceCard == CARD_ATIIXP) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_atiixp_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_atiixp_exit);
    }
    if((ForceCard == CARD_NONE || ForceCard == CARD_AUDIGYLS) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_ca0106_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_ca0106_exit);
    }
    if((ForceCard == CARD_NONE || ForceCard == CARD_AZX) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_azx_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_azx_exit);
    }
    if((ForceCard == CARD_NONE || ForceCard == CARD_BT87X) &&
       nrCardsDetected < (OSS32_MAX_AUDIOCARDS-1) && call_module_init(alsa_card_bt87x_init) == 0)
    {
        fnCardExitCall[nrCardsDetected] = name_module_exit(alsa_card_bt87x_exit);
    }

#endif
    fStrategyInit = FALSE;

    if(nrCardsDetected != 0) {
        dprintf(("OSS32_Initialize2"));
        pcm_info();
        dprintf(("OSS32_Initialize3"));
        for(int i=0;i<nrCardsDetected;i++) {
            dprintf(("OSS32_Initialize4 start: %d",i));
            FillCaps(i);
            dprintf(("OSS32_Initialize4 end: %d",i));
        }
        dprintf(("OSS32_Initialize: SUCCESS. nr. of cards: %d",nrCardsDetected));
        return OSSERR_SUCCESS;
    }
    dprintf(("OSS32_Initialize. FAILED"));
    return OSSERR_INIT_FAILED;
}
//******************************************************************************
//Called during OS/2 shutdown
//******************************************************************************
OSSRET OSS32_Shutdown()
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
//******************************************************************************
int OSS32_ProcessIRQ()
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
//******************************************************************************
extern "C" int MyDevBlock(ULONG id, ULONG tout, char flag);

int MyDevBlock(ULONG id, ULONG tout, char flag)
{
    return DevBlock(id,tout, flag);
}

