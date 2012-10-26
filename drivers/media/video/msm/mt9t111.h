/*
================================================================================
Camera Interface Device Driver for MICRON MT9T111 CMOS 3.1MP sensor

DESCRIPTION
    This file contains the definitions needed for the camera interface
    device driver.

Copyright (c) 2007 by PANTECH, Incorporated. All Rights Reserved.

convert from camsensor_mt9t111_tuneup_sp39s.txt
================================================================================
*/

#ifndef __CAMSENSOR_MT9T111_H__
#define __CAMSENSOR_MT9T111_H__

#ifndef F_SKYCAM_TUP_LOAD_FILE
//튜닝용 바이너리에서는 burst모드 사용 안하기로 함
#define CAMSENSOR_BURSTMODE  //you4me20110127
#endif

#include <linux/types.h>
#include <mach/camera.h>


#define MT9T111_CFG_BRIGHT_MAX_PARAMS 			2
#define MT9T111_CFG_WB_MAX_PARAMS 				15
#define MT9T111_CFG_EFFECT_MAX_PARAMS 			16
#define MT9T111_CFG_EXPOSURE_MAX_PARAMS 		10
#define MT9T111_CFG_PREVIEW_FPS_MAX_PARAMS 	8
#define MT9T111_CFG_REFLECT_MAX_PARAMS 			8
#define MT9T111_CFG_FLICKER_MAX_PARAMS 			6 
#define MT9T111_CFG_ANTISHAKE_MAX_PARAMS 		6
#define MT9T111_CFG_FOCUS_STEP_MAX_PARAMS 		6
//#define MT9T111_CFG_AUTOFOCUS_TRIGGER_MAX_PARAMS 		19
//#define MT9T111_CFG_SCENE_MODE_MAX_PARAMS 		80


extern struct mt9t111_reg mt9t111_regs;



enum mt9t111_width {
	WORD_LEN,
	TRIPLE_LEN,
	BYTE_LEN,	
	ZERO_LEN,
	BURST_WORD_START,
	BURST_WORD_END,
	POLL_MCU_VAR,
	POLL_MCU_VAR_NOT // polling을 빠져나오는 조건이 정해준 값과 같지 않을 경우 100818_psj
};

#ifdef CAMSENSOR_BURSTMODE //you4me20110127 burst
struct mt9t111_i2c_burst_reg_addr {
	unsigned short waddr;
	unsigned short wdata;
};

struct mt9t111_i2c_burst_reg_data {
	unsigned short waddr;
	unsigned short wdata[8];
	unsigned short width;
};

struct mt9t111_i2c_burst_set {
	struct mt9t111_i2c_burst_reg_addr burst_addr;
	struct mt9t111_i2c_burst_reg_data burst_data;
};
#endif

struct mt9t111_i2c_reg_conf {
	unsigned short waddr;
	unsigned short wdata;
	enum mt9t111_width width;
	unsigned short mdelay_time;
};

#ifndef F_SKYCAM_TUP_LOAD_FILE
 typedef enum {
    MT9T111_CFG_WB_AUTO,
    MT9T111_CFG_WB_CUSTOM,
    MT9T111_CFG_WB_INCANDESCENT,
    MT9T111_CFG_WB_FLUORESCENT,
    MT9T111_CFG_WB_DAYLIGHT,
    MT9T111_CFG_WB_CLOUDY,
    MT9T111_CFG_WB_MAX
} MT9T111_cfg_wb_e;
 
typedef enum {
    MT9T111_CFG_BRIGHT_M5,
    MT9T111_CFG_BRIGHT_M4,
    MT9T111_CFG_BRIGHT_M3,
    MT9T111_CFG_BRIGHT_M2,
    MT9T111_CFG_BRIGHT_M1,
    MT9T111_CFG_BRIGHT_0,
    MT9T111_CFG_BRIGHT_P1,
    MT9T111_CFG_BRIGHT_P2,
    MT9T111_CFG_BRIGHT_P3,
    MT9T111_CFG_BRIGHT_P4,
    MT9T111_CFG_BRIGHT_P5,
    MT9T111_CFG_BRIGHT_MAX
} MT9T111_cfg_bright_e;

 typedef enum {
    MT9T111_CFG_EXPOSURE_NORMAL,
    MT9T111_CFG_EXPOSURE_CENTER,
    MT9T111_CFG_EXPOSURE_AVERAGE,
    MT9T111_CFG_EXPOSURE_SPOT,
    MT9T111_CFG_EXPOSURE_MAX
} MT9T111_cfg_exposure_e;

#ifdef F_SKYCAM_FIX_CFG_REFLECT
typedef enum {
    MT9T111_CFG_REFLECT_NONE,
    MT9T111_CFG_REFLECT_MIRROR,
    MT9T111_CFG_REFLECT_WATER,
    MT9T111_CFG_REFLECT_MIRROR_WATER,
    MT9T111_CFG_REFLECT_MAX
} MT9T111_cfg_reflect_e;
#endif

typedef enum {
    MT9T111_CFG_EFFECT_NONE,
    MT9T111_CFG_EFFECT_GRAY,
    MT9T111_CFG_EFFECT_NEGATIVE,
    MT9T111_CFG_EFFECT_SOLARIZE_1,    
    MT9T111_CFG_EFFECT_SEPIA,
    MT9T111_CFG_EFFECT_AQUA,
    MT9T111_CFG_EFFECT_MAX
} MT9T111_cfg_effect_e;

 typedef enum {
    MT9T111_CFG_FOCUS_STEP_0,
    MT9T111_CFG_FOCUS_STEP_1,
    MT9T111_CFG_FOCUS_STEP_2,
    MT9T111_CFG_FOCUS_STEP_3,
    MT9T111_CFG_FOCUS_STEP_4,
    MT9T111_CFG_FOCUS_STEP_5,
    MT9T111_CFG_FOCUS_STEP_6,
    MT9T111_CFG_FOCUS_STEP_7,
    MT9T111_CFG_FOCUS_STEP_8,
    MT9T111_CFG_FOCUS_STEP_9,
    MT9T111_CFG_FOCUS_STEP_10,
    MT9T111_CFG_FOCUS_STEP_MAX
} mt9t111_cfg_focus_step_e;

typedef enum {
    MT9T111_CFG_FLICKER_OFF,
    MT9T111_CFG_FLICKER_60HZ,
    MT9T111_CFG_FLICKER_50HZ,
    MT9T111_CFG_FLICKER_AUTO,
    MT9T111_CFG_FLICKER_MAX
} MT9T111_cfg_flicker_e;

typedef enum {
    MT9T111_CFG_ANTISHAKE_OFF,
    MT9T111_CFG_ANTISHAKE_ON,
    MT9T111_CFG_ANTISHAKE_MAX
} MT9T111_cfg_antishake_e;

typedef enum {
    MT9T111_CFG_AUTOFOCUS_TRIGGER_NORMAL,
    MT9T111_CFG_AUTOFOCUS_TRIGGER_MACRO,
    MT9T111_CFG_AUTOFOCUS_TRIGGER_AUTO,
    MT9T111_CFG_AUTOFOCUS_TRIGGER_INFINITY,
    MT9T111_CFG_AUTOFOCUS_TRIGGER_MAX
} MT9T111_cfg_autofocus_trigger_e;

typedef enum {
    MT9T111_CFG_SCENE_MODE_OFF,
    MT9T111_CFG_SCENE_MODE_PORTRAIT,
    MT9T111_CFG_SCENE_MODE_LANDSCAPE,
    MT9T111_CFG_SCENE_MODE_INDOOR,
    MT9T111_CFG_SCENE_MODE_SPORTS,
    MT9T111_CFG_SCENE_MODE_NIGHT,
    MT9T111_CFG_SCENE_MODE_BEACH,
    MT9T111_CFG_SCENE_MODE_WINTER,
    MT9T111_CFG_SCENE_MODE_SUNSET,
    MT9T111_CFG_SCENE_MODE_TEXT,
    MT9T111_CFG_SCENE_MODE_PARTY,
    MT9T111_CFG_SCENE_MODE_MAX,
} MT9T111_cfg_scene_mode_e;
#endif

struct mt9t111_reg {
	const struct mt9t111_i2c_reg_conf *init_settings;
	uint16_t init_settings_size;
#ifdef CAMSENSOR_BURSTMODE
	const struct mt9t111_i2c_burst_set *init_patch_burst_settings;
	uint16_t 	init_patch_burst_settings_size;
#endif
	const struct mt9t111_i2c_reg_conf *preview_cfg_settings;
	uint16_t preview_cfg_settings_size;
	const struct mt9t111_i2c_reg_conf *snapshot_cfg_settings;
	uint16_t snapshot_cfg_settings_size;
	
//effect
	const struct mt9t111_i2c_reg_conf (*effect_cfg_settings)[MT9T111_CFG_EFFECT_MAX_PARAMS];
	uint16_t effect_cfg_settings_size;

//wb	
	const struct mt9t111_i2c_reg_conf (*wb_cfg_settings)[MT9T111_CFG_WB_MAX_PARAMS];
	uint16_t wb_cfg_settings_size;

//brightness	
	const struct mt9t111_i2c_reg_conf (*bright_cfg_settings)[MT9T111_CFG_BRIGHT_MAX_PARAMS];
	uint16_t bright_cfg_settings_size;
	
//exposure	
	const struct mt9t111_i2c_reg_conf (*exposure_cfg_settings)[MT9T111_CFG_EXPOSURE_MAX_PARAMS];
	uint16_t exposure_cfg_settings_size;

//preview frame rate	
	const struct mt9t111_i2c_reg_conf (*preview_fps_cfg_settings)[MT9T111_CFG_PREVIEW_FPS_MAX_PARAMS];
	uint16_t preview_fps_cfg_settings_size;	

//flicker
	const struct mt9t111_i2c_reg_conf (*flicker_cfg_settings)[MT9T111_CFG_FLICKER_MAX_PARAMS];
	uint16_t flicker_cfg_settings_size;

//antishake
	const struct mt9t111_i2c_reg_conf (*antishake_cfg_settings)[MT9T111_CFG_ANTISHAKE_MAX_PARAMS];
	uint16_t antishake_cfg_settings_size;

//manual foucs
	const struct mt9t111_i2c_reg_conf (*focus_step_cfg_settings)[MT9T111_CFG_FOCUS_STEP_MAX_PARAMS];
	uint16_t focus_step_cfg_settings_size;

#if 0
//focus mode
	const struct mt9t111_i2c_reg_conf (*autofocus_trigger_cfg_settings)[MT9T111_CFG_AUTOFOCUS_TRIGGER_MAX_PARAMS];
	uint16_t autofocus_trigger_cfg_settings_size;	
#else
	const struct mt9t111_i2c_reg_conf *autofocus_trigger_cfg_settings;
	uint16_t autofocus_trigger_cfg_settings_size;	
#endif

	const struct mt9t111_i2c_reg_conf *autofocus_rect_cfg_settings;
	uint16_t autofocus_rect_cfg_settings_size;	

#if 1 //you4me20110124
	const struct mt9t111_i2c_reg_conf (*scene_portrait_mode_cfg_settings)[48];
	uint16_t scene_portrait_mode_cfg_settings_size;
	const struct mt9t111_i2c_reg_conf (*scene_landscape_mode_cfg_settings)[46];
	uint16_t scene_landscape_mode_cfg_settings_size;
	const struct mt9t111_i2c_reg_conf (*scene_indoor_mode_cfg_settings)[18];
	uint16_t scene_indoor_mode_cfg_settings_size;
	const struct mt9t111_i2c_reg_conf (*scene_sports_mode_cfg_settings)[6];
	uint16_t scene_sports_mode_cfg_settings_size;
	const struct mt9t111_i2c_reg_conf (*scene_night_mode_cfg_settings)[6];
	uint16_t scene_night_mode_cfg_settings_size;
	const struct mt9t111_i2c_reg_conf (*scene_beach_mode_cfg_settings)[46];
	uint16_t scene_beach_mode_cfg_settings_size;
	const struct mt9t111_i2c_reg_conf (*scene_snow_mode_cfg_settings)[4];
	uint16_t scene_snow_mode_cfg_settings_size;
	const struct mt9t111_i2c_reg_conf (*scene_sunset_mode_cfg_settings)[10];
	uint16_t scene_sunset_mode_cfg_settings_size;
	const struct mt9t111_i2c_reg_conf (*scene_text_mode_cfg_settings)[116];
	uint16_t scene_text_mode_cfg_settings_size;
	const struct mt9t111_i2c_reg_conf (*scene_fireworks_mode_cfg_settings)[8];
	uint16_t scene_fireworks_mode_cfg_settings_size;
#else
//scene mode
	const struct mt9t111_i2c_reg_conf (*scene_mode_cfg_settings)[MT9T111_CFG_SCENE_MODE_MAX_PARAMS];
	uint16_t scene_mode_cfg_settings_size;	
#endif

#ifdef F_SKYCAM_FIX_CFG_REFLECT
	const struct mt9t111_i2c_reg_conf (*reflect_cfg_settings)[MT9T111_CFG_REFLECT_MAX_PARAMS];
	uint16_t reflect_cfg_settings_size;
#endif
};


#endif /* __CAMSENSOR_MT9T111_H__ */
