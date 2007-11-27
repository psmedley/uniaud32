#ifndef __INITCALL_H__
#define __INITCALL_H__

/*
 * Used for initialization calls..
 */
typedef int  (*initcall_t)(void);
typedef void (*exitcall_t)(void);

extern "C" int use_internal_drums;

#define __initcall(fn)								\
	initcall_t __initcall_##fn  = fn

#define __exitcall(fn)								\
	exitcall_t __exitcall_##fn = fn

#define module_init(x)	__initcall(x);
#define module_exit(x)	__exitcall(x);

#define extern_module_init(x)	extern "C" initcall_t __initcall_##x;
#define extern_module_exit(x)   extern "C" exitcall_t __exitcall_##x;

#define call_module_init(x)     __initcall_##x()
#define call_module_exit(x)     __exitcall_##x()

#define name_module_init(x)     __initcall_##x
#define name_module_exit(x)     __exitcall_##x

#define name_module(x, y, z)          __initcall_##x##y, __exitcall_##x##z
//#define name_module(x)          name_module_init(x)_init, name_module_exit(x)_exit

extern_module_init(alsa_sound_init)
extern_module_exit(alsa_sound_exit)
extern_module_init(alsa_pcm_init)
extern_module_exit(alsa_pcm_exit)
extern_module_init(alsa_hwdep_init)
extern_module_exit(alsa_hwdep_exit)
extern_module_init(alsa_timer_init)
extern_module_exit(alsa_timer_exit)
extern_module_init(alsa_pcm_oss_init)
extern_module_exit(alsa_pcm_oss_exit)
extern_module_init(alsa_rawmidi_init)
extern_module_exit(alsa_rawmidi_exit)
extern_module_init(alsa_seq_init)
extern_module_exit(alsa_seq_exit)
extern_module_init(alsa_opl3_init)
extern_module_exit(alsa_opl3_exit)
extern_module_init(alsa_opl3_seq_init)
extern_module_exit(alsa_opl3_seq_exit)
extern_module_init(alsa_mpu401_uart_init)
extern_module_exit(alsa_mpu401_uart_exit)


//pci cards
extern_module_init(alsa_card_ali_init)
extern_module_exit(alsa_card_ali_exit)
extern_module_init(alsa_card_als4000_init)
extern_module_exit(alsa_card_als4000_exit)
extern_module_init(alsa_card_cs4281_init)
extern_module_exit(alsa_card_cs4281_exit)
extern_module_init(alsa_card_cs46xx_init)
extern_module_exit(alsa_card_cs46xx_exit)
extern_module_init(alsa_card_cmipci_init)
extern_module_exit(alsa_card_cmipci_exit)
extern_module_init(alsa_card_emu10k1_init)
extern_module_exit(alsa_card_emu10k1_exit)
extern_module_init(alsa_card_ens137x_init)
extern_module_exit(alsa_card_ens137x_exit)
extern_module_init(alsa_card_es1938_init)
extern_module_exit(alsa_card_es1938_exit)
extern_module_init(alsa_card_es1968_init)
extern_module_exit(alsa_card_es1968_exit)
extern_module_init(alsa_card_fm801_init)
extern_module_exit(alsa_card_fm801_exit)
extern_module_init(alsa_card_ice1712_init)
extern_module_exit(alsa_card_ice1712_exit)
extern_module_init(alsa_card_intel8x0_init)
extern_module_exit(alsa_card_intel8x0_exit)
extern_module_init(alsa_card_m3_init)
extern_module_exit(alsa_card_m3_exit)
extern_module_init(alsa_card_nm256_init)
extern_module_exit(alsa_card_nm256_exit)
extern_module_init(alsa_card_rme96_init)
extern_module_exit(alsa_card_rme96_exit)
extern_module_init(alsa_card_hammerfall_init)
extern_module_exit(alsa_card_hammerfall_exit)
extern_module_init(alsa_card_sonicvibes_init)
extern_module_exit(alsa_card_sonicvibes_exit)
extern_module_init(alsa_card_trident_init)
extern_module_exit(alsa_card_trident_exit)
extern_module_init(alsa_card_via82xx_init)
extern_module_exit(alsa_card_via82xx_exit)
extern_module_init(alsa_card_ymfpci_init)
extern_module_exit(alsa_card_ymfpci_exit)
extern_module_init(alsa_card_vortex_init)
extern_module_exit(alsa_card_vortex_exit)
extern_module_init(alsa_card_atiixp_init)
extern_module_exit(alsa_card_atiixp_exit)
extern_module_init(alsa_card_ca0106_init)
extern_module_exit(alsa_card_ca0106_exit)
extern_module_init(alsa_card_bt87x_init)
extern_module_exit(alsa_card_bt87x_exit)
extern_module_init(alsa_card_azx_init)
extern_module_exit(alsa_card_azx_exit)


//isa pnp cards
extern_module_init(alsa_card_ad1816a_init)
extern_module_exit(alsa_card_ad1816a_exit)
extern_module_init(alsa_card_ad1848_init)
extern_module_exit(alsa_card_ad1848_exit)
extern_module_init(alsa_card_als100_init)
extern_module_exit(alsa_card_als100_exit)
extern_module_init(alsa_card_cs423x_init)
extern_module_exit(alsa_card_cs423x_exit)
extern_module_init(alsa_card_es18xx_init)
extern_module_exit(alsa_card_es18xx_exit)
extern_module_init(alsa_card_es968_init)
extern_module_exit(alsa_card_es968_exit)
extern_module_init(alsa_card_interwave_init)
extern_module_exit(alsa_card_interwave_exit)
extern_module_init(alsa_card_opl3sa2_init)
extern_module_exit(alsa_card_opl3sa2_exit)
extern_module_init(alsa_card_opti9xx_init)
extern_module_exit(alsa_card_opti9xx_exit)
extern_module_init(alsa_card_sb16_init)
extern_module_exit(alsa_card_sb16_exit)

//isa cards
extern_module_init(alsa_card_cs4231_init)
extern_module_exit(alsa_card_cs4231_exit)
extern_module_init(alsa_card_es1688_init)
extern_module_exit(alsa_card_es1688_exit)
extern_module_init(alsa_card_gusclassic_init)
extern_module_exit(alsa_card_gusclassic_exit)
extern_module_init(alsa_card_gusextreme_init)
extern_module_exit(alsa_card_gusextreme_exit)
extern_module_init(alsa_card_sb8_init)
extern_module_exit(alsa_card_sb8_exit)
extern_module_init(alsa_card_sgalaxy_init)
extern_module_exit(alsa_card_sgalaxy_exit)

#endif //__INITCALL_H__
