/************************************************************************************************
**
**    AUDIO EXTERNAL AMP
**
**    FILE
**        Sky_snd_ext_amp_max970001.c
**
**    DESCRIPTION
**        This file contains audio external amp api
**          
**          void snd_extamp_api_Init()
**          void snd_extamp_api_SetPath()
**          void snd_extamp_api_SetVolume()
**          void snd_extamp_api_Sleep()
**
**    Copyright (c) 2010 by PANTECH Incorporated.  All Rights Reserved.
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
#include <asm/ioctls.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/platform_device.h>
//#include "dal.h"
//#include "dal_audio.h"
#include "audmgr.h"
#include "sky_snd_ext_amp_max8981.h"

/************************************************************************************************
** Definition
*************************************************************************************************/

/************************************************************************************************
** Variables
*************************************************************************************************/
/*static*/ extamp_info_t tExtampInfo;
static unsigned char bHeadphonePath = 0;
//static struct i2c_client *max8981_i2c_client = NULL;
static uint32_t CurrDeviceId = 0xffffffff;

extamp_info_t TestExtampInfo;
int TestExtampEnable[10]={0,0,0,0,0,0,0,0,0,0};
int TestStaticExtampEnable=0;

int sky_amp_log[4]={1,1,1,0};   // 4:  AMP suspend
extern int g_call_status;
#ifdef FEATURE_BOOT_SOUND_PLAY
extern int earjack_status;
#endif
extern void snd_extamp_api_SetDevice(int on, uint32_t cad_device);

/************************************************************************************************
** Declaration
*************************************************************************************************/
static int snd_extamp_i2c_write(u8 reg, u8 data);
/*static int snd_extamp_i2c_read(u8 reg, u8 *data);*/
#if 1
static int max8981_probe(struct platform_device *pdev);
static int __exit max8981_remove(struct platform_device *pdev);
static void max8981_shutdown(struct platform_device *pdev);
#else
static int max8981_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int __exit max8981_remove(struct i2c_client *client);
static void max8981_shutdown(struct i2c_client *client);
#endif
/*static*/ void snd_extamp_sleep(u8 enable);

int snd_extamp_write_all_reg(extamp_info_t tCurrentExtampInfo);
void snd_extamp_make_default(extamp_info_t* tCurrentExtampInfo);

static int aud_sub_open(struct inode *inode, struct file *file)
{
	if(sky_amp_log[0]) printk("[Audio] aud_sub_open \n");
	return 0;
}

static int aud_sub_release(struct inode *inode, struct file *file)
{
	if(sky_amp_log[0]) printk("[Audio] aud_sub_release \n");
	return 0;	
}

// EF13S added // SangwonLee 100511
#define SND_AMP_IOCTL_MAGIC 'a'
#define SND_SKY_SET_AMPVOLUME _IOW(SND_AMP_IOCTL_MAGIC, 0, unsigned)
#ifdef FEATURE_BOOT_SOUND_PLAY
#define SND_SKY_SET_BOOTSOUND _IOW(SND_AMP_IOCTL_MAGIC, 1, unsigned)
extern int earjack_status;
int boot_snd_play=BOOTSND_PLAY_DONE;
#endif
#define SND_SKY_SET_AMPVOLUME_TEST _IOW(SND_AMP_IOCTL_MAGIC, 2, unsigned)
#define SND_SKY_GET_AMPVOLUME_TEST _IOW(SND_AMP_IOCTL_MAGIC, 3, unsigned)
#define SND_SKY_SET_MODE_PARAM  _IOW(SND_AMP_IOCTL_MAGIC, 4, unsigned)
#define SND_SKY_GET_CURR_DEV _IOW(SND_AMP_IOCTL_MAGIC, 5, unsigned)
#define SND_SKY_SET_RECORD_STATUS _IOW(SND_AMP_IOCTL_MAGIC, 6, unsigned)
#define SND_SKY_SET_EXCEPTION_STATUS _IOW(SND_AMP_IOCTL_MAGIC, 7, unsigned)
#define SND_SKY_GET_MODE_PARAM  _IOW(SND_AMP_IOCTL_MAGIC, 8, unsigned)

#ifdef FEATURE_SKY_SND_AUTOANSWER
#define SND_SKY_SET_AUTOANSER_STATUS  _IOW(SND_AMP_IOCTL_MAGIC, 9, unsigned)
static int auto_answering_status=0;
#endif

int test_preAMPgain = PGAINB_5p5_DB/*PGAINB_0_DB*/;

int test_spkPreAMPgain = PGAINA_0_DB;
int test_hpPreAMPgain = PGAINB_5p5_DB/*PGAINB_0_DB*/;
int test_spkAMPgain = SPKVOL_M_4_DB/*SPKVOL_P_0_DB*/;
int test_hpAMPgain = HPRVOL_P_0_DB;

#define LOOPBACK_DEVICE_OFF 0
#define LOOPBACK_DEVICE_ON  1
int loopback_test=0;
int record_status=0;
static int setAMP_exception=0;
static int exloopback_path=0;
int current_mode_param[2]={0,0};    // TEMP_100806
static int CurrCallMode = 0;
static int vt_mute_condition=0;
static int forced_mute=0;

/*static*/ long aud_sub_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
       uint32_t value;
	if(sky_amp_log[0]) printk("[Audio] aud_sub_ioctl cmd:%d, arg:%d \n", cmd, arg);
    	switch (cmd) {
            case LOOPBACK_DEVICE_ON:
                  exloopback_path=CurrDeviceId;
                  snd_extamp_api_SetDevice(1, arg);
                  snd_extamp_api_SetDefaultVolume(arg,1);
                  loopback_test = 1;
  	           if(sky_amp_log[0]) printk("[Audio] LOOPBACK_DEVICE_ON \n");
                  break;                
            case LOOPBACK_DEVICE_OFF:
		    if (loopback_test) {
                      loopback_test = 0;
                      snd_extamp_api_SetDevice(0, exloopback_path);
                      snd_extamp_api_SetDefaultVolume(exloopback_path,0);
      	               if(sky_amp_log[0]) printk("[Audio] LOOPBACK_DEVICE_OFF \n");
		    }
                  break;                
            case SND_SKY_SET_AMPVOLUME_TEST:
                  if (get_user(value, (uint32_t __user *) arg)) {
                     if(sky_amp_log[0]) printk("[Audio] aud_sub_ioctl cmd SND_SKY_SET_AMPVOLUME\n");
			return;
                  }
                  if(sky_amp_log[0]) printk("[Audio] aud_sub_ioctl cmd SND_SKY_SET_AMPVOLUME: %d\n", value);
                  if (value >= 0x100 && value < 0x1000)    // preAMP
                  {
                      value = value & 0x700;
                      value = value >> 8;
                      if(value==7)  value=0;
                      if(sky_amp_log[0]) printk("[Audio] aud_sub_ioctl cmd SND_SKY_SET_AMPVOLUME(2): 0x%X(preAMP)\n", value);
                      snd_amp_external_control(0, value);  
                  }
                  else if(value < 0x100)   // spkAMP
                  {
                      value = value & 0x3f;
                      if(sky_amp_log[0]) printk("[Audio] aud_sub_ioctl cmd SND_SKY_SET_AMPVOLUME(2): 0x%02X (spkAMP)\n", value);
                      snd_amp_external_control(1, value);
                  }
				  else	// headphone AMP
				  {
				  	value = value & 0xFF000;
					value = value >> 12;
					if(sky_amp_log[0]) printk("[Audio] aud_sub_ioctl cmd SND_SKY_SET_AMPVOLUME(2): 0x%02X(hpAMP)\n", value);
					snd_amp_external_control(2, value);
                  }
              break;
       // EF13S added // SangwonLee 100511
            case SND_SKY_SET_AMPVOLUME:    {
                  if (get_user(value, (uint32_t __user *) arg)) {
                     if(sky_amp_log[0]) printk("[Audio] Err:aud_sub_ioctl cmd SND_SKY_SET_AMPVOLUME\n");
					 return;
                  }
                  if(sky_amp_log[1]) printk("[Audio] aud_sub_ioctl cmd SND_SKY_SET_AMPVOLUME: 0x%02X\n", value);
                  
                  if(!value)     snd_extamp_api_SetDefaultVolume(CurrDeviceId, 0);
                  else{
#if 1//EF32K QC2925		  	
  			if((CurrDeviceId == 3) && g_call_status) snd_extamp_api_SetDevice(1, CurrDeviceId);
#endif			
				  	snd_extamp_api_SetDefaultVolume(CurrDeviceId, 1);
                  	}

                  forced_mute = true;
              }
              break;
			case SND_SKY_GET_AMPVOLUME_TEST:
				switch (arg)
				{
					case 0:  // preAMP
						if(sky_amp_log[0]) printk("[Audio] get preAMP=[0x%02X]\n", test_preAMPgain);
						return test_preAMPgain;
					case 1:  // spkAMP
					if(sky_amp_log[0]) printk("[Audio] get spkAMP=[0x%02X]\n", test_spkAMPgain);
						return test_spkAMPgain;
					case 2:  // hpAMP
						if(sky_amp_log[0]) printk("[Audio] get hpAMP=[0x%02X]\n", test_hpAMPgain);
						return test_hpAMPgain;
					default:
						return -1;
				}
              break;
#ifdef FEATURE_BOOT_SOUND_PLAY
           case SND_SKY_SET_BOOTSOUND:  {
                     if (get_user(value, (uint32_t __user *) arg)) {
                        if(sky_amp_log[0]) printk("[Audio] Err:aud_sub_ioctl cmd SND_SKY_SET_BOOTSOUND\n");
   			return;
                     }
                     boot_snd_play=BOOTSND_SET_PATH;
                     if(sky_amp_log[2]) printk("[Audio] aud_sub_ioctl cmd SND_SKY_SET_BOOTSOUND(%d): value=%d, earjack=%d \n", SND_SKY_SET_BOOTSOUND, value, earjack_status);
                     if(earjack_status)     {
                        if(!value) snd_extamp_api_SetDefaultVolume(SND_DEVICE_STEREO_HEADSET,0);
                        snd_extamp_api_SetDevice(value,SND_DEVICE_STEREO_HEADSET);
                        snd_extamp_api_SetDefaultVolume(SND_DEVICE_STEREO_HEADSET,value);
                     }
                     else   {
                        if(!value) snd_extamp_api_SetDefaultVolume(SND_DEVICE_SPEAKER_PHONE,0);
                        snd_extamp_api_SetDevice(value,SND_DEVICE_SPEAKER_PHONE);
                        snd_extamp_api_SetDefaultVolume(SND_DEVICE_SPEAKER_PHONE,value);
                     }
                     if(value)   boot_snd_play=BOOTSND_PLAYING;
                     else     {
                        boot_snd_play=BOOTSND_PLAY_DONE;
                        CurrDeviceId = SND_DEVICE_SPEAKER_PHONE;    // Make Default.
                     }

                     if(sky_amp_log[2]) printk("[Audio] aud_sub_ioctl cmd SND_SKY_SET_BOOTSOUND: boot_snd_play=%d \n", boot_snd_play);
              }
              break;
#endif
           case SND_SKY_SET_MODE_PARAM:
                  if (get_user(value, (uint32_t __user *) arg)) {
                        printk("[Audio] Err:aud_sub_ioctl cmd SND_SKY_SET_MODE_PARAM\n");
			   return;
                  }
                      current_mode_param[0] = (value&0xff00)>>8;
                      current_mode_param[1] = (value&0xff);

                  if(sky_amp_log[1]) printk("[Audio] aud_sub_ioctl cmd SND_SKY_SET_MODE_PARAM: 0x%02X, [0]%d, [1]%d\n", value, current_mode_param[0], current_mode_param[1]);

                  if(current_mode_param[0]==1)  {   // VT
                        //g_call_status= true;    // g_call_status is false until setMode.

                        if(current_mode_param[1]==1)    {     //Mute Critical Section. 
                            vt_mute_condition=true;
                            snd_extamp_api_SetDefaultVolume(CurrDeviceId, 0);
                        }
                        else {
                            if(current_mode_param[1]==2) vt_mute_condition=false;
				if((vt_mute_condition == true) && (current_mode_param[1]==0)) vt_mute_condition=false;//seoultek VT callend mute fix
                            snd_extamp_api_SetDefaultVolume(CurrDeviceId, 1);
                        }
                  }
                  
                  #ifndef FEATURE_SKY_SND_CAL_SEGMENT
                  if(current_mode_param[0])                    snd_extamp_api_SetDefaultVolume(CurrDeviceId, 1);
                  #endif
              break;
          case SND_SKY_GET_MODE_PARAM:
              if(sky_amp_log[1]) printk("[Audio] aud_sub_ioctl cmd SND_SKY_GET_MODE_PARAM current mode:%d\n", current_mode_param[0]);
		if (copy_to_user((void*) arg, &current_mode_param[0], sizeof(int))) {
			printk("[Audio] aud_sub_ioctl cmd SND_SKY_GET_MODE_PARAM ERROR!!!\n");
		}
            break;
	   // jykim100713@DS2
          case SND_SKY_GET_CURR_DEV:
		  	return CurrDeviceId;
		break;
          case SND_SKY_SET_RECORD_STATUS:
                  if (get_user(value, (uint32_t __user *) arg)) {
                        printk("[Audio] Err:aud_sub_ioctl cmd SND_SKY_SET_RECORD_STATUS\n");
			   return;
                  }
                  if(sky_amp_log[1]) printk("[Audio] aud_sub_ioctl cmd SND_SKY_SET_RECORD_STATUS: %d\n", value);
                  record_status = value;
              break;
          case SND_SKY_SET_EXCEPTION_STATUS:
                  if (get_user(value, (uint32_t __user *) arg)) {
                        printk("[Audio] Err:aud_sub_ioctl cmd SND_SKY_SET_EXCEPTION_STATUS\n");
			   return;
                  }
                  if(sky_amp_log[1]) printk("[Audio] aud_sub_ioctl cmd SND_SKY_SET_EXCEPTION_STATUS: %d\n", value);
                  setAMP_exception = value;
              break;
#ifdef FEATURE_SKY_SND_AUTOANSWER
	case SND_SKY_SET_AUTOANSER_STATUS:
		if (get_user(value, (uint32_t __user *) arg)) {
			  printk("[Audio] Err:aud_sub_ioctl cmd SND_SKY_SET_AUTOANSER_STATUS\n");
			  return;
		}
		if(sky_amp_log[0]) printk("[Audio] SND_SKY_SET_AUTOANSER_STATUS (%d)\n", value);
		auto_answering_status = value;
		break;
#endif            
           default:
              break;
    	}
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
#if 1
static int max8981_probe(struct platform_device *pdev)
{
 	int rc = 0;
	int status = 0;
	/*u8 inregval, outregval, volregval;*/
/*
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		max8981_i2c_client = NULL;
		rc = -1;
	}
	else {
		max8981_i2c_client = client;
*/
	{
              snd_extamp_make_default(&tExtampInfo);
              snd_extamp_make_default(&TestExtampInfo);

    		/* Init MAX8981 register */
		status |= snd_extamp_i2c_write(INPUT_MODE_CTL_REG_ADDR, tExtampInfo.input_mode_ctl_reg_val);
		status |= snd_extamp_i2c_write(SPEAKER_VOL_CTL_REG_ADDR, tExtampInfo.sp_ctl_reg_val);
		status |= snd_extamp_i2c_write(HEADPHONE_LEFT_CTL_REG_ADDR, tExtampInfo.hpl_ctl_reg_val);
		status |= snd_extamp_i2c_write(HEADPHONE_RIGHT_CTL_REG_ADDR, tExtampInfo.hpr_ctl_reg_val);
		status |= snd_extamp_i2c_write(OUTPUT_MODE_CTL_REG_ADDR, tExtampInfo.output_mode_ctl_reg_val);

#if 0
              snd_extamp_api_SetDevice(1,SND_DEVICE_SPEAKER_PHONE);
              snd_extamp_api_SetVolume(SPKVOL_M_30_DB);
              msleep(10);
              snd_extamp_api_SetDevice(0,SND_DEVICE_SPEAKER_PHONE);
              snd_extamp_api_SetVolume(0);
#endif
		if(status){
			rc = -1;
		}
	}
	if(sky_amp_log[1]) printk("[SKY AUDIO] max8981_probe %d \n", rc);
	return rc;
    
}
#else
static int max8981_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
 	int rc = 0;
	int status = 0;
	/*u8 inregval, outregval, volregval;*/
/*
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		max8981_i2c_client = NULL;
		rc = -1;
	}
	else {
		max8981_i2c_client = client;
*/
	{
              snd_extamp_make_default(&tExtampInfo);
              snd_extamp_make_default(&TestExtampInfo);

    		/* Init MAX8981 register */
		status |= snd_extamp_i2c_write(INPUT_MODE_CTL_REG_ADDR, tExtampInfo.input_mode_ctl_reg_val);
		status |= snd_extamp_i2c_write(SPEAKER_VOL_CTL_REG_ADDR, tExtampInfo.sp_ctl_reg_val);
		status |= snd_extamp_i2c_write(HEADPHONE_LEFT_CTL_REG_ADDR, tExtampInfo.hpl_ctl_reg_val);
		status |= snd_extamp_i2c_write(HEADPHONE_RIGHT_CTL_REG_ADDR, tExtampInfo.hpr_ctl_reg_val);
		status |= snd_extamp_i2c_write(OUTPUT_MODE_CTL_REG_ADDR, tExtampInfo.output_mode_ctl_reg_val);

#if 0
              snd_extamp_api_SetDevice(1,SND_DEVICE_SPEAKER_PHONE);
              snd_extamp_api_SetVolume(SPKVOL_M_30_DB);
              msleep(10);
              snd_extamp_api_SetDevice(0,SND_DEVICE_SPEAKER_PHONE);
              snd_extamp_api_SetVolume(0);
#endif
		if(status){
			rc = -1;
		}
	}
	if(sky_amp_log[1]) printk("[SKY AUDIO] max8981_probe %d \n", rc);
	return rc;
    
}
#endif
/*==========================================================================
** max8981_remove
**=========================================================================*/
#if 1
static int __exit max8981_remove(struct platform_device *pdev)
{
	int rc = 0;
//	max8981_i2c_client = NULL;
	/*rc = i2c_detach_client(client);*/
	return rc;
}
#else
static int __exit max8981_remove(struct i2c_client *client)
{
	int rc = 0;
//	max8981_i2c_client = NULL;
	/*rc = i2c_detach_client(client);*/
	return rc;
}
#endif
#if 1
/*static*/int max8981_suspend(struct platform_device *pdev)
{
	unsigned char SuspendOk = 0;
       unsigned char spk_vol, hpl_vol, hpr_vol,ctl_val;
#if 1
	if(tExtampInfo.sp_ctl_reg_val == SPKVOL_MUTE){
		spk_vol = 1;
		}
	else{
		spk_vol = 0;
		}
	if(tExtampInfo.hpl_ctl_reg_val == HPLVOL_MUTE){
		hpl_vol = 1;
		}
	else{
		hpl_vol = 0;
		}
	if(tExtampInfo.hpr_ctl_reg_val == HPRVOL_MUTE){
		hpr_vol = 1;
		}
	else{
		hpr_vol = 0;
		}
	if(tExtampInfo.output_mode_ctl_reg_val == (SHDN_DIS | BYPASS_EN | SPKEN_DIS | HPHEN_DIS)){
		ctl_val = 1;
		}
	else{
		ctl_val = 0;
		}
#else
       spk_vol = tExtampInfo.sp_ctl_reg_val & SPKVOL_MUTE ? 1:0;
       hpl_vol =  tExtampInfo.hpl_ctl_reg_val & HPLVOL_MUTE ? 1:0;
       hpr_vol =  tExtampInfo.hpr_ctl_reg_val & HPRVOL_MUTE ? 1:0;
#endif
	if(sky_amp_log[1])        printk("[SKY Audio] AMP suspend(%d)(%d)(%d)\n", spk_vol, hpl_vol, hpr_vol);
    
       if(!g_call_status)   {
            if(!spk_vol)     {
                tExtampInfo.sp_ctl_reg_val = SPKVOL_MUTE;
                snd_extamp_i2c_write(SPEAKER_VOL_CTL_REG_ADDR, tExtampInfo.sp_ctl_reg_val);
                spk_vol = 1;
            }
            if(!hpl_vol)     {
                tExtampInfo.hpl_ctl_reg_val = HPLVOL_MUTE;
                snd_extamp_i2c_write(HEADPHONE_LEFT_CTL_REG_ADDR, tExtampInfo.hpl_ctl_reg_val);
                hpl_vol = 1;
            }
            if(!hpr_vol)     {
                tExtampInfo.hpr_ctl_reg_val = HPRVOL_MUTE;
                snd_extamp_i2c_write(HEADPHONE_RIGHT_CTL_REG_ADDR, tExtampInfo.hpr_ctl_reg_val);
                hpr_vol = 1;
            }
       }
	SuspendOk = spk_vol & hpl_vol & hpr_vol;
#if 1
	if((SuspendOk) && (!ctl_val))
#else
       if(SuspendOk)
#endif	   	
	{
            tExtampInfo.output_mode_ctl_reg_val = SHDN_DIS | BYPASS_EN | SPKEN_DIS | HPHEN_DIS;
   	     snd_extamp_i2c_write(OUTPUT_MODE_CTL_REG_ADDR, tExtampInfo.output_mode_ctl_reg_val);
       }
	if(sky_amp_log[4])        printk("[SKY Audio] AMP suspend(%d)\n", SuspendOk);
	
	return 0;
}
#else
static int max8981_suspend(struct i2c_client *client)
{
	unsigned char SuspendOk = 0;
       unsigned char spk_vol, hpl_vol, hpr_vol;

       spk_vol = tExtampInfo.sp_ctl_reg_val & SPKVOL_MUTE ? 1:0;
       hpl_vol =  tExtampInfo.hpl_ctl_reg_val & HPLVOL_MUTE ? 1:0;
       hpr_vol =  tExtampInfo.hpr_ctl_reg_val & HPRVOL_MUTE ? 1:0;

	if(sky_amp_log[1])        printk("[SKY Audio] AMP suspend(%d)(%d)(%d)\n", spk_vol, hpl_vol, hpr_vol);
    
       if(!g_call_status)   {
            if(!spk_vol)     {
                tExtampInfo.sp_ctl_reg_val = SPKVOL_MUTE;
                snd_extamp_i2c_write(SPEAKER_VOL_CTL_REG_ADDR, tExtampInfo.sp_ctl_reg_val);
                spk_vol = 1;
            }
            if(!hpl_vol)     {
                tExtampInfo.hpl_ctl_reg_val = HPLVOL_MUTE;
                snd_extamp_i2c_write(HEADPHONE_LEFT_CTL_REG_ADDR, tExtampInfo.hpl_ctl_reg_val);
                hpl_vol = 1;
            }
            if(!hpr_vol)     {
                tExtampInfo.hpr_ctl_reg_val = HPRVOL_MUTE;
                snd_extamp_i2c_write(HEADPHONE_RIGHT_CTL_REG_ADDR, tExtampInfo.hpr_ctl_reg_val);
                hpr_vol = 1;
            }
       }
	SuspendOk = spk_vol & hpl_vol & hpr_vol;

       if(SuspendOk)    {
            tExtampInfo.output_mode_ctl_reg_val = SHDN_DIS | BYPASS_DIS | SPKEN_DIS | HPHEN_DIS;
   	     snd_extamp_i2c_write(OUTPUT_MODE_CTL_REG_ADDR, tExtampInfo.output_mode_ctl_reg_val);
       }
	if(sky_amp_log[4])        printk("[SKY Audio] AMP suspend(%d)\n", SuspendOk);
	
	return 0;
}
#endif
void max8981_external_suspend(void)
{
	unsigned char SuspendOk = 0;
       unsigned char spk_vol, hpl_vol, hpr_vol,ctl_val;
#if 1
		   if(tExtampInfo.sp_ctl_reg_val == SPKVOL_MUTE){
			   spk_vol = 1;
			   }
		   else{
			   spk_vol = 0;
			   }
		   if(tExtampInfo.hpl_ctl_reg_val == HPLVOL_MUTE){
			   hpl_vol = 1;
			   }
		   else{
			   hpl_vol = 0;
			   }
		   if(tExtampInfo.hpr_ctl_reg_val == HPRVOL_MUTE){
			   hpr_vol = 1;
			   }
		   else{
			   hpr_vol = 0;
			   }
		   if(tExtampInfo.output_mode_ctl_reg_val == (SHDN_DIS | BYPASS_EN | SPKEN_DIS | HPHEN_DIS)){
			   ctl_val = 1;
			   }
		   else{
			   ctl_val = 0;
			   }
#else
       spk_vol = tExtampInfo.sp_ctl_reg_val & SPKVOL_MUTE ? 1:0;
       hpl_vol =  tExtampInfo.hpl_ctl_reg_val & HPLVOL_MUTE ? 1:0;
       hpr_vol =  tExtampInfo.hpr_ctl_reg_val & HPRVOL_MUTE ? 1:0;
#endif
	if(sky_amp_log[1])        printk("[SKY Audio] AMP External suspend(%d)(%d)(%d)\n", spk_vol, hpl_vol, hpr_vol);
    
       if(!g_call_status)   {
            if(!spk_vol)     {
                tExtampInfo.sp_ctl_reg_val = SPKVOL_MUTE;
                snd_extamp_i2c_write(SPEAKER_VOL_CTL_REG_ADDR, tExtampInfo.sp_ctl_reg_val);
                spk_vol = 1;
            }
            if(!hpl_vol)     {
                tExtampInfo.hpl_ctl_reg_val = HPLVOL_MUTE;
                snd_extamp_i2c_write(HEADPHONE_LEFT_CTL_REG_ADDR, tExtampInfo.hpl_ctl_reg_val);
                hpl_vol = 1;
            }
            if(!hpr_vol)     {
                tExtampInfo.hpr_ctl_reg_val = HPRVOL_MUTE;
                snd_extamp_i2c_write(HEADPHONE_RIGHT_CTL_REG_ADDR, tExtampInfo.hpr_ctl_reg_val);
                hpr_vol = 1;
            }
       }
	SuspendOk = spk_vol & hpl_vol & hpr_vol;
#if 1
	if((SuspendOk) && (!ctl_val))
#else
       if(SuspendOk)
#endif	   	
	{
            tExtampInfo.output_mode_ctl_reg_val = SHDN_DIS | BYPASS_EN | SPKEN_DIS | HPHEN_DIS;
   	     snd_extamp_i2c_write(OUTPUT_MODE_CTL_REG_ADDR, tExtampInfo.output_mode_ctl_reg_val);
       }
	if(sky_amp_log[4])        printk("[SKY Audio] AMP External suspend(%d)\n", SuspendOk);	
}
#if 1
/*static*/int max8981_resume(struct platform_device *pdev)
{
	snd_extamp_sleep(1);
	return 0;
}
#else
static int max8981_resume(struct i2c_client *client)
{
	snd_extamp_sleep(1);
	return 0;
}
#endif
#if 1
static void max8981_shutdown(struct platform_device *pdev)
{
	tExtampInfo.output_mode_ctl_reg_val = SHDN_DIS | BYPASS_DIS | SPKEN_DIS | HPHEN_DIS;
	
	snd_extamp_i2c_write(OUTPUT_MODE_CTL_REG_ADDR, tExtampInfo.output_mode_ctl_reg_val);
	if(sky_amp_log[4]) printk("[SKY Audio] AMP Shutdown for power-off\n");
}
#else
static void max8981_shutdown(struct i2c_client *client)
{
	tExtampInfo.output_mode_ctl_reg_val = SHDN_EN;
	
	snd_extamp_i2c_write(OUTPUT_MODE_CTL_REG_ADDR, tExtampInfo.output_mode_ctl_reg_val);
	if(sky_amp_log[4]) printk("[SKY Audio] AMP Shutdown for power-off\n");
}
#endif
#if 1
static const struct platform_device_id max8981_id[] = {
	{ "max8981-amp", 0},
};
#else
static const struct i2c_device_id max8981_id[] = {
	{ "max8981-amp", 0},
};
#endif
#ifdef FEATURE_SKY_SND_FROYO_PATCH
const static struct dev_pm_ops i2c_device_max8981_pm_ops = {
             .suspend = max8981_suspend,
             .resume = max8981_resume,
};
#endif
#if 1
static struct platform_driver max8981_driver = {
             .id_table = max8981_id,
             .probe  = max8981_probe,
             .remove = __exit_p(max8981_remove),
#ifndef FEATURE_SKY_SND_FROYO_PATCH
             .suspend = max8981_suspend,
             .resume = max8981_resume,
#endif
             .shutdown = max8981_shutdown,
             .driver = {
                          .name = "max8981-amp",             
                          .pm = &i2c_device_max8981_pm_ops,
             },
};
#else
static struct i2c_driver max8981_driver = {
             .id_table = max8981_id,
             .probe  = max8981_probe,
             .remove = __exit_p(max8981_remove),
#ifndef FEATURE_SKY_SND_FROYO_PATCH
             .suspend = max8981_suspend,
             .resume = max8981_resume,
#endif
             .shutdown = max8981_shutdown,
             .driver = {
                          .name = "max8981-amp",             
                          .pm = &i2c_device_max8981_pm_ops,
             },
};
#endif

static int __init pantech_extamp_init(void)
{
	int rc;
	uint32_t ver;

	printk(KERN_ERR "%s is called\n", __func__);

	rc =  platform_driver_register(&max8981_driver);
	printk(KERN_ERR "%s: platform_driver_register rc = %d\n",
			__func__, rc);

	return rc;
}

/*==========================================================================
** snd_extamp_api_Init
**=========================================================================*/
void snd_extamp_api_Init(void)
{
	int result = 0;

	result=misc_register(&miscdev);
	if(result)
	{
		if(sky_amp_log[1]) printk(KERN_ERR "misc_register failed.\n");
	}
	
	if(sky_amp_log[1]) printk(KERN_ERR "[SKY Audio] init max8981\n");
#if 1
//	result = platform_driver_register(&max8981_driver);
#else
	result = i2c_add_driver(&max8981_driver);
#endif
	if(result){
		if(sky_amp_log[1]) printk(KERN_ERR "[SKY Audio] init max8981 Fail\n");
	}
}

/*==========================================================================
** snd_extamp_api_DeInit
**=========================================================================*/
void snd_extamp_api_DeInit(void)
{
	misc_deregister(&miscdev);
#if 1
	platform_driver_unregister(&max8981_driver);
#else
	i2c_del_driver(&max8981_driver);
#endif
}

/*==========================================================================
** snd_amp_external_control
**=========================================================================*/

int testAMPgainCTL[3]={0,0,0};

void snd_amp_external_control(int which_amp, int value)
{
	int fd;
	int testtemp;
    if(sky_amp_log[0]) printk(KERN_ERR "[SKY Audio] snd_amp_external_control (which_amp:%d, value:%d)\n", which_amp, value);

	switch (which_amp)
	{
		case 0:  // preAMP
			test_preAMPgain=value;
			testAMPgainCTL[0]=1;
			break;
		case 1:  // spkAMP
			test_spkAMPgain=value;
			testAMPgainCTL[1]=1;
			break;
		case 2:  // hpAMP
			test_hpAMPgain=value;
			testAMPgainCTL[2]=1;
			break;
		default:
			break;
	}

    snd_extamp_api_SetDevice(1, CurrDeviceId);
}
/*==========================================================================
** snd_extamp_write_all_reg
**=========================================================================*/
int DELAY_ON = 100;
int test_delay[10]={0,0,0,0,0,0,0,0,0,0};
int snd_extamp_write_all_reg(extamp_info_t tCurrentExtampInfo)
{
	int i;
	int status[9] = {0,0,0,0,0,0,0,0,0};
       int write_status=0;
       
      //const u8 DEALY_DN = 15; /*ms*/

       if(TestStaticExtampEnable) {
            memcpy(&tCurrentExtampInfo, &TestExtampInfo, sizeof(extamp_info_t));
       }

       if(testAMPgainCTL[0])   {
            tCurrentExtampInfo.input_mode_ctl_reg_val = tCurrentExtampInfo.input_mode_ctl_reg_val & 0xf0;
            tCurrentExtampInfo.input_mode_ctl_reg_val = tCurrentExtampInfo.input_mode_ctl_reg_val | test_preAMPgain;
            tCurrentExtampInfo.input_mode_ctl_reg_val = tCurrentExtampInfo.input_mode_ctl_reg_val | (test_preAMPgain<<2);
            TestExtampEnable[0]=1;
       }
       if(testAMPgainCTL[1])   {
            tCurrentExtampInfo.sp_ctl_reg_val = tCurrentExtampInfo.sp_ctl_reg_val & 0x00;
            tCurrentExtampInfo.sp_ctl_reg_val = tCurrentExtampInfo.sp_ctl_reg_val | test_spkAMPgain;
            TestExtampEnable[5]=1;
       }
       if(testAMPgainCTL[2])   {
            tCurrentExtampInfo.hpl_ctl_reg_val = tCurrentExtampInfo.hpl_ctl_reg_val & 0x00;
            tCurrentExtampInfo.hpl_ctl_reg_val = tCurrentExtampInfo.hpl_ctl_reg_val | test_hpAMPgain;
            TestExtampEnable[3]=1;

			tCurrentExtampInfo.hpr_ctl_reg_val = tCurrentExtampInfo.hpr_ctl_reg_val & 0x00;
            tCurrentExtampInfo.hpr_ctl_reg_val = tCurrentExtampInfo.hpr_ctl_reg_val | test_hpAMPgain;
			TestExtampEnable[4]=1;
       }

    	if(TestExtampEnable[0] || tExtampInfo.input_mode_ctl_reg_val != tCurrentExtampInfo.input_mode_ctl_reg_val)	{
		status[0] = snd_extamp_i2c_write(INPUT_MODE_CTL_REG_ADDR, tCurrentExtampInfo.input_mode_ctl_reg_val);
              //if(sky_amp_log[0]) printk(KERN_ERR "[SKY Audio] snd_extamp_write_all_reg (input_mode_ctl_reg_val:%d)\n", tCurrentExtampInfo.input_mode_ctl_reg_val); // TEMP
              write_status |= 0x1;
	}
	if(TestExtampEnable[3] || tExtampInfo.hpl_ctl_reg_val != tCurrentExtampInfo.hpl_ctl_reg_val)	{
		status[3] = snd_extamp_i2c_write(HEADPHONE_LEFT_CTL_REG_ADDR, tCurrentExtampInfo.hpl_ctl_reg_val);
              //if(sky_amp_log[0]) printk(KERN_ERR "[SKY Audio] snd_extamp_write_all_reg (hpl_ctl_reg_val:%d)\n", tCurrentExtampInfo.hpl_ctl_reg_val); // TEMP
              write_status |= 0x8;
	}
	if(TestExtampEnable[4] || tExtampInfo.hpr_ctl_reg_val != tCurrentExtampInfo.hpr_ctl_reg_val)	{
		status[4] = snd_extamp_i2c_write(HEADPHONE_RIGHT_CTL_REG_ADDR, tCurrentExtampInfo.hpr_ctl_reg_val);
              write_status |= 0x10;
	}
	if(TestExtampEnable[5] || tExtampInfo.sp_ctl_reg_val != tCurrentExtampInfo.sp_ctl_reg_val)	{
		status[5] = snd_extamp_i2c_write(SPEAKER_VOL_CTL_REG_ADDR, tCurrentExtampInfo.sp_ctl_reg_val  );
              //if(sky_amp_log[0]) printk(KERN_ERR "[SKY Audio] snd_extamp_write_all_reg (sp_ctl_reg_val:%d)\n", tCurrentExtampInfo.sp_ctl_reg_val); // TEMP
              write_status |= 0x20;
	}
	if(TestExtampEnable[7] || tExtampInfo.output_mode_ctl_reg_val != tCurrentExtampInfo.output_mode_ctl_reg_val)	{
		status[7] = snd_extamp_i2c_write(OUTPUT_MODE_CTL_REG_ADDR, tCurrentExtampInfo.output_mode_ctl_reg_val);
              //if(sky_amp_log[0]) printk(KERN_ERR "[SKY Audio] snd_extamp_write_all_reg (output_mode_ctl_reg_val:%d)\n", tCurrentExtampInfo.output_mode_ctl_reg_val); // TEMP
              write_status |= 0x80;
	}

	memcpy(&tExtampInfo, &tCurrentExtampInfo, sizeof(extamp_info_t));
	for(i=0;i<9;i++)	{
		if(status[i])	{
                    if(sky_amp_log[1]) printk(KERN_ERR "[SKY Audio] snd_extamp_api_SetDevice Fail (%d Reg.)\n", i);
                    return 0;
		}
	}

       if(write_status!=0)  {
       if(sky_amp_log[0]) printk(KERN_ERR "[SKY Audio] snd_extamp_write_all_reg (Write Status:%x)(g_call_status=%d)\n", write_status, g_call_status);
        return 1;
       }
       else {
        if(sky_amp_log[0]) printk(KERN_ERR "[SKY Audio] snd_extamp_write_all_reg (No change) (g_call_status=%d) \n", g_call_status);
        return 0;
       }
}
/*==========================================================================
** snd_extamp_make_default
**=========================================================================*/
void snd_extamp_make_default(extamp_info_t* tCurrentExtampInfo)
{
	tCurrentExtampInfo->input_mode_ctl_reg_val		= ZCD_EN | INADIFF_STEREO_SINGLE | INBDIFF_DIFFERENTIAL | PGAINA_0_DB | PGAINB_5p5_DB/*PGAINB_0_DB*/;
	tCurrentExtampInfo->sp_ctl_reg_val		= SPKVOL_MUTE;
	tCurrentExtampInfo->hpl_ctl_reg_val			= HPLVOL_MUTE;
	tCurrentExtampInfo->hpr_ctl_reg_val			= HPRVOL_MUTE;
	tCurrentExtampInfo->output_mode_ctl_reg_val			= SHDN_EN | BYPASS_DIS | INB_DIS | INA_DIS | SPKEN_DIS | HPHEN_DIS;
}
/*==========================================================================
** snd_extamp_make_current
**=========================================================================*/
void snd_extamp_make_current(extamp_info_t* tCurrentExtampInfo)
{
    memcpy(tCurrentExtampInfo, &tExtampInfo, sizeof(extamp_info_t));
}
/*==========================================================================
** snd_extamp_api_SetDevice
**=========================================================================*/
#define FEATURE_SND_SKY_AMP_TEST
#ifdef FEATURE_SND_SKY_AMP_TEST
int test_set_device[2]={0,6};
#endif
static int noise_critical_point=1;
void snd_extamp_api_SetDevice(int on, uint32_t cad_device)
{
	extamp_info_t tCurrentExtampInfo;
       int result;

#ifdef FEATURE_SND_SKY_AMP_TEST
       if(test_set_device[0])   {
            on=1;
            cad_device=test_set_device[1];
       }
       if(!on)  {
		   if(g_call_status || loopback_test 
#ifdef FEATURE_SKY_SND_AUTOANSWER
			   || auto_answering_status
#endif
			   )   {
	#ifdef FEATURE_SKY_SND_AUTOANSWER
			   if(sky_amp_log[0]) printk(KERN_ERR "[SKY Audio] snd_extamp_api_SetDevice skipped (%d,%d, %d),\n",g_call_status, loopback_test, auto_answering_status); 
	#else
			   if(sky_amp_log[0]) printk(KERN_ERR "[SKY Audio] snd_extamp_api_SetDevice skipped (%d,%d),\n",g_call_status, loopback_test); 
	#endif
                return;
            }
       }
       else {
            if(loopback_test)   {
                if(sky_amp_log[0]) printk(KERN_ERR "[SKY Audio] snd_extamp_api_SetDefaultVolume skipped (%d,%d),\n",g_call_status, loopback_test); 
                return;
            }
       }
#endif
       if(cad_device == SND_DEVICE_CURRENT) cad_device = CurrDeviceId;

       if( CurrDeviceId!=cad_device)    {
    	    CurrDeviceId = cad_device;
           #ifdef FEATURE_SKY_SND_DUAL_PATH_REDIFINITION
           noise_critical_point=0;
           #else
           noise_critical_point=1;
           #endif
       }
       if(sky_amp_log[2]) printk(KERN_ERR "[SKY Audio] snd_extamp_api_SetDevice(OnOff:%d,Device:%d,CallState:%d)\n", on, cad_device, g_call_status);
       
	snd_extamp_make_current(&tCurrentExtampInfo);

       switch(cad_device)
	{
		case SND_DEVICE_HANDSET:
		case SND_DEVICE_VOIP_HANDSET:
			tCurrentExtampInfo.output_mode_ctl_reg_val = SHDN_DIS | BYPASS_EN | INB_DIS | INA_DIS | SPKEN_DIS | HPHEN_DIS;
			break;
		case SND_DEVICE_HEADSET:
		case SND_DEVICE_STEREO_HEADSET:
        case SND_DEVICE_VOIP_HEADSET:
            tCurrentExtampInfo.input_mode_ctl_reg_val= ZCD_EN | INADIFF_STEREO_SINGLE | PGAINA_0_DB | PGAINB_5p5_DB/*PGAINB_0_DB*/;
			tCurrentExtampInfo.output_mode_ctl_reg_val= SHDN_EN | BYPASS_DIS;
			tCurrentExtampInfo.output_mode_ctl_reg_val |= HPHEN_EN;
			if(on)    
			{
				tCurrentExtampInfo.output_mode_ctl_reg_val |= INA_EN;
			}
			else	 
				tCurrentExtampInfo.output_mode_ctl_reg_val &= ~(INA_EN | INB_EN);
    
#ifdef FEATURE_BOOT_SOUND_PLAY
			if(boot_snd_play>=BOOTSND_SET_PATH)    {
				if(sky_amp_log[1]) printk("[Audio] snd_extamp_api_SetDevice SND_SKY_SET_BOOTSOUND: boot_snd_play=%d \n", boot_snd_play);                        
				tCurrentExtampInfo.input_mode_ctl_reg_val= ZCD_EN | INBDIFF_STEREO_SINGLE | PGAINA_0_DB | PGAINB_5p5_DB/*PGAINB_0_DB*/;

				if(on)    
					tCurrentExtampInfo.output_mode_ctl_reg_val |= INB_EN;
				else   
					tCurrentExtampInfo.output_mode_ctl_reg_val &= ~(INA_EN | INB_EN);
			}
#endif
			break;
		case SND_DEVICE_SPEAKER_PHONE:
		case SND_DEVICE_SPKPHONE:
		case SND_DEVICE_VT_SPK:
		case SND_DEVICE_VOIP_SPK:
				tCurrentExtampInfo.input_mode_ctl_reg_val= ZCD_EN | INBDIFF_DIFFERENTIAL | PGAINA_0_DB | PGAINB_5p5_DB/*PGAINB_0_DB*/;

				if(tCurrentExtampInfo.output_mode_ctl_reg_val & (HPHEN_EN) == (HPHEN_EN))
				{
					// There is pop-up noise when hp pwr is on.
					tCurrentExtampInfo.output_mode_ctl_reg_val= HPHEN_EN;
				}
				tCurrentExtampInfo.output_mode_ctl_reg_val |= SHDN_EN;
				tCurrentExtampInfo.output_mode_ctl_reg_val &= ~(BYPASS_EN);
				if(on)    
				{
					tCurrentExtampInfo.output_mode_ctl_reg_val |= SPKEN_EN;
					tCurrentExtampInfo.output_mode_ctl_reg_val |= INB_EN;			
				}
				else   
					tCurrentExtampInfo.output_mode_ctl_reg_val &= ~(INA_EN | INB_EN);
			break;
		case SND_DEVICE_HEADSET_AND_SPEAKER:
                     #ifdef FEATURE_SKY_SND_DUAL_PATH_DEFAULT_SPEAKER
                     tCurrentExtampInfo.input_mode_ctl_reg_val= ZCD_EN | INBDIFF_DIFFERENTIAL | PGAINA_0_DB | PGAINB_5p5_DB/*PGAINB_0_DB*/;
                     #if 0//def FEATURE_SKY_SND_DUAL_PATH_REDIFINITION	//dual path speak vol QE fix
                     tCurrentExtampInfo.input_mode_ctl_reg_val |= INADIFF_STEREO_SINGLE;
                     #endif
			tCurrentExtampInfo.output_mode_ctl_reg_val= SHDN_EN  | BYPASS_DIS;
                     tCurrentExtampInfo.output_mode_ctl_reg_val |= HPHEN_EN;
                     if(on)    
                     {
			 tCurrentExtampInfo.output_mode_ctl_reg_val |= SPKEN_EN;
                        #if 0//def FEATURE_SKY_SND_DUAL_PATH_REDIFINITION	//dual path speak vol QE fix
                        tCurrentExtampInfo.output_mode_ctl_reg_val |= INA_EN;
                        #else
                        tCurrentExtampInfo.output_mode_ctl_reg_val |= INB_EN;
                        #endif
                        tCurrentExtampInfo.output_mode_ctl_reg_val |= INB_EN;

                     }
                     else   {
                        tCurrentExtampInfo.output_mode_ctl_reg_val &= ~(INA_EN | INB_EN);
                        tCurrentExtampInfo.output_mode_ctl_reg_val &= ~(INA_EN | INB_EN);
                     }
                     #else /* NOT FEATURE_SKY_SND_DUAL_PATH_DEFAULT_SPEAKER */
                     #ifdef FEATURE_SKY_SND_INCALL_CAL_SEPERATION
                     tCurrentExtampInfo.input_mode_ctl_reg_val= ZCD_EN | INBDIFF_STEREO_SINGLE | PGAINA_0_DB | PGAINB_5p5_DB/*PGAINB_0_DB*/;
                     #else
                     tCurrentExtampInfo.input_mode_ctl_reg_val= ZCD_EN | INADIFF_STEREO_SINGLE | PGAINA_0_DB | PGAINB_5p5_DB/*PGAINB_0_DB*/;
                     #endif

			tCurrentExtampInfo.output_mode_ctl_reg_val= SHDN_EN  | BYPASS_DIS;
                     tCurrentExtampInfo.output_mode_ctl_reg_val |= HPHEN_EN;				 
                     if(on)    
                     {
			 tCurrentExtampInfo.output_mode_ctl_reg_val |= SPKEN_EN;
                        #ifdef FEATURE_SKY_SND_INCALL_CAL_SEPERATION
                         tCurrentExtampInfo.output_mode_ctl_reg_val |= INB_EN;
                        #else
                         tCurrentExtampInfo.output_mode_ctl_reg_val |= INA_EN;
                        #endif
                        tCurrentExtampInfo.output_mode_ctl_reg_val |= INB_EN;
                     }
                     else   {
                        tCurrentExtampInfo.output_mode_ctl_reg_val &= ~(INA_EN | INB_EN);
                        tCurrentExtampInfo.output_mode_ctl_reg_val &= ~(INA_EN | INB_EN);
                     }
                     #endif  /* NOT FEATURE_SKY_SND_DUAL_PATH_DEFAULT_SPEAKER */
			break;

		default:
			break;
	}
	result = snd_extamp_write_all_reg(tCurrentExtampInfo); 
       if(on && result && noise_critical_point) {
            if(sky_amp_log[1]) printk(KERN_ERR "[SKY Audio] snd_extamp_api_time_delay start(Delay:%d)(Callstate:%d)\n", DELAY_ON, g_call_status);
            msleep(DELAY_ON);
       }
       //printk(KERN_ERR "[SKY Audio] snd_extamp_api_SetDevice [1:Set noise_critical_point=%d]\n", noise_critical_point);
       noise_critical_point = 0;
       //printk(KERN_ERR "[SKY Audio] snd_extamp_api_SetDevice [2:Set noise_critical_point=0]\n");

}

/*==========================================================================
** snd_extamp_api_SetDefaultVolume
**=========================================================================*/
int SET_VOLUME_DELAY[2]={1,10};
int ext_cad_device_vol[2]={-1,-1};
void snd_extamp_api_SetDefaultVolume(uint32_t cad_device, int unmute)
{
       int current_vol;

       if(vt_mute_condition)    {
           if(g_call_status || current_mode_param[0]==1/* VT */)    {
                if(unmute)  {
                    if(sky_amp_log[2]) printk(KERN_ERR "[SKY Audio] snd_extamp_api_SetDefaultVolume forced to mute. (before:%d)\n", unmute);
                    unmute=0;   // SangwonLee 101012 VT mute critical section.
                }
           }
           else {
                if(sky_amp_log[2]) printk(KERN_ERR "[SKY Audio] snd_extamp_api_SetDefaultVolume vt_mute_condition reset.\n");
                vt_mute_condition=false;
           }
       }
       
       if(sky_amp_log[2]) printk(KERN_ERR "[SKY Audio] snd_extamp_api_SetDefaultVolume (device:%d,unmute:%d,call_state:%d,cur_mode:%d)\n", cad_device, unmute, g_call_status, current_mode_param[0]);
	if(cad_device!=SND_DEVICE_CURRENT) {
    	    CurrDeviceId = cad_device;
	}
       else  cad_device = CurrDeviceId;

#ifdef FEATURE_SND_SKY_AMP_TEST       
       if(test_set_device[0])   {
            unmute=1;
            cad_device=test_set_device[1];
       }
#endif       
       if(!unmute)  {
		   if((g_call_status && !vt_mute_condition) || loopback_test
#ifdef FEATURE_SKY_SND_AUTOANSWER
			   || auto_answering_status
#endif
		   )   {
	#ifdef FEATURE_SKY_SND_AUTOANSWER
			   if(sky_amp_log[0]) printk(KERN_ERR "[SKY Audio] snd_extamp_api_SetDefaultVolume skipped (%d,%d, %d),\n",g_call_status, loopback_test, auto_answering_status); 
	#else
			   if(sky_amp_log[0]) printk(KERN_ERR "[SKY Audio] snd_extamp_api_SetDefaultVolume skipped (%d,%d),\n",g_call_status, loopback_test); 
	#endif
                return;
            }
       }
       else {
            if(loopback_test)   {
                if(sky_amp_log[0]) printk(KERN_ERR "[SKY Audio] snd_extamp_api_SetDefaultVolume skipped (%d,%d),\n",g_call_status, loopback_test); 
                return;
            }
       }
//       if(unmute && SET_VOLUME_DELAY[0])   msleep(SET_VOLUME_DELAY[1]);
/* Target Volume Set */
       switch(cad_device)
	{
		case SND_DEVICE_HANDSET:
		case SND_DEVICE_VOIP_HANDSET:
                       current_vol = 0;
                    break;
		case SND_DEVICE_HEADSET:
		case SND_DEVICE_STEREO_HEADSET:
              case SND_DEVICE_VOIP_HEADSET:
                     if(unmute) {
                         if(g_call_status)  {
                            if( current_mode_param[0]==1)   current_vol = HP_VT_INCALL;    // VT
                            else if( current_mode_param[0]==2)  current_vol = HP_VOIP_INCALL;    // VoIP
                            else          current_vol = HP_INCALL;
                         }
                         else   {
                            current_vol = HP_MEDIA;
                         }
                     }
                     else   {
                        current_vol = 0;
                     }
			break;
		case SND_DEVICE_SPEAKER_PHONE:
              case SND_DEVICE_SPKPHONE:
              case SND_DEVICE_VT_SPK:
              case SND_DEVICE_VOIP_SPK:
                     if(unmute) {
                        if(g_call_status)  {
                            if( current_mode_param[0]==1)   {   // VT
                                if( current_mode_param[1]==2)   current_vol = VT_SPKPHONE;  // InCall
                                else    current_vol = VT_CALL_START_GAIN;
                            }
                            else if( current_mode_param[0]==2)   {   // VoIP
                                current_vol = VOIP_SPKPHONE;
                            }
                            else    {
                                current_vol = SP_SPKPHONE;
                            }
                            if(sky_amp_log[2]) printk(KERN_ERR, "SND_DEVICE_SPEAKER_PHONE:%d\n", current_vol); 
                        }
                        else    {
                            current_vol = SP_MEDIA;
                        }
                     }
                     else   {
                        current_vol = 0;
                     }
			break;
        	case SND_DEVICE_HEADSET_AND_SPEAKER:
                     if(unmute) {
                        current_vol = HP_DUAL_MEDIA; //pz1693_110310
                     }
                     else   {
                        current_vol = 0;
                     }
                     snd_extamp_api_SetVolume(current_vol, 1);
                     if(unmute) {
                        current_vol = SP_MEDIA;
                     }
                     else   {
                        current_vol = 0;
                     }
                     break;
		default:
			break;
	}

       snd_extamp_api_SetVolume(current_vol, 0);   /* Target Volume Set */
       CurrCallMode = current_mode_param[0];
       ext_cad_device_vol[0] = cad_device;
       ext_cad_device_vol[1] = current_vol; 
}

/*==========================================================================
** snd_extamp_api_SetVolume
**=========================================================================*/
void snd_extamp_api_SetVolume(uint32_t master_volume, int param)
{
	extamp_info_t tCurrentExtampInfo;
       if(sky_amp_log[2]) printk(KERN_ERR "[SKY Audio] snd_extamp_api_SetVolume(Device:%d, vol:%d, mode:%d)\n", CurrDeviceId, master_volume, current_mode_param[0]);    
	snd_extamp_make_current(&tCurrentExtampInfo);

	switch(CurrDeviceId)
	{
		case SND_DEVICE_HANDSET:
		case SND_DEVICE_VOIP_HANDSET:
                        tCurrentExtampInfo.input_mode_ctl_reg_val= ZCD_EN | PGAINA_0_DB | PGAINB_5p5_DB/*PGAINB_0_DB*/;
        		   tCurrentExtampInfo.sp_ctl_reg_val= SPKVOL_MUTE;
    			   tCurrentExtampInfo.hpl_ctl_reg_val= HPLVOL_MUTE;
                        tCurrentExtampInfo.hpr_ctl_reg_val= HPRVOL_MUTE;
                    break;
		case SND_DEVICE_HEADSET:
		case SND_DEVICE_STEREO_HEADSET:
              case SND_DEVICE_VOIP_HEADSET:
                     if(master_volume==0)   {
                        tCurrentExtampInfo.input_mode_ctl_reg_val= ZCD_EN | PGAINA_0_DB | PGAINB_5p5_DB/*PGAINB_0_DB*/;
     			   tCurrentExtampInfo.hpl_ctl_reg_val= HPLVOL_MUTE;
                        tCurrentExtampInfo.hpr_ctl_reg_val= HPRVOL_MUTE;
                     }
                     else   {
                        tCurrentExtampInfo.input_mode_ctl_reg_val= ZCD_EN | INADIFF_STEREO_SINGLE |  PGAINA_0_DB | PGAINB_5p5_DB/*PGAINB_0_DB*/;
    			   tCurrentExtampInfo.hpl_ctl_reg_val= master_volume;
                        tCurrentExtampInfo.hpr_ctl_reg_val= master_volume;
                     }
                     tCurrentExtampInfo.sp_ctl_reg_val= SPKVOL_MUTE;
			break;
		case SND_DEVICE_SPEAKER_PHONE:
              case SND_DEVICE_SPKPHONE:
              case SND_DEVICE_VT_SPK:
              case SND_DEVICE_VOIP_SPK:
                     if(master_volume==0)   {
                        tCurrentExtampInfo.input_mode_ctl_reg_val= ZCD_EN | PGAINA_0_DB | PGAINB_5p5_DB/*PGAINB_0_DB*/;
                        tCurrentExtampInfo.sp_ctl_reg_val= SPKVOL_MUTE;
                     }
                     else   {
                            #ifndef FEATURE_SKY_SND_CAL_SEGMENT
                            if( g_call_status && (current_mode_param[0]==1 || current_mode_param[0]==2))   {   // VT || VoIP
                                tCurrentExtampInfo.input_mode_ctl_reg_val= ZCD_EN | INBDIFF_DIFFERENTIAL | PGAINA_0_DB | PGAINB_5p5_DB/*PGAINB_0_DB*/;
             			    tCurrentExtampInfo.sp_ctl_reg_val= master_volume;
                                if(sky_amp_log[0]) printk(KERN_ERR "[SKY Audio] snd_extamp_api_SetVolume - current_mode_param:VT\n");  
                            }
                            else    
                            #endif
                            {
                                tCurrentExtampInfo.input_mode_ctl_reg_val= ZCD_EN | INBDIFF_DIFFERENTIAL | PGAINA_0_DB | PGAINB_5p5_DB/*PGAINB_0_DB*/;
                                tCurrentExtampInfo.sp_ctl_reg_val= master_volume;
                            }
                     }
                     tCurrentExtampInfo.hpl_ctl_reg_val= HPLVOL_MUTE;
                     tCurrentExtampInfo.hpr_ctl_reg_val= HPRVOL_MUTE;
			break;
              case SND_DEVICE_HEADSET_AND_SPEAKER:
                     if(master_volume==0)   {
                        tCurrentExtampInfo.input_mode_ctl_reg_val= ZCD_EN | PGAINA_0_DB | PGAINB_5p5_DB/*PGAINB_0_DB*/;
        		   tCurrentExtampInfo.sp_ctl_reg_val= SPKVOL_MUTE;
    			   tCurrentExtampInfo.hpl_ctl_reg_val= HPLVOL_MUTE;
                        tCurrentExtampInfo.hpr_ctl_reg_val= HPRVOL_MUTE;
                     }
                     else   {
                        #ifdef FEATURE_SKY_SND_DUAL_PATH_DEFAULT_SPEAKER
                        tCurrentExtampInfo.input_mode_ctl_reg_val= ZCD_EN | INBDIFF_DIFFERENTIAL | PGAINA_0_DB | PGAINB_5p5_DB/*PGAINB_0_DB*/;

                        #if 0//def FEATURE_SKY_SND_DUAL_PATH_REDIFINITION	//dual path speak vol QE fix
                        tCurrentExtampInfo.input_mode_ctl_reg_val |= INADIFF_STEREO_SINGLE;
                        #endif
                        #else
                        tCurrentExtampInfo.input_mode_ctl_reg_val= ZCD_EN | INADIFF_STEREO_SINGLE | PGAINA_0_DB | PGAINB_5p5_DB/*PGAINB_0_DB*/;
                        #endif
                        if(!param)    {
              		tCurrentExtampInfo.sp_ctl_reg_val= master_volume;
                        }
                        else    {
      			       tCurrentExtampInfo.hpl_ctl_reg_val= master_volume;
                            tCurrentExtampInfo.hpr_ctl_reg_val= master_volume;
                        }
                     }
                     break;
		default:
			break;
	}
	snd_extamp_write_all_reg(tCurrentExtampInfo);
}

/*==========================================================================
** snd_extamp_api_Snd_SetDevice
**=========================================================================*/
int snd_extamp_GetCurrentDevice(void)
{
    return CurrDeviceId;
}
/*==========================================================================
** snd_extamp_api_Snd_SetDevice
**=========================================================================*/
void snd_extamp_api_Snd_SetDevice(uint32_t cad_device, int ear_mute, int mic_mute)
{
    extern int g_audio_enable;

#ifdef FEATURE_SND_SKY_AMP_TEST       
       if(test_set_device[0])   {
            cad_device=test_set_device[1];
       }
#endif       

    if(setAMP_exception) {
        if(!current_mode_param[0] && cad_device==SND_DEVICE_SPEAKER_PHONE)    {
                cad_device=SND_DEVICE_HEADSET_AND_SPEAKER;
                if(sky_amp_log[2])  {
                    printk(KERN_ERR "[SKY Audio] (0)snd_extamp_api_Snd_SetDevice - Device is changed from %d to %d\n", SND_DEVICE_SPEAKER_PHONE, SND_DEVICE_HEADSET_AND_SPEAKER);                                  
                }
        }
    }

    if(sky_amp_log[2]) 
        printk(KERN_ERR "[SKY Audio] snd_extamp_api_Snd_SetDevice - cad_device:%d, g_call_status:%d, g_audio_enable:%d, loopback_test:%d, ear_mute:%d\n",
        cad_device, g_call_status, g_audio_enable, loopback_test, ear_mute);  

    if(!loopback_test && (cad_device==SND_DEVICE_CURRENT || CurrDeviceId==cad_device) && CurrCallMode==current_mode_param[0] && !forced_mute)
    {
        if(sky_amp_log[2])  {
            printk(KERN_ERR "[SKY Audio] (1)snd_extamp_api_Snd_SetDevice skip! - CurrDeviceId:%d, cad_device:%d\n", CurrDeviceId, cad_device);  
            printk(KERN_ERR "[SKY Audio] (2)snd_extamp_api_Snd_SetDevice skip! - CurrCallMode:%d, current_mode_param[2]:%d\n", CurrCallMode, current_mode_param[0]);  
        }
        return;
    }
    
    forced_mute = false;    // SangwonLee 101013 Earjack call mute bug fix when silent mode.
    
    if(g_call_status)   {
            snd_extamp_api_SetDevice(1, cad_device);
            snd_extamp_api_SetDefaultVolume(cad_device, 1);
    }
    else if(g_audio_enable || loopback_test)   {
            {
                snd_extamp_api_SetDevice(1, cad_device);
                snd_extamp_api_SetDefaultVolume(cad_device, 1);
            }
    }
    else   {
        if(cad_device == SND_DEVICE_CURRENT) cad_device = CurrDeviceId;

        if( CurrDeviceId!=cad_device)    {
            CurrDeviceId = cad_device;
           #ifdef FEATURE_SKY_SND_DUAL_PATH_REDIFINITION
            noise_critical_point=0;
           #else
            noise_critical_point=1;     // device was changed.
           #endif
        }
        if(sky_amp_log[2])  printk(KERN_ERR "[SKY Audio] snd_extamp_api_Snd_SetDevice skip!(update_device) - CurrDeviceId:%d\n", CurrDeviceId);  
    }
}
/*==========================================================================
** snd_extamp_sleep
**=========================================================================*/
/*static*/ void snd_extamp_sleep(u8 enable)
{
	unsigned char SleepOk = 0;
       unsigned char spk_vol, hpl_vol, hpr_vol,ctl_val;
#if 1
				  if(tExtampInfo.sp_ctl_reg_val == SPKVOL_MUTE){
					  spk_vol = 1;
					  }
				  else{
					  spk_vol = 0;
					  }
				  if(tExtampInfo.hpl_ctl_reg_val == HPLVOL_MUTE){
					  hpl_vol = 1;
					  }
				  else{
					  hpl_vol = 0;
					  }
				  if(tExtampInfo.hpr_ctl_reg_val == HPRVOL_MUTE){
					  hpr_vol = 1;
					  }
				  else{
					  hpr_vol = 0;
					  }
				  if(tExtampInfo.output_mode_ctl_reg_val == (SHDN_DIS | BYPASS_EN | SPKEN_DIS | HPHEN_DIS)){
					  ctl_val = 1;
					  }
				  else{
					  ctl_val = 0;
					  }
#else
       spk_vol = tExtampInfo.sp_ctl_reg_val & SPKVOL_MUTE ? 1:0;
       hpl_vol = tExtampInfo.hpl_ctl_reg_val & HPLVOL_MUTE ? 1:0;
       hpr_vol = tExtampInfo.hpr_ctl_reg_val & HPRVOL_MUTE ? 1:0;
#endif
	if(sky_amp_log[1])        printk("[SKY Audio] snd_extamp_sleep (%d)(%d)(%d)\n", spk_vol, hpl_vol, hpr_vol);
    
       if(!g_call_status)   {
            if(!spk_vol)     {
                tExtampInfo.sp_ctl_reg_val = SPKVOL_MUTE;
                snd_extamp_i2c_write(SPEAKER_VOL_CTL_REG_ADDR, tExtampInfo.sp_ctl_reg_val);
                spk_vol = 1;
            }
            if(!hpl_vol)     {
                tExtampInfo.hpl_ctl_reg_val = HPLVOL_MUTE;
                snd_extamp_i2c_write(HEADPHONE_LEFT_CTL_REG_ADDR, tExtampInfo.hpl_ctl_reg_val);
                hpl_vol = 1;
            }
            if(!hpr_vol)     {
                tExtampInfo.hpr_ctl_reg_val = HPRVOL_MUTE;
                snd_extamp_i2c_write(HEADPHONE_RIGHT_CTL_REG_ADDR, tExtampInfo.hpr_ctl_reg_val);
                hpr_vol = 1;
            }
       }
	
	SleepOk = enable & spk_vol & hpl_vol & hpr_vol;

       if(sky_amp_log[0])       printk("[SKY Audio] AMP Sleep (sp_ctl_reg_val:%d)\n", spk_vol);
#if 1
	if((SleepOk) && (!ctl_val))
#else	   
       if(SleepOk)
#endif	   	
	{
            tExtampInfo.output_mode_ctl_reg_val = SHDN_DIS | BYPASS_EN | SPKEN_DIS | HPHEN_DIS;
 	     snd_extamp_i2c_write(OUTPUT_MODE_CTL_REG_ADDR, tExtampInfo.output_mode_ctl_reg_val);
       }
       #ifdef FEATURE_SKY_SND_DUAL_PATH_REDIFINITION
       noise_critical_point=0;
       #else
       noise_critical_point=1;
       #endif

	if(sky_amp_log[1])       printk("[SKY Audio] AMP Sleep(%d)\n", SleepOk);
}
/*==========================================================================
** snd_extamp_i2c_write
**=========================================================================*/
#if 1//jks_test
#include "../proc_comm.h"
#endif
static int snd_extamp_i2c_write(u8 reg, u8 data)
{
#if 1//jks_test
	unsigned cmd = SMEM_PROC_COMM_OEM_CMD1_AUDIO_TEST;
	static int ret = 0;
	unsigned char buf[2];

	buf[0] = reg;
	buf[1] = data;

	ret = msm_proc_comm(PCOM_CUSTOMER_CMD1,&cmd,buf);

	printk("[audio test]snd_extamp_i2c_write - ret(%d),reg(0x%x),data(0x%x)\n",ret,reg,data);
	
	if (ret < 0) {
		return -1;
	}

	return 0;
#else
	static int ret = 0;
	unsigned char buf[2];
	struct i2c_msg msg[1];

	if(!max8981_i2c_client){
		return -1;
	}

	buf[0] = (unsigned char)reg;
	buf[1] = (unsigned char)data;

	msg[0].addr = max8981_i2c_client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = buf;
	
	ret = i2c_transfer(max8981_i2c_client->adapter, msg, 1);
	if (ret < 0) {
		return -1;
	}

	return 0;
#endif	
}

/*==========================================================================
** snd_extamp_i2c_read
**=========================================================================*/
/*static int snd_extamp_i2c_read(u8 reg, u8 *data)
{
	static int ret = 0;
	unsigned char buf[1];
	struct i2c_msg msgs[2];

	if(!max8981_i2c_client){
		return -1;
	}

	buf[0] = reg;

	msgs[0].addr = max8981_i2c_client->addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = buf;

	msgs[1].addr = max8981_i2c_client->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = 1;
	msgs[1].buf = buf;

	ret = i2c_transfer(max8981_i2c_client->adapter, msgs, 2);
	if ( ret < 0) {
		return -1;
	}

	*data = (u8)buf[0];
	return 0;
}*/
late_initcall(pantech_extamp_init);	// 7th stage
		
MODULE_AUTHOR("Jinwoo Cha <jwcha@pantech.com>");
MODULE_DESCRIPTION("pantech extamp driver");
MODULE_LICENSE("GPL");
