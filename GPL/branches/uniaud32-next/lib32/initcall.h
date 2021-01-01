#ifndef __INITCALL_H__
#define __INITCALL_H__

/*
 * Used for initialization calls..
 */
typedef int  (*initcall_t)(void);
typedef void (*exitcall_t)(void);

extern int use_internal_drums;

#define __initcall(fn)								\
	initcall_t __initcall_##fn  = fn

#define __exitcall(fn)								\
	exitcall_t __exitcall_##fn = fn

#define module_init(x)	__initcall(x);
#define module_exit(x)	__exitcall(x);

#define extern_module_init(x)	extern initcall_t __initcall_##x;
#define extern_module_exit(x)   extern exitcall_t __exitcall_##x;

#define call_module_init(x)     __initcall_##x()
#define call_module_exit(x)     __exitcall_##x()

#define name_module_init(x)     __initcall_##x
#define name_module_exit(x)     __exitcall_##x

#define name_module(x, y, z)          __initcall_##x##y, __exitcall_##x##z
//#define name_module(x)          name_module_init(x)_init, name_module_exit(x)_exit

extern_module_init(alsa_sound_init)
extern_module_exit(alsa_sound_exit)
extern_module_init(alsa_seq_device_init)
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
//extern_module_init(alsa_opl3_init)
//extern_module_exit(alsa_opl3_exit)
extern_module_init(opl3_seq_driver_init)
extern_module_exit(opl3_seq_driver_exit)
extern_module_init(regmap_initcall)

//extern_module_init(alsa_mpu401_uart_init)
//extern_module_exit(alsa_mpu401_uart_exit)


//pci cards
extern_module_init(ali5451_driver_init)
extern_module_exit(ali5451_driver_exit)
extern_module_init(als4000_driver_init)
extern_module_exit(als4000_driver_exit)
extern_module_init(cs4281_driver_init)
extern_module_exit(cs4281_driver_exit)
extern_module_init(cs46xx_driver_init)
extern_module_exit(cs46xx_driver_exit)
extern_module_init(cs5535audio_driver_init)
extern_module_exit(cs5535audio_driver_exit)
extern_module_init(cmipci_driver_init)
extern_module_exit(cmipci_driver_exit)
extern_module_init(emu10k1_driver_init)
extern_module_exit(emu10k1_driver_exit)
extern_module_init(ens137x_driver_init)
extern_module_exit(ens137x_driver_exit)
extern_module_init(es1938_driver_init)
extern_module_exit(es1938_driver_exit)
extern_module_init(es1968_driver_init)
extern_module_exit(es1968_driver_exit)
extern_module_init(fm801_driver_init)
extern_module_exit(fm801_driver_exit)
extern_module_init(ice1712_driver_init)
extern_module_exit(ice1712_driver_exit)
extern_module_init(intel8x0_driver_init)
extern_module_exit(intel8x0_driver_exit)
extern_module_init(m3_driver_init)
extern_module_exit(m3_driver_exit)
extern_module_init(nm256_driver_init)
extern_module_exit(nm256_driver_exit)
extern_module_init(rme96_driver_init)
extern_module_exit(rme96_driver_exit)
extern_module_init(hammerfall_driver_init)
extern_module_exit(hammerfall_driver_exit)
extern_module_init(sonicvibes_driver_init)
extern_module_exit(sonicvibes_driver_exit)
extern_module_init(trident_driver_init)
extern_module_exit(trident_driver_exit)
extern_module_init(via82xx_driver_init)
extern_module_exit(via82xx_driver_exit)
extern_module_init(ymfpci_driver_init)
extern_module_exit(ymfpci_driver_exit)
extern_module_init(vortex_driver_init)
extern_module_exit(vortex_driver_exit)
extern_module_init(atiixp_driver_init)
extern_module_exit(atiixp_driver_exit)
extern_module_init(ca0106_driver_init)
extern_module_exit(ca0106_driver_exit)
extern_module_init(alsa_card_bt87x_init)
extern_module_exit(alsa_card_bt87x_exit)
extern_module_init(ac97_bus_init)



//hda
extern_module_init(hda_bus_init)
extern_module_init(generic_driver_init)
extern_module_exit(generic_driver_exit)
extern_module_init(azx_driver_init)
extern_module_exit(azx_driver_exit)
extern_module_init(analog_driver_init)
extern_module_exit(analog_driver_exit)
extern_module_init(cmedia_driver_init)
extern_module_exit(cmedia_driver_exit)
extern_module_init(conexant_driver_init)
extern_module_exit(conexant_driver_exit)
extern_module_init(realtek_driver_init)
extern_module_exit(realtek_driver_exit)
extern_module_init(sigmatel_driver_init)
extern_module_exit(sigmatel_driver_exit)
extern_module_init(via_driver_init)
extern_module_exit(via_driver_exit)


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
