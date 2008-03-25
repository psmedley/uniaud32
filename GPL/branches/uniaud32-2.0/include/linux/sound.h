/* $Id: sound.h,v 1.1.1.1 2003/07/02 13:57:00 eleph Exp $ */

/*
 *	Sound core interface functions
 */
 
extern int register_sound_special(struct file_operations *fops, int unit);
extern int register_sound_mixer(struct file_operations *fops, int dev);
extern int register_sound_midi(struct file_operations *fops, int dev);
extern int register_sound_dsp(struct file_operations *fops, int dev);
extern int register_sound_synth(struct file_operations *fops, int dev);

extern void unregister_sound_special(int unit);
extern void unregister_sound_mixer(int unit);
extern void unregister_sound_midi(int unit);
extern void unregister_sound_dsp(int unit);
extern void unregister_sound_synth(int unit);
