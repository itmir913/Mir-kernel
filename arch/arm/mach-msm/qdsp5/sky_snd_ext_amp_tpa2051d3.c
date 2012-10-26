/*
**
**    AUDIO EXTERNAL AMP
**
**    FILE
**        sky_snd_ext_amp_tpa2051d3.cpp
**
**    DESCRIPTION
**        This file contains audio external amp api
**          
**          void snd_extamp_api_Init()
**          void snd_extamp_api_SetPath()
**          void snd_extamp_api_SetVolume()
**          void snd_extamp_api_Sleep()
**
**    Copyright (c) 2009 by PANTECH Incorporated.  All Rights Reserved.
*************************************************************************************************/


/************************************************************************************************
** Includes
*************************************************************************************************/
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <mach/gpio.h>
#include "sky_snd_ext_amp_tpa2051d3.h"

/************************************************************************************************
** Definition
*************************************************************************************************/
/* Default Register Value */ 
#define DEFAULT_MONO_VOL_CTL_REG_VAL  				(MONO_INPUT | 0x18 /*(0 dB)*/)
#define DEFAULT_STEREO_INPUT1_VOL_CTL_REG_VAL		(0|  0x18 /*(0 dB)*/)
#define DEFAULT_AMP_CTL_REG_VAL						(LIMITER_SELECTED|VOICE_MODE_BYPASS_ENABLE)

/************************************************************************************************
** Variables
*************************************************************************************************/
static extamp_info_t tExtampInfo;
static unsigned char bHeadphonePath = 0;
static unsigned char bSuspend = 0;
static struct i2c_client *tpa2051d3_i2c_client = NULL;
//static u8 curr_amppath = 0x00; //deleted by elecjang 20100312
static unsigned char audio_amp_en = 0; //added by elecjang 20100312 

/************************************************************************************************
** Declaration
*************************************************************************************************/
static int snd_extamp_i2c_write(u8 reg, u8 data);
static int snd_extamp_i2c_read(u8 reg, u8 *data);
static void snd_extamp_shutdown();
static void snd_extamp_sleep(u8 enable);
static int tpa2051d3_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int __exit tpa2051d3_remove(struct i2c_client *client);
static void tpa2051d3_shutdown(struct i2c_client *client);


static int aud_sub_open(struct inode *inode, struct file *file)
{
	printk("[Audio] aud_sub_open");
	return 0;
}

static int aud_sub_release(struct inode *inode, struct file *file)
{
	printk("[Audio] aud_sub_release");
	return 0;	
}

static long aud_sub_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	printk("[Audio] aud_sub_ioctl cmd:%d, arg:%d \n", cmd, arg);
	snd_audio_subsystem_en(cmd, arg);
}



static struct file_operations snd_fops = {
	.owner		= THIS_MODULE,
	.open		= aud_sub_open,
	.release	= aud_sub_release,
	.unlocked_ioctl	= aud_sub_ioctl,
};

static struct miscdevice miscdev = 
{
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     "aud_sub",
	.fops =     &snd_fops
};



/*==========================================================================
** mt9d112_probe
**=========================================================================*/
static int tpa2051d3_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int rc = 0;
	int status = 0;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		tpa2051d3_i2c_client = NULL;
		rc = -1;
	}
	else {
		tpa2051d3_i2c_client = client;
		
		tExtampInfo.mono_vol_ctl_reg_val = DEFAULT_MONO_VOL_CTL_REG_VAL;
		tExtampInfo.stereo_input1_vol_ctl_reg_val = DEFAULT_STEREO_INPUT1_VOL_CTL_REG_VAL;
		tExtampInfo.amp_ctl_reg_val=DEFAULT_AMP_CTL_REG_VAL;
		
		/* Init tpa2051d3 register */
		status |= snd_extamp_i2c_write(MODE_MONO_INPUT_VOL_CTL_REG_ADDR, tExtampInfo.mono_vol_ctl_reg_val);
		status |= snd_extamp_i2c_write(STEREO_INPUT1_OUTPUT_GAIN_CTL_REG_ADDR, tExtampInfo.stereo_input1_vol_ctl_reg_val);
		status |= snd_extamp_i2c_write(AMP_CTL_REG_ADDR, tExtampInfo.amp_ctl_reg_val);	
		if(status){
			rc = -1;
		}
	}
	printk("[Audio] tpa2051d3_probe %d \n", rc);
	return rc;
}


/*==========================================================================
** mt9d112_remove
**=========================================================================*/
static int __exit tpa2051d3_remove(struct i2c_client *client)
{
	int rc;

	tpa2051d3_i2c_client = NULL;
	rc = i2c_detach_client(client);

	return rc;
}

static int tpa2051d3_suspend(struct i2c_client *client, pm_message_t mesg)
{
	//printk("[SKY Audio] Sleep tpa2051d3\n");
	bSuspend = 1;
	snd_extamp_sleep(1);
	return 0;
}

static int tpa2051d3_resume(struct i2c_client *client)
{
	//printk("[SKY Audio] Resume tpa2051d3\n");
	/*if(tExtampInfo.spk_vol_reg_val == 0 && tExtampInfo.hpl_vol_reg_val == 0 && tExtampInfo.hpr_vol_reg_val == 0)
		snd_extamp_sleep(0);*/
	bSuspend = 0;
	return 0;
}

static void tpa2051d3_shutdown(struct i2c_client *client)
{
	snd_extamp_shutdown();
}

static const struct i2c_device_id tpa2051d3_id[] = {
	{ "tpa2051d3-amp", 0},
};

static struct i2c_driver tpa2051d3_driver = {
	.id_table = tpa2051d3_id,
	.probe  = tpa2051d3_probe,
	.remove = __exit_p(tpa2051d3_remove),
	.suspend = tpa2051d3_suspend,
	.resume = tpa2051d3_resume,
	.shutdown = tpa2051d3_shutdown,
	.driver = {
		.name = "tpa2051d3-amp",
	},
};



/*==========================================================================
** snd_extamp_api_Init
**=========================================================================*/
void snd_extamp_api_Init(void)
{
	int result = 0;
	result=misc_register(&miscdev);
	if(result)
	{
		printk(KERN_ERR "misc_register failed.\n");
	}
	

	result = i2c_add_driver(&tpa2051d3_driver);
	if(result){
		/* error */
	}
}

/*==========================================================================
** snd_extamp_api_DeInit
**=========================================================================*/
void snd_extamp_api_DeInit(void)
{
	misc_deregister(&miscdev);
	i2c_del_driver(&tpa2051d3_driver);
}




/*==========================================================================
** snd_extamp_api_SetPath
**=========================================================================*/
void snd_extamp_api_SetPath(int new_device)
{
	u8 reg_value=0;
	//printk("[SKY Audio] snd_extamp_api_SetPath %d \n", new_device);

	switch(new_device)
	{
	case SND_DEVICE_HANDSET: //receiver
		reg_value=LIMITER_SELECTED|VOICE_MODE_BYPASS_ENABLE;
		printk("[SKY Audio Amp] SND_DEVICE_HANDSET %d %d \n", new_device, reg_value);		
		snd_extamp_i2c_write(AMP_CTL_REG_ADDR,reg_value);
		snd_extamp_api_SetVolume(0x18);
		break;

	case SND_DEVICE_HEADSET:
	case SND_DEVICE_STEREO_HEADSET:
		reg_value=LIMITER_SELECTED|HEADPHONE_RIGHT_CHANNEL_ENABLE|HEADPHONE_LEFT_CHANNEL_ENABLE;
		printk("[SKY Audio Amp] SND_DEVICE_HEADSET %d %d \n", new_device, reg_value);			
		snd_extamp_i2c_write(AMP_CTL_REG_ADDR,reg_value);
		snd_extamp_api_SetVolume(0x18);
		break;

	case SND_DEVICE_SPEAKER_PHONE:
		reg_value=LIMITER_SELECTED|SPEAKER_AMPLIFIER_ENABLE;
		printk("[SKY Audio Amp] SND_DEVICE_SPEAKER %d %d \n", new_device, reg_value);
		snd_extamp_i2c_write(AMP_CTL_REG_ADDR,reg_value);
		snd_extamp_api_SetVolume(0x18);
		break;

	case SND_DEVICE_HEADSET_AND_SPEAKER:
		reg_value=LIMITER_SELECTED|HEADPHONE_LEFT_CHANNEL_ENABLE|HEADPHONE_RIGHT_CHANNEL_ENABLE|SPEAKER_AMPLIFIER_ENABLE;
		printk("[SKY Audio Amp] SND_DEVICE_HEADSET_AND_SPEAKER %d %d \n", new_device, reg_value);
		snd_extamp_i2c_write(AMP_CTL_REG_ADDR,reg_value);
		snd_extamp_api_SetVolume(0x18);
		break;

	case SND_DEVICE_TTY_HFK:	
	case SND_DEVICE_TTY_HEADSET:
	case SND_DEVICE_TTY_VCO:
	case SND_DEVICE_TTY_HCO:
		reg_value=LIMITER_SELECTED|HEADPHONE_RIGHT_CHANNEL_ENABLE|HEADPHONE_LEFT_CHANNEL_ENABLE;
		printk("[SKY Audio Amp] SND_DEVICE_TTY %d %d \n", new_device, reg_value);			
		snd_extamp_i2c_write(AMP_CTL_REG_ADDR,reg_value);
		//snd_extamp_api_SetVolume(0x18);
		break;	
	
	case SND_DEVICE_HFK:
	case SND_DEVICE_AHFK:
		break;

	default:
		break;
	}
}

/*==========================================================================
** snd_extamp_api_SetVolume
**=========================================================================*/
void snd_extamp_api_SetVolume(u8 volume)
{
	u8 reg_value=0;
	switch(sub_device) //modified by elecjang 20100312
	{
	case SND_DEVICE_HANDSET://receiver
		break;
			
	case SND_DEVICE_HEADSET:
		reg_value=(MONO_INPUT|volume);
		snd_extamp_i2c_write(MODE_MONO_INPUT_VOL_CTL_REG_ADDR,reg_value);
		break;
	
	case SND_DEVICE_STEREO_HEADSET:
		reg_value=(STEREO_SE_INPUT_1|volume);
		snd_extamp_i2c_write(MODE_MONO_INPUT_VOL_CTL_REG_ADDR,reg_value);
		
		reg_value=volume;
		snd_extamp_i2c_write(STEREO_INPUT1_OUTPUT_GAIN_CTL_REG_ADDR,reg_value);
		break;

	case SND_DEVICE_HEADSET_AND_SPEAKER:
		reg_value=(MONO_INPUT|volume);
		snd_extamp_i2c_write(MODE_MONO_INPUT_VOL_CTL_REG_ADDR,reg_value);
		break;
	
	case SND_DEVICE_SPEAKER_PHONE:
		reg_value=(MONO_INPUT|volume);
		snd_extamp_i2c_write(MODE_MONO_INPUT_VOL_CTL_REG_ADDR,reg_value);
		break;
	
	case SND_DEVICE_TTY_HFK:	
	case SND_DEVICE_TTY_HEADSET:
	case SND_DEVICE_TTY_VCO:
	case SND_DEVICE_TTY_HCO:
		break;
	
	case SND_DEVICE_HFK:
	case SND_DEVICE_AHFK:
		break;

	default:
		break;
			
	}
}


/*==========================================================================
** snd_extamp_api_Sleep
**=========================================================================*/
static void snd_extamp_sleep(u8 enable)
{
}

/*==========================================================================
** snd_extamp_shutdown
**=========================================================================*/
static void snd_extamp_shutdown()
{
	printk("[SKY Audio] AMD Shutdown for power-off\n");
}

/*==========================================================================
** snd_extamp_i2c_write
**=========================================================================*/
static int snd_extamp_i2c_write(u8 reg, u8 data)
{	
	static int ret = 0;
	unsigned char buf[2];
	struct i2c_msg msg[1];

	if(!tpa2051d3_i2c_client){
		return -1;
	}

	buf[0] = (unsigned char)reg;
	buf[1] = (unsigned char)data;

	msg[0].addr = tpa2051d3_i2c_client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = buf;
	
	ret = i2c_transfer(tpa2051d3_i2c_client->adapter, msg, 1);
	if (ret < 0) {
		return -1;
	}
	printk("[Audio] snd_extamp_i2c_write %d \n", ret);
	return 0;
}

/*==========================================================================
** snd_extamp_i2c_read
**=========================================================================*/
static int snd_extamp_i2c_read(u8 reg, u8 *data)
{
	static int ret = 0;
	unsigned char buf[1];
	struct i2c_msg msgs[2];

	if(!tpa2051d3_i2c_client){
		return -1;
	}

	buf[0] = reg;

	msgs[0].addr = tpa2051d3_i2c_client->addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = buf;

	msgs[1].addr = tpa2051d3_i2c_client->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = 1;
	msgs[1].buf = buf;

	ret = i2c_transfer(tpa2051d3_i2c_client->adapter, msgs, 2);
	if ( ret < 0) {
		return -1;
	}

	*data = (u8)buf[0];
	return 0;
}




/*==========================================================================
** snd_extamp_api_SetAudioSystem
**=========================================================================*/
extern void snd_extamp_api_SetAudioSystem(int on)
{

	//printk("[Audio] snd_extamp_api_SetAudioSystem %d \n", on);
	if(on)
	{
		gpio_request(GPIO_AUDIO_ENABLE, "i2c_audio_enable");
		gpio_direction_output(GPIO_AUDIO_ENABLE, 1);
	}
	else
	{
		gpio_direction_output(GPIO_AUDIO_ENABLE, 0);
	}
}

//added by elecjang 20100312
/*==========================================================================
 * ** snd_audio_subsystem_en
 * **=========================================================================*/
void snd_audio_subsystem_en(int en, int new_device)
{

	if(new_device == -1){
		if(audio_amp_en){
        	snd_extamp_api_SetAudioSystem(0); //GPIO OFF
        	audio_amp_en = 0;
    		printk("5. Audio Subsystem Amp Disable %d %d\n", en, audio_amp_en, 0);
		}
		return;
	}
	if(en)
	{
	 	if(audio_amp_en == 0){
			snd_extamp_api_SetAudioSystem(1); //GPIO ON
			audio_amp_en = 1;
			snd_extamp_api_SetPath(new_device); //Path setting
			printk("1.Audio Subsystem Amp Enable %d %d\n", en, audio_amp_en, 0);
		}
		else{
			audio_amp_en = 1;
			snd_extamp_api_SetPath(new_device); //Path setting
			printk("2.Audio Subsystem Amp Enable %d %d\n", en, audio_amp_en, 0);
		}
	}
	else{
		if(audio_amp_en){
        	snd_extamp_api_SetAudioSystem(0); //GPIO OFF
        	audio_amp_en = 0;
    		printk("3.Audio Subsystem Amp Disable %d %d\n", en, audio_amp_en, 0);
		}
		else{
			audio_amp_en = 0;
			printk("4.Audio Subsystem Amp Disable %d %d\n", en, audio_amp_en, 0);
		}
	}
}