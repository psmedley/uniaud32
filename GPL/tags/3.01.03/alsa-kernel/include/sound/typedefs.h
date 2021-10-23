/*
 * Typedef's for backward compatibility (for out-of-kernel drivers)
 *
 * This file will be removed soon in future
 */

/* info */
typedef struct snd_info_entry snd_info_entry_t;
typedef struct snd_info_buffer snd_info_buffer_t;


/* seq_oss */
typedef struct snd_seq_oss_arg snd_seq_oss_arg_t;
typedef struct snd_seq_oss_callback snd_seq_oss_callback_t;
typedef struct snd_seq_oss_reg snd_seq_oss_reg_t;

/* opl3/4 */
typedef struct snd_opl3 opl3_t;
typedef struct snd_opl4 opl4_t;

/* i2c */
typedef struct snd_i2c_device snd_i2c_device_t;
typedef struct snd_i2c_bus snd_i2c_bus_t;
