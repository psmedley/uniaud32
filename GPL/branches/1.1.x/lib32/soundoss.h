#ifndef __SOUNDOSS_H__
#define __SOUNDOSS_H__

#define MAGIC_WAVE_ALSA32     0xABCDEF00
#define MAGIC_MIXER_ALSA32    0xABCDEF01
#define MAGIC_MIDI_ALSA32     0xABCDEF02

#define MIXER_PLAYBACKVOLUME    "Playback Volume"
#define MIXER_VOLUME            "Volume"
#define MIXER_PLAYBACKSWITCH    "Playback Switch"
#define MIXER_SWITCH            "Switch"
#define MIXER_SOURCE            "Source"
#define MIXER_BOOST             "Boost"
#define MIXER_CAPTUREROUTE      "Capture Route"
#define MIXER_CAPTURESWITCH     "Capture Switch"
                
typedef struct {
    int            magic;
    int            doublesamplesize;
    struct dentry  d_entry;
    struct inode   inode;
    struct file    file;
} soundhandle;

typedef struct OpenedHandles {
    USHORT FileId;
    soundhandle *handle;
    int reuse; /* reuse flag */
} OpenedHandles;

#define MIDISTATE_OPENED        1
#define MIDISTATE_PORTCREATED   2
#define MIDISTATE_SUBSCRIBED    4

#define RECTYPE_SELECTOR        1       //input selector control
#define RECTYPE_SWITCH          2       //switch for each record input

typedef struct {
    int           magic;
    int           clientid;
    int           clientport;
    int           destclient;
    int           destport; 
    struct dentry d_entry;
    struct inode  inode;
    struct file   file;
    int           state;
} midihandle;

typedef struct {
    int            idxVolume;       //Volume index in snd_ctl_elem_id_t array
    int            idxMute;         //Mute index in snd_ctl_elem_id_t array
    int            idxCustom;       //Custom index in snd_ctl_elem_id_t array (e.g. mic boost or recording gain)
    int            idxCaptureSwitch;//Route indexin snd_ctl_elem_id_t array (record selection)
} mixcontrol;

typedef struct {
    int                 magic;
    struct dentry       d_entry;
    struct inode        inode;
    struct file         file;
    ULONG               reccaps;
    int                 idxRecCaps[OSS32_MIX_RECSRC_MAX];
    int                 rectype;
    snd_ctl_card_info_t info;	//mixer information
    snd_ctl_elem_list_t list;	//element list
    snd_ctl_elem_id_t  *pids;   //array of mixer elements
    mixcontrol          controls[OSS_MIXER_NRDEVICES]; //array of standard mixer controls
} mixerhandle;

extern struct file_operations *alsa_fops;
extern int                     nrCardsDetected;

OSSRET UNIXToOSSError(int unixerror);

OSSRET OSS32_FMMidiLoadInstruments(OSSSTREAMID streamid);

extern int ALSAToOSSDataType(ULONG ALSADataType);
OSSRET ALSAToOSSRateFlags(ULONG fuRates);

#endif //__SOUNDOSS_H__

