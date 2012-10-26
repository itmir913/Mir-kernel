#ifndef SKY_SND_EXT_AMP_MAX8981_H
#define SKY_SND_EXT_AMP_MAX8981_H
/************************************************************************************************
**
**    AUDIO EXTERNAL AMP
**
**    FILE
**        Sky_snd_ext_amp_max8981.h
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
#define MAX8981_SLAVE_ADDR	0x9A	//b'10011010

/* MAX8981 Control Registers */
#define INPUT_MODE_CTL_REG_ADDR			0x00
#define SPEAKER_VOL_CTL_REG_ADDR			0x01	
#define HEADPHONE_LEFT_CTL_REG_ADDR		0x02	
#define HEADPHONE_RIGHT_CTL_REG_ADDR	0x03
#define OUTPUT_MODE_CTL_REG_ADDR			0x04

/* MAX8981 Input Register */
#define ZCD_EN					0x40
#define ZCD_DIS					0x00
#define INADIFF_STEREO_SINGLE	0x00
#define INADIFF_DIFFERENTIAL	0x20
#define INBDIFF_STEREO_SINGLE	0x00
#define INBDIFF_DIFFERENTIAL	0x10
#define PGAINA_20_DB			0x08
#define PGAINA_5p5_DB			0x04
#define PGAINA_0_DB				0x00
#define PGAINB_20_DB			0x02
#define PGAINB_5p5_DB			0x01
#define PGAINB_0_DB				0x00

/* MAX8981 Volume Control Register */
#define SPKVOL_MUTE			0x00
#define SPKVOL_M_75_DB		0x01
#define SPKVOL_M_71_DB		0x02
#define SPKVOL_M_67_DB		0x03
#define SPKVOL_M_63_DB		0x04
#define SPKVOL_M_59_DB		0x05
#define SPKVOL_M_55_DB		0x06
#define SPKVOL_M_51_DB		0x07
#define SPKVOL_M_47_DB		0x08
#define SPKVOL_M_44_DB		0x09
#define SPKVOL_M_41_DB		0x0A
#define SPKVOL_M_38_DB		0x0B
#define SPKVOL_M_35_DB		0x0C
#define SPKVOL_M_32_DB		0x0D
#define SPKVOL_M_29_DB		0x0E
#define SPKVOL_M_26_DB		0x0F
#define SPKVOL_M_23_DB		0x10
#define SPKVOL_M_21_DB		0x11
#define SPKVOL_M_19_DB		0x12
#define SPKVOL_M_17_DB		0x13
#define SPKVOL_M_15_DB		0x14
#define SPKVOL_M_13_DB		0x15
#define SPKVOL_M_11_DB		0x16
#define SPKVOL_M_9_DB		0x17
#define SPKVOL_M_7_DB		0x18
#define SPKVOL_M_6_DB		0x19
#define SPKVOL_M_5_DB		0x1A
#define SPKVOL_M_4_DB		0x1B
#define SPKVOL_M_3_DB		0x1C
#define SPKVOL_M_2_DB		0x1D
#define SPKVOL_M_1_DB		0x1E
#define SPKVOL_P_0_DB		0x1F

#define HPLVOL_MUTE			0x00
#define HPLVOL_M_75_DB		0x01
#define HPLVOL_M_71_DB		0x02
#define HPLVOL_M_67_DB		0x03
#define HPLVOL_M_63_DB		0x04
#define HPLVOL_M_59_DB		0x05
#define HPLVOL_M_55_DB		0x06
#define HPLVOL_M_51_DB		0x07
#define HPLVOL_M_47_DB		0x08
#define HPLVOL_M_44_DB		0x09
#define HPLVOL_M_41_DB		0x0A
#define HPLVOL_M_38_DB		0x0B
#define HPLVOL_M_35_DB		0x0C
#define HPLVOL_M_32_DB		0x0D
#define HPLVOL_M_29_DB		0x0E
#define HPLVOL_M_26_DB		0x0F
#define HPLVOL_M_23_DB		0x10
#define HPLVOL_M_21_DB		0x11
#define HPLVOL_M_19_DB		0x12
#define HPLVOL_M_17_DB		0x13
#define HPLVOL_M_15_DB		0x14
#define HPLVOL_M_13_DB		0x15
#define HPLVOL_M_11_DB		0x16
#define HPLVOL_M_9_DB		0x17
#define HPLVOL_M_7_DB		0x18
#define HPLVOL_M_6_DB		0x19
#define HPLVOL_M_5_DB		0x1A
#define HPLVOL_M_4_DB		0x1B
#define HPLVOL_M_3_DB		0x1C
#define HPLVOL_M_2_DB		0x1D
#define HPLVOL_M_1_DB		0x1E
#define HPLVOL_P_0_DB		0x1F

#define HPRVOL_MUTE		0x00
#define HPRVOL_M_75_DB		0x01
#define HPRVOL_M_71_DB		0x02
#define HPRVOL_M_67_DB		0x03
#define HPRVOL_M_63_DB		0x04
#define HPRVOL_M_59_DB		0x05
#define HPRVOL_M_55_DB		0x06
#define HPRVOL_M_51_DB		0x07
#define HPRVOL_M_47_DB		0x08
#define HPRVOL_M_44_DB		0x09
#define HPRVOL_M_41_DB		0x0A
#define HPRVOL_M_38_DB		0x0B
#define HPRVOL_M_35_DB		0x0C
#define HPRVOL_M_32_DB		0x0D
#define HPRVOL_M_29_DB		0x0E
#define HPRVOL_M_26_DB		0x0F
#define HPRVOL_M_23_DB		0x10
#define HPRVOL_M_21_DB		0x11
#define HPRVOL_M_19_DB		0x12
#define HPRVOL_M_17_DB		0x13
#define HPRVOL_M_15_DB		0x14
#define HPRVOL_M_13_DB		0x15
#define HPRVOL_M_11_DB		0x16
#define HPRVOL_M_9_DB		0x17
#define HPRVOL_M_7_DB		0x18
#define HPRVOL_M_6_DB		0x19
#define HPRVOL_M_5_DB		0x1A
#define HPRVOL_M_4_DB		0x1B
#define HPRVOL_M_3_DB		0x1C
#define HPRVOL_M_2_DB		0x1D
#define HPRVOL_M_1_DB		0x1E
#define HPRVOL_P_0_DB		0x1F

/* MAX8981 Output Register */
#define SHDN_DIS	0x00
#define SHDN_EN		0x80
#define BYPASS_DIS	0x00
#define BYPASS_EN	0x40
#define INB_DIS		0x00
#define INB_EN		0x10
#define INA_DIS		0x00
#define INA_EN		0x08
#define SPKEN_DIS	0x00
#define SPKEN_EN	0x04
#define HPHEN_DIS	0x00
#define HPHEN_EN	0x01

typedef struct
{
  unsigned char input_mode_ctl_reg_val;
  unsigned char sp_ctl_reg_val;
  unsigned char hpl_ctl_reg_val;
  unsigned char hpr_ctl_reg_val;
  unsigned char output_mode_ctl_reg_val;
}extamp_info_t;

/************************************************************************************************
** Variables
*************************************************************************************************/

/************************************************************************************************
** Declaration
*************************************************************************************************/
#if 1
/*static*/int max8981_suspend(struct platform_device *pdev);
/*static*/int max8981_resume(struct platform_device *pdev);
#else
static int max8981_suspend(struct i2c_client *client);
static int max8981_resume(struct i2c_client *client);
#endif
extern void snd_extamp_api_Init(void);
extern void snd_extamp_api_DeInit(void);
extern void snd_extamp_api_SetDevice(int on, uint32_t cad_device);
extern void snd_extamp_api_SetDefaultVolume(uint32_t cad_device, int unmute);
extern void snd_extamp_api_SetVolume(uint32_t master_volume, int param);
extern void snd_amp_external_control(int which_amp, int value);
extern void snd_extamp_api_Snd_SetDevice(uint32_t cad_device, int ear_mute, int mic_mute);
extern int snd_extamp_GetCurrentDevice(void);
extern void max8981_external_suspend(void);
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
  SND_DEVICE_VOIP_HANDSET	= SND_DEVICE_BT_INTERCOM,
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
#define FEATURE_SKY_SND_AUTOANSWER

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
#define HP_INCALL   HPLVOL_P_0_DB
#define HP_VOIP_INCALL  HPLVOL_P_0_DB
#define HP_VT_INCALL   HPLVOL_P_0_DB

#define HP_MEDIA  HPLVOL_P_0_DB
#define HP_DUAL_MEDIA  HPLVOL_M_11_DB //pz1693_110310
#define SP_SPKPHONE     SPKVOL_M_4_DB
#ifdef FEATURE_SKY_SND_CAL_SEGMENT
#define VT_CALL_START_GAIN      SPKVOL_M_4_DB

#define VT_SPKPHONE     SPKVOL_M_4_DB
#define VOIP_SPKPHONE     SPKVOL_M_4_DB
#else
#define VT_SPKPHONE     SPKVOL_M_4_DB
#define VOIP_SPKPHONE     SPKVOL_M_4_DB
#endif
#define SP_MEDIA         SPKVOL_M_4_DB
/************************************************/
#endif /*SKY_SND_EXT_AMP_MAX8981_H*/

