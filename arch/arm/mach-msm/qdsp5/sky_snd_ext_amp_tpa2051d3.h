#ifndef SNDEXTAMP_H
#define SNDEXTAMP_H
/************************************************************************************************
**
**    AUDIO EXTERNAL AMP
**
**    FILE
**        SndExtAmp.h
**
**    DESCRIPTION
**        This file contains audio external amp api
**
**    Copyright (c) 2009 by PANTECH Incorporated.  All Rights Reserved.
*************************************************************************************************/


/************************************************************************************************
** Includes
*************************************************************************************************/
#include <linux/kernel.h>
/************************************************************************************************
** Definition
*************************************************************************************************/

#define TPA2051D3_SLAVE_ADDR	  				0x70	//b'1110000
//#define MAX9877_SLAVE_ADDR_W    				0xE0	//write 11100000
//#define MAX9877_SLAVE_ADDR_R    				0xE1	//read 11100001

#define GPIO_AUDIO_ENABLE						109


#define AMP_CTL_REG_ADDR						0x01
#define MODE_MONO_INPUT_VOL_CTL_REG_ADDR		0x04
#define STEREO_INPUT1_OUTPUT_GAIN_CTL_REG_ADDR	0x05


/*Amplifier Control Register*/
#define LIMITER_SELECTED						0x80
#define LIMITER_FUNCTION_ENABLE					0x40
#define SOFTWARE_SHUTDOWN						0x10
#define HEADPHONE_LEFT_CHANNEL_ENABLE			0x08
#define HEADPHONE_RIGHT_CHANNEL_ENABLE			0x04
#define	SPEAKER_AMPLIFIER_ENABLE				0x02
#define	VOICE_MODE_BYPASS_ENABLE				0x01


/*Mode */
#define MONO_INPUT								(0x00<<5)
#define MONO_DIFF_INPUT_1						(0x01<<5)
#define MONO_DIFF_INPUT_2						(0x02<<5)
#define STEREO_SE_INPUT_1						(0x03<<5)
#define STEREO_SE_INPUT_2						(0x04<<5)
#define MONO_DIFF_INPUT_1_PLUS_2				(0x05<<5)
#define MONO_SE_INPUT_1_PLUS_2					(0x06<<5)
#define	MUTE									(0x07<<5)

/*Stereo Input 1*/
#define STEREO_INPUT_1_SPK_GAIN 				0x80
#define STEREO_INPUT_1_HP_0dB					0x40


#define SND_DEVICE_HANDSET						0
#define SND_DEVICE_HFK				   			1
#define SND_DEVICE_HEADSET				   		2 	/* Mono headset */
#define SND_DEVICE_STEREO_HEADSET		   		3 	/* Stereo headset */
#define SND_DEVICE_AHFK 			   			4
#define SND_DEVICE_SDAC 			   			5
#define SND_DEVICE_SPEAKER_PHONE		   		6
#define SND_DEVICE_TTY_HFK				   		7
#define SND_DEVICE_TTY_HEADSET			   		8
#define SND_DEVICE_TTY_VCO				   		9
#define SND_DEVICE_TTY_HCO				   		10
#define SND_DEVICE_HEADSET_AND_SPEAKER			28
#define SND_DEVICE_CURRENT			   		27


/*Stereo Input 1 / Output Gain Control Register*/
typedef struct
{
  u8	mono_vol_ctl_reg_val;
  u8	stereo_input1_vol_ctl_reg_val;
  u8	amp_ctl_reg_val;
}extamp_info_t;

/************************************************************************************************
** Variables
*************************************************************************************************/

/************************************************************************************************
** Declaration
*************************************************************************************************/
extern void snd_extamp_api_Init(void);
extern void snd_extamp_api_DeInit(void);
extern void snd_extamp_api_SetPath(int new_device);
extern void snd_extamp_api_SetVolume(u8 volume);
extern void snd_extamp_api_SetAudioSystem(int on);
//added by elecjang 20100312 
extern void snd_audio_subsystem_en(int en, int new_device);
extern int sub_device;
#endif /*SNDEXTAMP_H*/
