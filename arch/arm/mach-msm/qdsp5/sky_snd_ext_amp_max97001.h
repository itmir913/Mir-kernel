#ifndef SKY_SND_EXT_AMP_MAX97001_H
#define SKY_SND_EXT_AMP_MAX97001_H
/************************************************************************************************
**
**    AUDIO EXTERNAL AMP
**
**    FILE
**        Sky_snd_ext_amp_max97001.h
**
**    DESCRIPTION
**        This file contains audio external amp api
**
**    Copyright (c) 2010 by PANTECH Incorporated.  All Rights Reserved.
*************************************************************************************************/


/************************************************************************************************
** Includes
*************************************************************************************************/
#include <linux/kernel.h>
/************************************************************************************************
** Definition
*************************************************************************************************/
#define MAX97001_SLAVE_ADDR	0x4D	//b'1001101


/* MAX97001 Control Registers */
#define INPUT_GAIN_CTL_REG_ADDR     		0x00
#define HEADPHONE_MIXERS_CTL_REG_ADDR     0x01	
#define SPEAKER_MIXER_CTL_REG_ADDR   		0x02	
#define HEADPHONE_LEFT_CTL_REG_ADDR   	0x03
#define HEADPHONE_RIGHT_CTL_REG_ADDR    	0x04
#define SPEAKER_CTL_REG_ADDR   			0x05
#define RESERVED_CTL_REG_ADDR    			0x06
#define LIMITER_CTL_REG_ADDR  				0x07
#define POWER_MANAGEMENT_CTL_REG_ADDR 	0x08
#define CHARGE_PUMP_CTL_REG_ADDR 		0x09
#define REV_ID_CTL_REG_ADDR				0x10


/* Input Register */
#define INADIFF_STEREO_SINGLE  0x00  //0x00
#define INADIFF_DIFFERENTIAL  0x80
#define INBDIFF_STEREO_SINGLE  0x00
#define INBDIFF_DIFFERENTIAL  0x40
#define PGAINA_M_6_DB  0x00
#define PGAINA_M_3_DB  0x08
#define PGAINA_P_0_DB  0x10
#define PGAINA_P_3_DB  0x18
#define PGAINA_P_6_DB  0x20
#define PGAINA_P_9_DB  0x28
#define PGAINA_P_18_DB  0x30
#define PGAINA_EXT  0x03F
#define PGAINB_M_6_DB  0x00
#define PGAINB_M_3_DB  0x01
#define PGAINB_P_0_DB  0x02
#define PGAINB_P_3_DB  0x03
#define PGAINB_P_6_DB  0x04
#define PGAINB_P_9_DB  0x05
#define PGAINB_P_18_DB  0x06
#define PGAINB_EXT  0x07

#define DATASHEET_FIX
/* Mixers Registers */
#ifdef DATASHEET_FIX
#define HPLMIX_NO_INPUT  0x00  //0x01
#define HPLMIX_INA1  0x10
#define HPLMIX_INA2  0x20
#define HPLMIX_INB1  0x40
#define HPLMIX_INB2  0x80
#define HPRMIX_NO_INPUT  0x00
#define HPRMIX_INA1  0x01
#define HPRMIX_INA2  0x02
#define HPRMIX_INB1  0x04
#define HPRMIX_INB2  0x08

#define SPKMIX_NO_INPUT  0x00  //0x02
#define SPKMIX_INA1  0x01
#define SPKMIX_INA2  0x02
#define SPKMIX_INB1  0x04
#define SPKMIX_INB2  0x08
#else
#define HPLMIX_NO_INPUT  0x00  //0x01
#define HPLMIX_INA1  0x80
#define HPLMIX_INA2  0x40
#define HPLMIX_INB1  0x20
#define HPLMIX_INB2  0x10
#define HPRMIX_NO_INPUT  0x00
#define HPRMIX_INA1  0x08
#define HPRMIX_INA2  0x04
#define HPRMIX_INB1  0x02
#define HPRMIX_INB2  0x01

#define SPKMIX_NO_INPUT  0x00  //0x02
#define SPKMIX_INA1  0x08
#define SPKMIX_INA2  0x04
#define SPKMIX_INB1  0x02
#define SPKMIX_INB2  0x01
#endif

/* Volume Control Register */
#define ZCD_EN  0x00  //0x03
#define ZCD_DIS   0x80
#define SLEW_EN   0x00
#define SLEW_DIS  0x40
#define HPLM_UNMUTE  0x00
#define HPLM_MUTE    0x20
#define HPLVOL_M_64_DB     0x00
#define HPLVOL_M_60_DB     0x01
#define HPLVOL_M_56_DB     0x02
#define HPLVOL_M_52_DB     0x03
#define HPLVOL_M_48_DB     0x04
#define HPLVOL_M_44_DB     0x05
#define HPLVOL_M_40_DB     0x06
#define HPLVOL_M_37_DB     0x07
#define HPLVOL_M_34_DB     0x08
#define HPLVOL_M_31_DB     0x09
#define HPLVOL_M_28_DB     0x0A
#define HPLVOL_M_25_DB     0x0B
#define HPLVOL_M_22_DB     0x0C
#define HPLVOL_M_19_DB     0x0D
#define HPLVOL_M_16_DB     0x0E
#define HPLVOL_M_14_DB     0x0F
#define HPLVOL_M_12_DB     0x10
#define HPLVOL_M_10_DB     0x11
#define HPLVOL_M_8_DB       0x12
#define HPLVOL_M_6_DB       0x13
#define HPLVOL_M_4_DB       0x14
#define HPLVOL_M_2_DB       0x15
#define HPLVOL_M_1_DB       0x16
#define HPLVOL_P_0_DB       0x17
#define HPLVOL_P_1_DB       0x18
#define HPLVOL_P_2_DB       0x19
#define HPLVOL_P_3_DB       0x1A
#define HPLVOL_P_4_DB       0x1B
#define HPLVOL_P_4_DOT_5_DB     0x1C
#define HPLVOL_P_5_DB                  0x1D
#define HPLVOL_P_5_DOT_5_DB     0x1E
#define HPLVOL_P_6_DB                  0x1F

#define LPGAIN_P_0_DB  0x00  //0x04
#define LPGAIN_P_3_DB  0x80
#define HPRM_UNMUTE  0x00
#define HPRM_MUTE  0x20
#define HPRVOL_M_64_DB     0x00
#define HPRVOL_M_60_DB     0x01
#define HPRVOL_M_56_DB     0x02
#define HPRVOL_M_52_DB     0x03
#define HPRVOL_M_48_DB     0x04
#define HPRVOL_M_44_DB     0x05
#define HPRVOL_M_40_DB     0x06
#define HPRVOL_M_37_DB     0x07
#define HPRVOL_M_34_DB     0x08
#define HPRVOL_M_31_DB     0x09
#define HPRVOL_M_28_DB     0x0A
#define HPRVOL_M_25_DB     0x0B
#define HPRVOL_M_22_DB     0x0C
#define HPRVOL_M_19_DB     0x0D
#define HPRVOL_M_16_DB     0x0E
#define HPRVOL_M_14_DB     0x0F
#define HPRVOL_M_12_DB     0x10
#define HPRVOL_M_10_DB     0x11
#define HPRVOL_M_8_DB       0x12
#define HPRVOL_M_6_DB       0x13
#define HPRVOL_M_4_DB       0x14
#define HPRVOL_M_2_DB       0x15
#define HPRVOL_M_1_DB       0x16
#define HPRVOL_P_0_DB       0x17
#define HPRVOL_P_1_DB       0x18
#define HPRVOL_P_2_DB       0x19
#define HPRVOL_P_3_DB       0x1A
#define HPRVOL_P_4_DB       0x1B
#define HPRVOL_P_4_DOT_5_DB     0x1C
#define HPRVOL_P_5_DB                  0x1D
#define HPRVOL_P_5_DOT_5_DB     0x1E
#define HPRVOL_P_6_DB                  0x1F
  
#define FFM_SS_MODE 0x00  //0x05
#define FFM_FF_MODE 0x80
#define SPKM_UNMUTE  0x00
#define SPKM_MUTE  0x40
#define SPKVOL_M_30_DB     0x18
#define SPKVOL_M_26_DB     0x19
#define SPKVOL_M_22_DB     0x1A
#define SPKVOL_M_18_DB     0x1B
#define SPKVOL_M_14_DB     0x1C
#define SPKVOL_M_12_DB     0x1D
#define SPKVOL_M_10_DB     0x1E
#define SPKVOL_M_8_DB       0x1F
#define SPKVOL_M_6_DB       0x20
#define SPKVOL_M_4_DB       0x21
#define SPKVOL_M_2_DB       0x22
#define SPKVOL_P_0_DB       0x23
#define SPKVOL_P_1_DB       0x24
#define SPKVOL_P_2_DB       0x25
#define SPKVOL_P_3_DB       0x26
#define SPKVOL_P_4_DB       0x27
#define SPKVOL_P_5_DB       0x28
#define SPKVOL_P_6_DB       0x29
#define SPKVOL_P_7_DB       0x2A
#define SPKVOL_P_8_DB       0x2B
#define SPKVOL_P_9_DB       0x2C
#define SPKVOL_P_10_DB     0x2D
#define SPKVOL_P_11_DB     0x2E
#define SPKVOL_P_12_DB     0x2F
#define SPKVOL_P_12_DOT_5_DB      0x30
#define SPKVOL_P_13_DB                  0x31
#define SPKVOL_P_13_DOT_5_DB      0x32
#define SPKVOL_P_14_DB                  0x33
#define SPKVOL_P_14_DOT_5_DB     0x34
#define SPKVOL_P_15_DB                  0x35
#define SPKVOL_P_15_DOT_5_DB     0x36
#define SPKVOL_P_16_DB                  0x37
#define SPKVOL_P_16_DOT_5_DB     0x38
#define SPKVOL_P_17_DB                  0x39
#define SPKVOL_P_17_DOT_5_DB     0x3A
#define SPKVOL_P_18_DB                  0x3B
#define SPKVOL_P_18_DOT_5_DB     0x3C
#define SPKVOL_P_19_DB                  0x3D
#define SPKVOL_P_19_DOT_5_DB     0x3E
#define SPKVOL_P_20_DB                  0x3F


/* Distortion Limiter Register */
#define THDCLP_DIS 0x00  //0x07
#define THDCLP_BELOW_4_PER  0x90
#define THDCLP_BELOW_5_PER  0xA0
#define THDCLP_BELOW_6_PER  0xB0
#define THDCLP_BELOW_8_PER  0xC0
#define THDCLP_BELOW_11_PER  0xD0
#define THDCLP_BELOW_12_PER  0xE0
#define THDCLP_BELOW_15_PER  0xF0
#define THDT1_1_DOT_4_SEC  0x00
#define THDT1_2_DOT_8_SEC  0x01


/* Power Management Register */
#define SHDN_EN  0x80
#define SHDN_DIS  0x0
#define LPMODE_DIS  0x00  
#define LPMODE_INA_SINGLE_END  0x20
#define LPMODE_INB_SINGLE_END  0x40
#define LPMODE_INA_DIFFERENTIAL  0x60
#define SPKEN_DIS  0x00
#define SPKEN_EN  0x10
#define HPLEN_DIS  0x00
#define HPLEN_EN  0x04
#define HPREN_DIS  0x00
#define HPREN_EN  0x02
#define SWEN_DIS  0x00
#define SWEN_EN  0x01


/* Charge Pump Control Register */
#define CPSEL_1 0x02  //+-0.9V on HPVDD/HPVSS  //0x09
#define CPSEL_0 0x00  //+-1.8V on HPVDD/HPVSS
#define F_SUPPLY_MODE 0x01  //Fixed Supply Mode
#define CLASS_H_MODE 0x00  //Class H Mode

typedef enum 
{
  EXTAMP_OUT_SPK,
  EXTAMP_OUT_HPH_L,
  EXTAMP_OUT_HPH_R,
  EXTAMP_OUT_HPH_LR,
  EXTAMP_OUT_SPK_HPH,
  EXTAMP_OUT_BYPASS
}extamp_outmode_e;

typedef enum
{
  EXTAMP_IN_INA,
  EXTAMP_IN_INB,
  EXTAMP_IN_INAB,
  EXTAMP_IN_RXIN
}extamp_inmode_e;

typedef enum
{
	EXTAMP_MONO,
	EXTAMP_STEREO
}extamp_outfmt_e;

typedef struct
{
  unsigned char inp_gain_ctl_reg_val;
  unsigned char hp_mixers_ctl_reg_val;
  unsigned char sp_mixer_ctl_reg_val;
  unsigned char hpl_ctl_reg_val;
  unsigned char hpr_ctl_reg_val;
  unsigned char sp_ctl_reg_val;
  unsigned char limiter_ctl_reg_val;
  unsigned char power_man_ctl_reg_val; 
  unsigned char charge_pump_ctl_reg_val;
}extamp_info_t;

/************************************************************************************************
** Variables
*************************************************************************************************/

/************************************************************************************************
** Declaration
*************************************************************************************************/
static int max97001_suspend(struct i2c_client *client);
static int max97001_resume(struct i2c_client *client);

extern void snd_extamp_api_Init(void);
extern void snd_extamp_api_DeInit(void);
extern void snd_extamp_api_SetDevice(int on, uint32_t cad_device);
extern void snd_extamp_api_SetDefaultVolume(uint32_t cad_device, int unmute);
extern void snd_extamp_api_SetVolume(uint32_t master_volume, int param);
extern void snd_amp_external_control(int which_amp, int value);
extern void snd_extamp_api_Snd_SetDevice(uint32_t cad_device, int ear_mute, int mic_mute);
extern int snd_extamp_GetCurrentDevice(void);
extern void max97001_external_suspend(void);
/* This definition is used to force enums to use 32 bits - required for L4 */
#define SNDDEV_DUMMY_DATA_UINT32_MAX 0x7FFFFFFF

/*  All of the audio devices supported on this platform
*/
typedef enum {
  SND_DEVICE_DEFAULT                     = 0,
  SND_DEVICE_HANDSET                     = SND_DEVICE_DEFAULT+0,
  SND_DEVICE_HFK		         = SND_DEVICE_DEFAULT+1,
  SND_DEVICE_HEADSET		         = SND_DEVICE_DEFAULT+2, /* Mono headset               */
  SND_DEVICE_STEREO_HEADSET	         = SND_DEVICE_DEFAULT+3, /* Stereo headset             */
  SND_DEVICE_AHFK		         = SND_DEVICE_DEFAULT+4,
  SND_DEVICE_SDAC		         = SND_DEVICE_DEFAULT+5,
  SND_DEVICE_SPEAKER_PHONE	         = SND_DEVICE_DEFAULT+6,
  SND_DEVICE_TTY_HFK		         = SND_DEVICE_DEFAULT+7,
  SND_DEVICE_SPKPHONE		         = SND_DEVICE_TTY_HFK,
  SND_DEVICE_TTY_HEADSET	         = SND_DEVICE_DEFAULT+8,
  SND_DEVICE_VT_SPK	         = SND_DEVICE_TTY_HEADSET,
  SND_DEVICE_TTY_VCO		         = SND_DEVICE_DEFAULT+9,
  SND_DEVICE_VOIP_SPK		         = SND_DEVICE_TTY_VCO,
  SND_DEVICE_TTY_HCO		         = SND_DEVICE_DEFAULT+10,
  SND_DEVICE_VOIP_HEADSET		         = SND_DEVICE_TTY_HCO,
  SND_DEVICE_BT_INTERCOM	         = SND_DEVICE_DEFAULT+11,
  SND_DEVICE_VOIP_HANDSET = SND_DEVICE_BT_INTERCOM,
  SND_DEVICE_BT_HEADSET		         = SND_DEVICE_DEFAULT+12,  
  SND_DEVICE_BT_AG_LOCAL_AUDIO	         = SND_DEVICE_DEFAULT+13,
  SND_DEVICE_USB		         = SND_DEVICE_DEFAULT+14,
  SND_DEVICE_STEREO_USB			 = SND_DEVICE_DEFAULT+15,
  SND_DEVICE_IN_S_SADC_OUT_HANDSET	 = SND_DEVICE_DEFAULT+16, /* Input Mono   SADD, Output Handset */
  SND_DEVICE_IN_S_SADC_OUT_HEADSET	 = SND_DEVICE_DEFAULT+17, /* Input Stereo SADD, Output Headset */
  SND_DEVICE_EXT_S_SADC_OUT_HANDSET	 = SND_DEVICE_DEFAULT+18, /* Input Stereo  SADD, Output Handset */
  SND_DEVICE_EXT_S_SADC_OUT_HEADSET	 = SND_DEVICE_DEFAULT+19, /* Input Stereo SADD, Output Headset  */
  SND_DEVICE_BT_A2DP_HEADSET		 = SND_DEVICE_DEFAULT+20, /* A BT device supporting A2DP */
  SND_DEVICE_BT_A2DP_SCO_HEADSET	 = SND_DEVICE_DEFAULT+21, /* A BT headset supporting A2DP and SCO */
  /* Input Internal Codec Stereo SADC, Output External AUXPCM  */
  SND_DEVICE_TX_INT_SADC_RX_EXT_AUXPCM	 = SND_DEVICE_DEFAULT+22,
  SND_DEVICE_RX_EXT_SDAC_TX_INTERNAL	 = SND_DEVICE_DEFAULT+23,
  SND_DEVICE_BT_CONFERENCE               = SND_DEVICE_DEFAULT+24,
  SND_DEVICE_IN_S_SADC_OUT_SPEAKER_PHONE = SND_DEVICE_DEFAULT+25,
  SND_DEVICE_MAX			 = SND_DEVICE_DEFAULT+26,
  SND_DEVICE_CURRENT			 = SND_DEVICE_DEFAULT+27,
  SND_DEVICE_HEADSET_AND_SPEAKER =SND_DEVICE_DEFAULT+28,
  /* DO NOT USE: Force this enum to be a 32bit type */
  SND_DEVICE_32BIT_DUMMY                 = SNDDEV_DUMMY_DATA_UINT32_MAX
} snd_device_type;

#define FEATURE_BOOT_SOUND_PLAY
#define FEATURE_SKY_SND_DUAL_PATH_DEFAULT_SPEAKER
#define FEATURE_SKY_SND_DUAL_PATH_REDIFINITION
#define FEATURE_SKY_SND_CAL_SEGMENT
#define FEATURE_SKY_SND_FROYO_PATCH

#ifdef FEATURE_BOOT_SOUND_PLAY
typedef enum {
  BOOTSND_PLAY_DONE  =0,
  BOOTSND_SET_PATH = 1,
  BOOTSND_PLAYING = 2,
  BOOTSND_MAX
} boot_snd_play_type;
extern int boot_snd_play;
#endif

/********* CAL VALUE *****************************/
#define HP_INCALL   HPLVOL_M_12_DB
#define HP_VOIP_INCALL  HPLVOL_M_12_DB
#define HP_VT_INCALL   HPLVOL_M_6_DB

#define HP_MEDIA  HPLVOL_M_10_DB
#define SP_SPKPHONE     SPKVOL_P_5_DB
#ifdef FEATURE_SKY_SND_CAL_SEGMENT
#define VT_CALL_START_GAIN      SPKVOL_P_19_DB

#define VT_SPKPHONE     SPKVOL_P_5_DB
#define VOIP_SPKPHONE     SPKVOL_P_5_DB
#else
#define VT_SPKPHONE     SPKVOL_P_17_DB
#define VOIP_SPKPHONE     SPKVOL_P_17_DB
#endif
#define SP_MEDIA         SPKVOL_P_6_DB
/************************************************/
#endif /*SKY_SND_EXT_AMP_MAX97001_H*/

