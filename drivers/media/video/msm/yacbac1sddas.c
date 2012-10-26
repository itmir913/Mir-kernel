/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include "yacbac1sddas.h"
#include <mach/vreg.h>

#include "camsensor_yacbac1sddas_tuneup.h"

/* Hynix yacbac1sddas Registers and their values */
/* Sensor Core Registers */
#define YACBAC1SDDAS_REG_MCU_ADDR      0x098E
#define YACBAC1SDDAS_REG_MCU_DATA      0x0990
#define  REG_YACBAC1SDDAS_MODEL_ID 0x3000
#define  YACBAC1SDDAS_MODEL_ID     0x1580

#define REG_YACBAC1SDDAS_REG_MCU_VAR_ADDR      0x098C
#define REG_YACBAC1SDDAS_REG_MCU_VAR_DATA      0x0990
#define REG_YACBAC1SDDAS_REG_FRAME_CNT         0x303a
#define REG_YACBAC1SDDAS_REG_MCU_DATA_SEQ_CMD  0xa103

#define YACBAC1SDDAS_I2C_RETRY_CNT   3
#define YACBAC1SDDAS_MAX_WAIT_CNT    20
#define YACBAC1SDDAS_POLL_PERIOD_MS  10

/* Time in milisecs for waiting for the sensor to reset.*/
#define YACBAC1SDDAS_RESET_DELAY_MSECS   66

#define YACBAC1SDDAS_DEFAULT_CLOCK_RATE 24000000

#ifdef F_SKYCAM_DRIVER_COMPLETE
bool bFRONT_cam_on = false;
EXPORT_SYMBOL(bFRONT_cam_on);
extern bBACK_cam_on;
#endif

struct yacbac1sddas_work
{
	struct work_struct work;
};

static struct yacbac1sddas_work *yacbac1sddas_sensorw;
static struct i2c_client *yacbac1sddas_client;


struct yacbac1sddas_ctrl_t
{
	const struct msm_camera_sensor_info *sensordata;
};

static struct yacbac1sddas_ctrl_t *yacbac1sddas_ctrl = NULL;

struct yacbac1sddas_vreg_t 
{
	const char * name;
	unsigned short mvolt;
};

static DECLARE_WAIT_QUEUE_HEAD(yacbac1sddas_wait_queue);
DECLARE_MUTEX(yacbac1sddas_sem);
static int16_t yacbac1sddas_effect = CAMERA_EFFECT_OFF;

#define ON	1
#define OFF	0

#ifdef F_SKYCAM_TARGET_EF31
#define CAM_3M_STANDBY 	3
#define CAM_VGA_STANDBY 	96

#define CAM_MAIN_STANDBY CAM_3M_STANDBY
#if ((BOARD_VER<=EF31S_PT10)||(BOARD_VER<=EF32K_PT10)) //EF31 PT10
#define CAM_LDO_EN 22
#endif
static struct yacbac1sddas_vreg_t yacbac1sddas_vreg[] = 
{
#if ((BOARD_VER<=EF31S_PT10)||(BOARD_VER<=EF32K_PT10)) //EF31 PT10
       {
		.name = "gp4", //,	/* 2.8V_CAM_IO --> VREG_GP4-AUX1 (out9)*/
		.mvolt = 1800,       
	},
#else
	{
		.name = "gp4", //,	/* 2.8V_CAM_IO --> VREG_GP4-AUX1 (out9)*/
		.mvolt = 2800,       
	},
	{
		.name = "rftx", //,	/* 1.8V_CAM --> VREG_GP1-CAM (out14)*/		
		.mvolt = 1800,      
	},	
#endif	
};
#endif

#ifdef F_SKYCAM_TUP_LOAD_FILE
static yacbac1sddas_tune_state_type yacbac1sddas_tup_state = YACBAC1SDDAS_TUNE_STATE_NONE;
static yacbac1sddas_tune_mode_type yacbac1sddas_tup_mode = YACBAC1SDDAS_TUNE_STATE_TUNNING_MODE_ON;
static uint8_t yacbac1sddas_done_set_tune_load = NULL;
static yacbac1sddas_params_tbl_t yacbac1sddas_params_tbl;
#endif


/*=============================================================
	EXTERNAL DECLARATIONS
==============================================================*/
extern struct yacbac1sddas_reg yacbac1sddas_regs;

/*
================================================================================
LOCAL API DECLARATIONS
================================================================================
*/
#ifdef F_SKYCAM_TUP_LOAD_FILE
static u_int32_t yacbac1sddas_i2c_write_params
(
    yacbac1sddas_cfg_param_t *params,
    u_int32_t num_params
);
#endif


/*
================================================================================
FUNCTION
================================================================================
*/
#define F_SKYCAM_GPIO_SET
#ifdef F_SKYCAM_GPIO_SET
static bool yacbac1sddas_gpio_set(unsigned int gpio_num, bool gpio_on)
{
	int rc =0;

	rc = gpio_request(gpio_num, "yacbac1sddas");
    	if (!rc) {					
    		gpio_direction_output(gpio_num, gpio_on);
    		SKYCDBG("GPIO [%d] ON SUCCESS\n", gpio_num);
		gpio_free(gpio_num);
    	}
        else
        {
    		SKYCERR("GPIO [%d] ON FAIL, retry\n", gpio_num);        
		gpio_free(gpio_num);
		
	    	rc = gpio_request(gpio_num, "yacbac1sddas");
	    	if (!rc) {					
	    		gpio_direction_output(gpio_num, gpio_on);
	    		SKYCDBG("GPIO [%d] ON SUCCESS\n", gpio_num);
			gpio_free(gpio_num);
	    	}
	        else
	    		SKYCERR("GPIO [%d] ON FAIL\n", gpio_num);    
	 }	
}
#endif

#ifdef F_SKYCAM_GPIO_SET
static int yacbac1sddas_power(int8_t on)
{
	struct vreg *vreg = NULL;
	uint32_t vnum =0;
	uint32_t i =0;
	int32_t rc = 0;

	SKYCDBG("%s start state = %d\n",__func__, on);

#if ((BOARD_VER<=EF31S_PT10)||(BOARD_VER<=EF32K_PT10)) //EF31 PT10
	yacbac1sddas_gpio_set(CAM_LDO_EN, on);
#endif
		
        vnum = ARRAY_SIZE(yacbac1sddas_vreg);
	for(i = 0; i < vnum; i++)
	{
		vreg = vreg_get(NULL, yacbac1sddas_vreg[i].name);
		if( IS_ERR(vreg))
		{
			SKYCERR("%s#%d error. i=%d\n", __func__, __LINE__, i);
			rc = -ENOENT;
			goto power_on_fail;
		}

		if(on)
		{
			rc = vreg_set_level(vreg, yacbac1sddas_vreg[i].mvolt);
			if(rc)
			{
				SKYCERR("%s#%d error. i=%d\n", __func__, __LINE__, i);
				goto power_on_fail;
			}

			rc = vreg_enable(vreg);
			if(rc)
			{
				SKYCERR("%s#%d error. i=%d\n", __func__, __LINE__, i);
				goto power_on_fail;
			}			
		}
		else
		{
			rc = vreg_disable(vreg);
				if (rc)
					goto power_on_fail;
		}
	}

	if (on) 
	{ //on
	yacbac1sddas_gpio_set(CAM_VGA_STANDBY, on); 
	}
	else
	{ //off

#if ((BOARD_VER<=EF31S_PT10)||(BOARD_VER<=EF32K_PT10)) //EF31 PT10
	yacbac1sddas_gpio_set(CAM_LDO_EN, on);
#endif		

	yacbac1sddas_gpio_set(CAM_VGA_STANDBY, on);

	yacbac1sddas_gpio_set(CAM_MAIN_STANDBY, on); //active high but off
		
	}
	/* For stability */
	mdelay(10);

	SKYCDBG("%s done.\n", __func__);
	return 0;

power_on_fail:
	SKYCDBG("%s failed! (%d)(%d)(%d)\n", __func__, rc, on, i);
	return rc;
	
}
#else 
static int yacbac1sddas_power(int8_t on)
{
	struct vreg *vreg = NULL;
	uint32_t vnum =0;
	uint32_t i =0;
	int32_t rc = 0;

	SKYCDBG("%s start state = %d\n",__func__, on);

#if ((BOARD_VER<=EF31S_PT10)||(BOARD_VER<=EF32K_PT10)) //EF31 PT10
		    	rc = gpio_request(CAM_LDO_EN, "yacbac1sddas");
		    	if (!rc) {					
		    		gpio_direction_output(CAM_LDO_EN, on);
		    		SKYCDBG("LDO ON SUCCESS\n");
		    	}
		        else
		    		SKYCERR("LDO ON FAIL\n");  
#endif
		
        vnum = ARRAY_SIZE(yacbac1sddas_vreg);
	for(i = 0; i < vnum; i++)
	{
		vreg = vreg_get(NULL, yacbac1sddas_vreg[i].name);
		if( IS_ERR(vreg))
		{
			SKYCERR("%s#%d error. i=%d\n", __func__, __LINE__, i);
			rc = -ENOENT;
			goto power_on_fail;
		}

		if(on)
		{
			rc = vreg_set_level(vreg, yacbac1sddas_vreg[i].mvolt);
			if(rc)
			{
				SKYCERR("%s#%d error. i=%d\n", __func__, __LINE__, i);
				goto power_on_fail;
			}

			rc = vreg_enable(vreg);
			if(rc)
			{
				SKYCERR("%s#%d error. i=%d\n", __func__, __LINE__, i);
				goto power_on_fail;
			}			
		}
		else
		{
			rc = vreg_disable(vreg);
				if (rc)
					goto power_on_fail;
		}
	}

	if (on) 
	{

    	rc = gpio_request(CAM_VGA_STANDBY, "yacbac1sddas");
    	if (!rc) {					
    		gpio_direction_output(CAM_VGA_STANDBY, on);
    		SKYCDBG("CAM_VGA_STANDBY ON SUCCESS\n");
    	}
        else
        {
    		SKYCERR("CAM_VGA_STANDBY ON FAIL\n");        
		gpio_free(CAM_VGA_STANDBY);
		
	    	rc = gpio_request(CAM_VGA_STANDBY, "yacbac1sddas");
	    	if (!rc) {					
	    		gpio_direction_output(CAM_VGA_STANDBY, on);
	    		SKYCDBG("CAM_VGA_STANDBY ON SUCCESS\n");
	    	}
	        else
	    		SKYCERR("CAM_VGA_STANDBY ON FAIL\n");    
	 }	
	}
	else
	{
#if ((BOARD_VER<=EF31S_PT10)||(BOARD_VER<=EF32K_PT10)) //EF31 PT10
		gpio_direction_output(CAM_LDO_EN, on);
		SKYCDBG("LDO OFF SUCCESS\n");
#endif
		gpio_direction_output(CAM_VGA_STANDBY, on);
		SKYCDBG("CAM_VGA_STANDBY OFF SUCCESS\n");

		gpio_direction_output(CAM_MAIN_STANDBY, on);
		SKYCDBG("CAM_MAIN_STANDBY OFF SUCCESS\n");
#if ((BOARD_VER<=EF31S_PT10)||(BOARD_VER<=EF32K_PT10)) //EF31 PT10  20110112
		gpio_free(CAM_LDO_EN);
#endif
		gpio_free(CAM_VGA_STANDBY);
		gpio_free(CAM_MAIN_STANDBY);
	}
	/* For stability */
	mdelay(10);

	SKYCDBG("%s done.\n", __func__);
	return 0;

power_on_fail:
	SKYCDBG("%s failed! (%d)(%d)(%d)\n", __func__, rc, on, i);
	return rc;
	
}
#endif



static int yacbac1sddas_reset(const struct msm_camera_sensor_info *dev, int set)
{
	int rc = 0;
	
	rc = gpio_request(dev->sensor_reset, "yacbac1sddas");
		SKYCDBG("[2.5_gpio_request(dev->sensor_reset) = %d]\n", dev->sensor_reset);
		
	if (!rc) 
	{					
		rc = gpio_direction_output(dev->sensor_reset, 0);
		SKYCDBG("[3_reset pin = %d, value = LOW]\n", dev->sensor_reset);
//		mdelay(20);
		if(set)
		{
			rc = gpio_direction_output(dev->sensor_reset, set);
				SKYCDBG("[4_reset pin = %d, value = HIGH]\n", dev->sensor_reset);
		}

 	gpio_free(dev->sensor_reset);


			
	}

	//if(set == OFF && dev->sensor_reset)
//		gpio_free(dev->sensor_reset);
	
	return rc;
}


static int32_t yacbac1sddas_i2c_txdata(unsigned short saddr, unsigned char *txdata, int length)
{
	struct i2c_msg msg[] = 
	{
		{
			.addr = saddr,
			.flags = 0,
			.len = length,
			.buf = txdata,
		},
	};

	if( i2c_transfer(yacbac1sddas_client->adapter, msg ,1 ) < 0)
	{
		int rc;
		SKYCERR("yacbac1sddas_i2c_txdata failed error = %d retry after 20ms\n", rc);
		mdelay(20);
		rc = i2c_transfer(yacbac1sddas_client->adapter, msg ,1 );
		if(rc<0)
			SKYCERR("yacbac1sddas_i2c_txdata failed error = %d \n", rc);
		return rc;
	//	SKYCERR("yacbac1sddas_i2c_txdata failed\n");
	//	return -EIO;
	}

	return 0;
		
}

static int32_t yacbac1sddas_i2c_write_byte(unsigned char waddr, unsigned char wdata)
{
	int32_t rc = -EIO;
	unsigned char buf[2];

	memset(buf, 0, sizeof(buf));

	buf[0] = waddr;
	buf[1] = wdata;
	
	rc = yacbac1sddas_i2c_txdata(yacbac1sddas_client->addr, buf, 2);
#ifdef F_SKYCAM_TUP_LOAD_FILE
	SKYCDBG("<<[WRITE BYTE!] waddr=0x%x, wdata=0x%x\n", waddr, wdata);
#endif
	if (rc < 0)
		SKYCERR(
		"i2c_write failed, saddr= 0x%x, addr = 0x%x, val = 0x%x!\n",
		yacbac1sddas_client->addr, waddr, wdata);

	return rc;
}

static int32_t yacbac1sddas_i2c_write(unsigned short saddr, unsigned short waddr, unsigned short wdata, enum yacbac1sddas_width width)
{
	int32_t rc = -EFAULT;
	unsigned char buf[4];

	memset(buf, 0, sizeof(buf));

	switch(width)
	{
		case WORD_LEN:  {
			buf[0] = (waddr & 0xFF00)>>8;
			buf[1] = (waddr & 0x00FF);
			buf[2] = (wdata & 0xFF00)>>8;
			buf[3] = (wdata & 0x00FF);

			rc = yacbac1sddas_i2c_txdata(saddr, buf, 4);
		}
			break;

		case TRIPLE_LEN: {
		buf[0] = (waddr & 0xFF00)>>8;
		buf[1] = (waddr & 0x00FF);
		buf[2] = wdata;
			rc = yacbac1sddas_i2c_txdata(saddr, buf, 3);
		}
			break;

		case BYTE_LEN:
		{
			buf[0] = waddr;
			buf[1] = wdata;
			rc = yacbac1sddas_i2c_txdata(saddr, buf, 2);
		}
		break;

		default:
			break;
	}

	if( rc < 0 )
		SKYCERR("i2c write failed, addr = 0x%x, val = 0x%x\n", waddr, wdata);

	return rc;

}



static int32_t yacbac1sddas_i2c_write_table(
	struct yacbac1sddas_i2c_reg_conf const * reg_conf_tbl, 
	int num_of_items_in_table)
{
	int i;
	int32_t rc = 0; //-EFAULT;

	for(i = 0; i < num_of_items_in_table; i++)
	{
		if(reg_conf_tbl->width == ZERO_LEN)
		{
			SKYCDBG("ZERO_LEN continue ADDR = 0x%x, VALUE = 0x%x\n",reg_conf_tbl->waddr, reg_conf_tbl->wdata);
			continue;
		}
		
		rc = yacbac1sddas_i2c_write(yacbac1sddas_client->addr,
			reg_conf_tbl->waddr, reg_conf_tbl->wdata,
			reg_conf_tbl->width);

		if ( rc < 0)
			return rc;

		if ( reg_conf_tbl->mdelay_time != 0)
			mdelay(reg_conf_tbl->mdelay_time);

		reg_conf_tbl++;
	}

	return rc;

}

static int yacbac1sddas_i2c_rxdata(unsigned short saddr,int slength, unsigned char *rxdata, int length)
{

	 struct i2c_msg msgs[] = 
	 {
	 		{
				.addr = saddr,
				.flags = 0,
				.len = slength,
				.buf = rxdata,
	 		},
			{
				.addr = saddr,
				.flags = I2C_M_RD,
				.len = length,
				.buf = rxdata,
			},
	 };

	if(i2c_transfer(yacbac1sddas_client->adapter, msgs, 2) < 0)
	{
		SKYCERR("yacbac1sddas_i2c_rxdata failed!\n");
		return -EIO;
	}

	return 0;

}

static int32_t yacbac1sddas_i2c_read_byte(unsigned char raddr, unsigned char *rdata)
{
	int32_t rc = 0;
	unsigned char buf[1];

	if (!rdata)
		return -EIO;

	memset(buf, 0, sizeof(buf));

	buf[0] = raddr;	

	rc = yacbac1sddas_i2c_rxdata(yacbac1sddas_client->addr, 1, buf, 1);
	if (rc < 0)
		return rc;

	*rdata = buf[0];

	if (rc < 0)
		SKYCERR("mt9p111_i2c_read_word failed!\n");

	return rc;
}

static int32_t yacbac1sddas_i2c_read(unsigned short saddr, unsigned short raddr, unsigned short *rdata)
{
	int32_t rc =0;
	unsigned char buf[4];

	if(!rdata)
		return -EIO;

	memset(buf, 0, sizeof(buf));

	buf[0] = (raddr & 0xFF00) >> 8;
	buf[1] = (raddr & 0x00FF);

	rc = yacbac1sddas_i2c_rxdata(saddr, 2, buf, 2);

	if(rc < 0)
		return rc;
	*rdata = buf[0] << 8 | buf[1];

	if( rc < 0 )
		SKYCERR("yacbac1sddas_i2c_read failed!\n");

	return rc;
}



/* Need to Check initializ register */
static int yacbac1sddas_reg_init(void)
{
	int rc = 0;

	SKYCDBG("yacbac1sddas_client->addr I2C Slave Addr = %x!\n", yacbac1sddas_client->addr);

#ifdef F_SKYCAM_TUP_LOAD_FILE
	// 튜닝 파일값
	if((yacbac1sddas_tup_state == YACBAC1SDDAS_TUNE_STATE_LOAD_VALUE) && (yacbac1sddas_params_tbl.init.num_params > 0))	
	{
		yacbac1sddas_i2c_write_params(yacbac1sddas_params_tbl.init.params,
								yacbac1sddas_params_tbl.init.num_params);
	}
	// static 값
	else
#endif
	{

		rc = yacbac1sddas_i2c_write_table(&yacbac1sddas_regs.init_parm[0],
						yacbac1sddas_regs.init_parm_size);	
	}

	if (rc < 0)
	{
		SKYCERR("yacbac1sddas_reg_init i2c write failed!\n");
		return rc;
	}

	SKYCDBG("yacbac1sddas_reg_init Success!\n");

	return rc;	
}

#ifdef F_SKYCAM_FIX_CFG_EFFECT
static int yacbac1sddas_set_effect(int effect)
{
	int rc = 0;

	if(effect < YACBAC1SDDAS_CFG_EFFECT_NONE || effect >= YACBAC1SDDAS_CFG_EFFECT_MAX){
		SKYCERR("%s error. effect=%d\n", __func__, effect);
		return -EINVAL;
	}

#ifdef F_SKYCAM_TUP_LOAD_FILE
	// 튜닝 파일값
	if((yacbac1sddas_tup_state == YACBAC1SDDAS_TUNE_STATE_LOAD_VALUE) && (yacbac1sddas_params_tbl.effect[effect].num_params > 0))	
	{
		yacbac1sddas_i2c_write_params(yacbac1sddas_params_tbl.effect[effect].params,
								yacbac1sddas_params_tbl.effect[effect].num_params);
	}
	// static 값
	else
#endif		
	{
		rc = yacbac1sddas_i2c_write_table(yacbac1sddas_regs.effect_cfg_settings[effect],
						yacbac1sddas_regs.effect_cfg_settings_size);
	}

	if (rc < 0)
	{
		SKYCERR("CAMERA_WB I2C FAIL!!! return~~\n");
		return rc;
	}		

	yacbac1sddas_effect = effect;
	/* Refresh Sequencer */	
	SKYCDBG("%s end\n",__func__);

	return rc;
	
}
#endif

static int32_t yacbac1sddas_start_preview(void)
{
	int32_t rc = 0;

	SKYCDBG("%s done.\n", __func__);
#ifdef F_SKYCAM_TUP_LOAD_FILE
	if(yacbac1sddas_tup_state == YACBAC1SDDAS_TUNE_STATE_LOAD_VALUE)
	{		
		rc = yacbac1sddas_reg_init();
		if (rc < 0)
		{
			SKYCERR("yacbac1sddas_i2c_write_table FAIL!!! return~~\n");
			return rc;
		}		
	}	
#endif		

	return rc;
}

static int32_t yacbac1sddas_start_snapshot(void)
{
	int32_t rc = 0;

       SKYCDBG("%s done.\n", __func__);
	   
	return rc;
}


static int yacbac1sddas_set_sensor_mode(int mode)
{
	int32_t rc = 0;

	SKYCDBG("%s start , mode = %d\n" , __func__, mode);

	switch(mode)  // check data sheet
	{
		case SENSOR_PREVIEW_MODE: /* Don't Need Set Sensor Driver to use i2c device */
			SKYCDBG("yacbac1sddas SENSOR_PREVIEW_MODE\n");
			yacbac1sddas_start_preview();		
			break;

		case SENSOR_SNAPSHOT_MODE: /* Don't Need Set Sensor Driver to use i2c device */
			SKYCDBG("yacbac1sddas SENSOR_SNAPSHOT_MODE\n");
			yacbac1sddas_start_snapshot();	
			break;

		default:
			rc = -EINVAL;
			break;
	}

	if (rc < 0)
		return rc;

	SKYCDBG("%s end\n",__func__);
	return rc;
}

#ifdef F_SKYCAM_FIX_CFG_PREVIEW_FPS
static int32_t yacbac1sddas_set_preview_fps(int32_t preview_fps)
{
	/* 0 : variable 30fps, 1 ~ 30 : fixed fps */
	/* default: variable 8 ~ 30fps */
	int32_t rc = 0;
	
	if ((preview_fps < C_SKYCAM_MIN_PREVIEW_FPS) || 
		(preview_fps > C_SKYCAM_MAX_PREVIEW_FPS)) {
		SKYCERR("%s: -EINVAL, preview_fps=%d\n", 
			__func__, preview_fps);
		return -EINVAL;
	}

	SKYCDBG("%s: preview_fps=%d\n", __func__, preview_fps);

#ifdef F_SKYCAM_TUP_LOAD_FILE
	if((yacbac1sddas_tup_state == YACBAC1SDDAS_TUNE_STATE_LOAD_VALUE) && (yacbac1sddas_params_tbl.ffps[preview_fps-5].num_params > 0))	
	{
		yacbac1sddas_i2c_write_params(yacbac1sddas_params_tbl.ffps[preview_fps-5].params,
								yacbac1sddas_params_tbl.ffps[preview_fps-5].num_params);
	}
	// static 값
	else
#endif		
	{
		rc = yacbac1sddas_i2c_write_table(&yacbac1sddas_regs.preview_fps_cfg_settings[preview_fps-5],
						yacbac1sddas_regs.preview_fps_cfg_settings_size);
	}	

	if(rc < 0)
	{
		SKYCERR("Video fps setting i2c_write_talbe fail\n");
		return rc;
	}

	SKYCDBG("%s end rc = %d\n",__func__, rc);

	return rc;	
}
#endif

#ifdef F_SKYCAM_FIX_CFG_BRIGHTNESS
static int32_t yacbac1sddas_set_brightness(int32_t brightness)
{
	int32_t rc = 0;
	int i = 0;
	SKYCDBG("%s start~ receive brightness = %d\n",__func__, brightness);
	
	if ((brightness < YACBAC1SDDAS_CFG_BRIGHT_M4) || (brightness >= YACBAC1SDDAS_CFG_BRIGHT_MAX)) {
		SKYCERR("%s error. brightness=%d\n", __func__, brightness);
		return -EINVAL;
	}
#ifdef F_SKYCAM_TUP_LOAD_FILE
	// 튜닝 파일값
	if((yacbac1sddas_tup_state == YACBAC1SDDAS_TUNE_STATE_LOAD_VALUE) && (yacbac1sddas_params_tbl.bright[brightness].num_params > 0))	
	{
		yacbac1sddas_i2c_write_params(yacbac1sddas_params_tbl.bright[brightness].params,
								yacbac1sddas_params_tbl.bright[brightness].num_params);
	}
	// static 값
	else
#endif		
	{
		rc = yacbac1sddas_i2c_write_table(yacbac1sddas_regs.bright_cfg_settings[brightness],	
						yacbac1sddas_regs.bright_cfg_settings_size);
	}

	if (rc < 0)
	{
		SKYCERR("CAMERA_BRIGHTNESS I2C FAIL!!! return~~\n");
		return rc;
	}		
	
	SKYCDBG("%s end\n",__func__);

	return rc;
}
#endif

#ifdef F_SKYCAM_FIX_CFG_WB
static int32_t yacbac1sddas_set_wb(int32_t wb)
{
	int32_t rc=0;

	SKYCDBG("%s: wb=%d\n", __func__, wb);
	
#ifdef F_SKYCAM_TUP_LOAD_FILE
	// 튜닝 파일값
	if((yacbac1sddas_tup_state == YACBAC1SDDAS_TUNE_STATE_LOAD_VALUE) && (yacbac1sddas_params_tbl.wb[wb-1].num_params > 0))	
	{
		yacbac1sddas_i2c_write_params(yacbac1sddas_params_tbl.wb[wb-1].params,
								yacbac1sddas_params_tbl.wb[wb-1].num_params);
	}
	// static 값
	else
#endif		
	{
		rc = yacbac1sddas_i2c_write_table(yacbac1sddas_regs.wb_cfg_settings[wb -1],
						yacbac1sddas_regs.wb_cfg_settings_size);
	}

	if (rc < 0)
	{
		SKYCERR("CAMERA_WB I2C FAIL!!! return~~\n");
		return rc;
	}		

	SKYCDBG("%s end\n",__func__);

	return rc;
}
#endif

static int32_t yacbac1sddas_set_exposure_mode(int32_t exposure)
{

	int32_t rc = 0;
		
	SKYCDBG("%s: exposure=%d\n", __func__, exposure);
	
	if ((exposure < YACBAC1SDDAS_CFG_EXPOSURE_NORMAL) || (exposure >= YACBAC1SDDAS_CFG_EXPOSURE_MAX))
		return -EINVAL;

#ifdef F_SKYCAM_TUP_LOAD_FILE
	if((yacbac1sddas_tup_state == YACBAC1SDDAS_TUNE_STATE_LOAD_VALUE) && (yacbac1sddas_params_tbl.exposure[exposure].num_params > 0))	
	{
		yacbac1sddas_i2c_write_params(yacbac1sddas_params_tbl.exposure[exposure].params,
								yacbac1sddas_params_tbl.exposure[exposure].num_params);
	}
	// static 값
	else
#endif		
	{
		rc = yacbac1sddas_i2c_write_table(yacbac1sddas_regs.exposure_cfg_settings[exposure],
						yacbac1sddas_regs.exposure_cfg_settings_size);
	}

	if (rc < 0)
	{
		SKYCERR("CAMERA_EXPOSURE I2C FAIL!!! return~~\n");
		return rc;
	}		
	
	SKYCDBG("%s end\n",__func__);

	return rc;
}

#ifdef F_SKYCAM_FIX_CFG_REFLECT
static int32_t yacbac1sddas_set_reflect(int32_t reflect)
{

	int32_t rc = 0;
		
	SKYCDBG("%s: reflect=%d\n", __func__, reflect);
	
	if ((reflect < YACBAC1SDDAS_CFG_REFLECT_NONE) || (reflect >= YACBAC1SDDAS_CFG_REFLECT_MAX))
		return -EINVAL;

#ifdef F_SKYCAM_TUP_LOAD_FILE
	if((yacbac1sddas_tup_state == YACBAC1SDDAS_TUNE_STATE_LOAD_VALUE) && (yacbac1sddas_params_tbl.reflect[reflect].num_params > 0))	
	{
		yacbac1sddas_i2c_write_params(yacbac1sddas_params_tbl.reflect[reflect].params,
								yacbac1sddas_params_tbl.reflect[reflect].num_params);
	}
	// static 값
	else
#endif		
	{
		rc = yacbac1sddas_i2c_write_table(yacbac1sddas_regs.reflect_cfg_settings[reflect],
						yacbac1sddas_regs.reflect_cfg_settings_size);
	}

	if (rc < 0)
	{
		SKYCERR("CAMERA_REFLECT I2C FAIL!!! return~~\n");
		return rc;
	}		
	
	SKYCDBG("%s end\n",__func__);

	return rc;
}
#endif

#ifdef F_SKYCAM_FIX_CFG_ANTIBANDING
static int32_t yacbac1sddas_set_antibanding(int32_t antibanding)
{
	int32_t rc = 0;

	if ((antibanding < 0) || (antibanding >= YACBAC1SDDAS_CFG_FLICKER_MAX))
	{
		SKYCERR("%s FAIL!!! return~~  antibanding = %d\n",__func__,antibanding);
		return 0;//-EINVAL;
	}
#ifdef F_SKYCAM_TUP_LOAD_FILE
	if((yacbac1sddas_tup_state == YACBAC1SDDAS_TUNE_STATE_LOAD_VALUE) && (yacbac1sddas_params_tbl.flicker[antibanding].num_params > 0))	
	{
		yacbac1sddas_i2c_write_params(yacbac1sddas_params_tbl.flicker[antibanding].params,
								yacbac1sddas_params_tbl.flicker[antibanding].num_params);
	}
	// static 값
	else
#endif		
	{
		rc = yacbac1sddas_i2c_write_table(yacbac1sddas_regs.flicker_cfg_settings[antibanding],	
					yacbac1sddas_regs.flicker_cfg_settings_size);
	}		

	if (rc < 0)
	{
		SKYCERR("%s FAIL!!! return~~\n",__func__);
		return rc;
	}		

	SKYCDBG("%s end\n",__func__);
	return rc;
}
#endif

#ifdef F_SKYCAM_TUP_LOAD_FILE
static u_int32_t yacbac1sddas_i2c_write_params
(
    yacbac1sddas_cfg_param_t *params,
    u_int32_t num_params
)
{
    u_int8_t read_data = 0;
    u_int8_t write_data = 0;
    u_int8_t write_data_byte = 0;	
    u_int32_t i = 0;

    SKYCDBG(">>%s ", __func__);
    SKYCDBG("- params = 0x%08x, num_params = %d", params, num_params, 0);

    if ((params == NULL) || (num_params == 0))
    {
        SKYCERR("Invalid param! [params = 0x%08x][num_params = %d]", (u_int32_t)params, num_params, 0);
        return -EIO;    
    }

    for (i = 0; i < num_params; i++)
    {
        switch(params[i].op)
        {
            case CAMOP_NOP:
                SKYCDBG("WRITE: NOP", 0, 0, 0);
                break;

            case CAMOP_DELAY:
                SKYCDBG("WRITE: DELAY (%dms)", params[i].data, 0, 0);
                mdelay(params[i].data);
                break;

            case CAMOP_WRITE:
                if (yacbac1sddas_i2c_write_byte((unsigned char)params[i].addr, (unsigned char)params[i].data) < 0)
                {
                    SKYCERR("<<%s (-EIO)", __func__, 0, 0);
                    return -EIO;
                }
                break;	     
            case CAMOP_WRITE_BIT:
            {
                u_int8_t bit_pos = 0;
                u_int8_t bit_val = 0;

                if (yacbac1sddas_i2c_read_byte((unsigned char)params[i].addr, &read_data) < 0)
                {
                    SKYCERR("<<%s (-EIO)", __func__, 0, 0);
                    return -EIO;
                }

                bit_pos = (params[i].data & 0xff00) >> 8;
                bit_val = (params[i].data & 0x00ff);

                if (bit_val != 0) // set to 1
                {
                    write_data = read_data | (0x01 << bit_pos);
                    SKYCDBG("set 0x%04x[%d] to 1", params[i].addr, bit_pos, 0);
                }
                else // set to 0
                {
                    write_data = read_data & (~(0x01 << bit_pos));
                    SKYCDBG("set 0x%04x[%d] to 0", params[i].addr, bit_pos, 0);
                }

                if (yacbac1sddas_i2c_write_byte((unsigned char)params[i].addr, write_data) < 0)
                {
                    SKYCERR("<<%s (-EIO)", __func__, 0, 0);
                    return -EIO;
                }
                break;
            }

            case CAMOP_POLL_REG:
            {
                u_int8_t poll_delay = 0;
                u_int8_t poll_retry = 0;
                u_int16_t poll_reg = 0;
                u_int8_t poll_data = 0;
                u_int8_t poll_mask = 0;
                u_int16_t retry_cnt = 0;

                if (params[i+1].op != CAMOP_POLL_REG)
                {
                    SKYCERR("<<%s (-EIO)", __func__, 0, 0);
                    return -EIO;
                }

                poll_delay = (params[i+1].data & 0xff00) >> 8;
                poll_retry = (params[i+1].data & 0x00ff);
                poll_reg = params[i].addr;
                poll_data = params[i+1].addr;
                poll_mask = params[i].data;;

                SKYCDBG("start polling register... if [0x%04x] AND 0x%04x equals 0x%04x, then stop polling", poll_reg, poll_mask, poll_data);


                for (retry_cnt = 0; retry_cnt < poll_retry; retry_cnt++)
                {
                    if (yacbac1sddas_i2c_read_byte((unsigned char)poll_reg, &read_data) < 0)
                    {
                        SKYCERR("<<%s (-EIO)", __func__, 0, 0);
                        return -EIO;
                    }
                    
                    if ((read_data & poll_mask) != poll_data)
                    {
                        SKYCDBG("retry polling register... after sleeping %d ms", poll_delay, 0, 0);
                        mdelay(poll_delay);
                    }
                    else
                    {
                        SKYCDBG("stop polling register... retried %d/%d time(s) (delay = %d ms)", retry_cnt, poll_retry, poll_delay);
                        break;
                    }
                }

                if (retry_cnt == poll_retry)
                {
                    SKYCDBG("<<%s (-EIO)", __func__, 0, 0);
                    return -EIO;
                }

                ++i;
                break;
            }

            case CAMOP_POLL_MCU_VAR:
            {
                u_int16_t poll_delay = 0;
                u_int16_t poll_retry = 0;
                u_int16_t poll_mcu_var = 0;
                u_int16_t poll_data = 0;
                u_int16_t poll_mask = 0;
                u_int16_t retry_cnt = 0;

                if (params[i+1].op != CAMOP_POLL_MCU_VAR)
                {
                    SKYCDBG("<<%s (-EIO)", __func__, 0, 0);
                    return -EIO;
                }

                poll_delay = (params[i+1].data & 0xff00) >> 8;
                poll_retry = (params[i+1].data & 0x00ff);
                poll_mcu_var = params[i].addr;
                poll_data = params[i+1].addr;
                poll_mask = params[i].data;;

                SKYCDBG("start polling MCU variable... if [0x%04x] AND 0x%04x equals 0x%04x, then stop polling", poll_mcu_var, poll_mask, poll_data);


                for (retry_cnt = 0; retry_cnt < poll_retry; retry_cnt++)
                {
                    if (yacbac1sddas_i2c_write_byte(YACBAC1SDDAS_REG_MCU_ADDR, poll_mcu_var) < 0)
                    {
                        SKYCERR("<<%s (-EIO)", __func__, 0, 0);
                        return -EIO;
                    }

                    if (yacbac1sddas_i2c_read_byte(YACBAC1SDDAS_REG_MCU_DATA, &read_data) < 0)
                    {
                        SKYCERR("<<%s (-EIO)", __func__, 0, 0);
                        return -EIO;
                    }
                    
                    if ((read_data & poll_mask) != poll_data)
                    {
                        SKYCDBG("retry polling MCU variable... after sleeping %d ms", poll_delay, 0, 0);
                        mdelay(poll_delay);
                    }
                    else
                    {
                        SKYCDBG("stop polling MCU variable... retried %d/%d time(s) (delay = %d ms)", retry_cnt, poll_retry, poll_delay);
                        break;
                    }
                }

                if (retry_cnt == poll_retry)
                {
                    SKYCDBG("<<%s (-EIO)", __func__, 0, 0);
                    return -EIO;
                }

                ++i;
                break;
            }

            default:
                SKYCDBG("Invalid op code! [op = %d]", params[i].op, 0, 0);
                return -EIO;
                break;
        }

    }

    SKYCDBG("<<%s (TRUE)", __func__, 0, 0);
    return TRUE;
}

static void yacbac1sddas_init_params_tbl (int8_t *stream)
{
#if defined(F_SKYCAM_SENSOR_TUNEUP)
    yacbac1sddas_cfg_param_t *params = NULL;
    u_int32_t num_params = 0;
#endif /* F_SKYCAM_SENSOR_TUNEUP */
    u_int32_t i = 0;

    SKYCDBG(">>%s ()", __func__, 0, 0);

#if defined(F_SKYCAM_SENSOR_TUNEUP)
    camsensor_yacbac1sddas_tup_init(stream);
    num_params = camsensor_yacbac1sddas_tup_get_init_params(&params);
    if ((num_params > 0) && (params != NULL))
    {
        yacbac1sddas_params_tbl.init.num_params = num_params;
        yacbac1sddas_params_tbl.init.params = params;
        SKYCDBG("YACBAC1SDDAS INIT params are loaded from TUNEUP file!", 0, 0, 0);
    }
    else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
    {
        yacbac1sddas_params_tbl.init.num_params = num_params;
        yacbac1sddas_params_tbl.init.params = params;
        SKYCDBG("YACBAC1SDDAS INIT params are loaded from TUNEUP file!", 0, 0, 0);
#if 0		
        yacbac1sddas_params_tbl.init.num_params = sizeof(yacbac1sddas_cfg_init_params) / sizeof(yacbac1sddas_cfg_param_t);
        yacbac1sddas_params_tbl.init.params = &yacbac1sddas_cfg_init_params[0];
        SKYCDBG("YACBAC1SDDAS INIT params are loaded from RO data!", 0, 0, 0);
#endif		
    }

    for (i = 0; i < YACBAC1SDDAS_CFG_WB_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_yacbac1sddas_tup_get_wb_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            yacbac1sddas_params_tbl.wb[i].num_params = num_params;
            yacbac1sddas_params_tbl.wb[i].params = params;
            SKYCDBG("YACBAC1SDDAS WB params [%d] are loaded from TUNEUP file!", i, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {        
            yacbac1sddas_params_tbl.wb[i].num_params = num_params;
            yacbac1sddas_params_tbl.wb[i].params = params;
            SKYCDBG("YACBAC1SDDAS WB params [%d] are loaded from TUNEUP file!", i, 0, 0);        
#if 0        
            yacbac1sddas_params_tbl.wb[i].num_params = YACBAC1SDDAS_CFG_WB_MAX_PARAMS;
            yacbac1sddas_params_tbl.wb[i].params = &yacbac1sddas_cfg_wb_params[i][0];
            SKYCDBG("YACBAC1SDDAS WB params [%d] are loaded from RO data!", i, 0, 0);
#endif			
        }
    }

    for (i = 0; i < YACBAC1SDDAS_CFG_BRIGHT_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_yacbac1sddas_tup_get_bright_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            yacbac1sddas_params_tbl.bright[i].num_params = num_params;
            yacbac1sddas_params_tbl.bright[i].params = params;
            SKYCDBG("YACBAC1SDDAS BRIGHT params [%d] are loaded from TUNEUP file!", i, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {
      	     yacbac1sddas_params_tbl.bright[i].num_params = num_params;
            yacbac1sddas_params_tbl.bright[i].params = params;
            SKYCDBG("YACBAC1SDDAS BRIGHT params [%d] are loaded from TUNEUP file!", i, 0, 0);
#if 0        
            yacbac1sddas_params_tbl.bright[i].num_params = YACBAC1SDDAS_CFG_BRIGHT_MAX_PARAMS;
            yacbac1sddas_params_tbl.bright[i].params = &yacbac1sddas_cfg_bright_params[i][0];
            SKYCDBG("YACBAC1SDDAS BRIGHT params [%d] are loaded from RO data!", i, 0, 0);
#endif			
        }

    }

    for (i = 0; i < YACBAC1SDDAS_CFG_EXPOSURE_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_yacbac1sddas_tup_get_exposure_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            yacbac1sddas_params_tbl.exposure[i].num_params = num_params;
            yacbac1sddas_params_tbl.exposure[i].params = params;
            SKYCDBG("YACBAC1SDDAS EXPOSURE params [%d] are loaded from TUNEUP file!", i, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {
            yacbac1sddas_params_tbl.exposure[i].num_params = num_params;
            yacbac1sddas_params_tbl.exposure[i].params = params;
            SKYCDBG("YACBAC1SDDAS EXPOSURE params [%d] are loaded from TUNEUP file!", i, 0, 0);
#if 0        
            yacbac1sddas_params_tbl.exposure[i].num_params = YACBAC1SDDAS_CFG_EXPOSURE_MAX_PARAMS;
            yacbac1sddas_params_tbl.exposure[i].params = &yacbac1sddas_cfg_exposure_params[i][0];
            SKYCDBG("YACBAC1SDDAS EXPOSURE params [%d] are loaded from RO data!", i, 0, 0);
#endif			
        }

    }

    for (i = YACBAC1SDDAS_CFG_FFPS_MIN; i <= YACBAC1SDDAS_CFG_FFPS_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_yacbac1sddas_tup_get_ffps_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            yacbac1sddas_params_tbl.ffps[i - YACBAC1SDDAS_CFG_FFPS_MIN].num_params = num_params;
            yacbac1sddas_params_tbl.ffps[i - YACBAC1SDDAS_CFG_FFPS_MIN].params = params;
            SKYCDBG("YACBAC1SDDAS fixed FPS params [%d] are loaded from TUNEUP file!", i - YACBAC1SDDAS_CFG_FFPS_MIN, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {
            yacbac1sddas_params_tbl.ffps[i - YACBAC1SDDAS_CFG_FFPS_MIN].num_params = num_params;
            yacbac1sddas_params_tbl.ffps[i - YACBAC1SDDAS_CFG_FFPS_MIN].params = params;
            SKYCDBG("YACBAC1SDDAS fixed FPS params [%d] are loaded from TUNEUP file!", i - YACBAC1SDDAS_CFG_FFPS_MIN, 0, 0);
#if 0        
            yacbac1sddas_params_tbl.ffps[i - YACBAC1SDDAS_CFG_FFPS_MIN].num_params = YACBAC1SDDAS_CFG_FFPS_MAX_PARAMS;
            yacbac1sddas_params_tbl.ffps[i - YACBAC1SDDAS_CFG_FFPS_MIN].params = &yacbac1sddas_cfg_ffps_params[i - YACBAC1SDDAS_CFG_FFPS_MIN][0];
            SKYCDBG("YACBAC1SDDAS fixed FPS params [%d] are loaded from RO data!", i - YACBAC1SDDAS_CFG_FFPS_MIN, 0, 0);
#endif			
        }
    }
#if 0
    for (i = 0; i < YACBAC1SDDAS_CFG_REFLECT_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_yacbac1sddas_tup_get_reflect_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            yacbac1sddas_params_tbl.reflect[i].num_params = num_params;
            yacbac1sddas_params_tbl.reflect[i].params = params;
            SKYCDBG("YACBAC1SDDAS REFLECT params [%d] are loaded from TUNEUP file!", i, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {
            yacbac1sddas_params_tbl.reflect[i].num_params = num_params;
            yacbac1sddas_params_tbl.reflect[i].params = params;
            SKYCDBG("YACBAC1SDDAS REFLECT params [%d] are loaded from TUNEUP file!", i, 0, 0);
#if 0        
            yacbac1sddas_params_tbl.reflect[i].num_params = YACBAC1SDDAS_CFG_FFPS_MAX_PARAMS;
            yacbac1sddas_params_tbl.reflect[i].params = &yacbac1sddas_cfg_reflect_params[i][0];
            SKYCDBG("YACBAC1SDDAS REFLECT params [%d] are loaded from RO data!", i, 0, 0);
#endif			
        }
    }

    for (i = 0; i < YACBAC1SDDAS_CFG_EFFECT_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_yacbac1sddas_tup_get_effect_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            yacbac1sddas_params_tbl.effect[i].num_params = num_params;
            yacbac1sddas_params_tbl.effect[i].params = params;
            SKYCDBG("YACBAC1SDDAS EFFECT params [%d] are loaded from TUNEUP file!", i, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {
            yacbac1sddas_params_tbl.effect[i].num_params = num_params;
            yacbac1sddas_params_tbl.effect[i].params = params;
            SKYCDBG("YACBAC1SDDAS EFFECT params [%d] are loaded from TUNEUP file!", i, 0, 0);
#if 0        
            yacbac1sddas_params_tbl.effect[i].num_params = YACBAC1SDDAS_CFG_EFFECT_MAX_PARAMS;
            yacbac1sddas_params_tbl.effect[i].params = &yacbac1sddas_cfg_effect_params[i][0];
            SKYCDBG("YACBAC1SDDAS EFFECT params [%d] are loaded from RO data!", i, 0, 0);
#endif
	}
    }
	
for (i = 0; i < YACBAC1SDDAS_CFG_FLICKER_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_yacbac1sddas_tup_get_flicker_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            yacbac1sddas_params_tbl.flicker[i].num_params = num_params;
            yacbac1sddas_params_tbl.flicker[i].params = params;
            SKYCDBG("YACBAC1SDDAS FLICKER params [%d] are loaded from TUNEUP file!", i, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {
            yacbac1sddas_params_tbl.flicker[i].num_params = num_params;
            yacbac1sddas_params_tbl.flicker[i].params = params;
	     SKYCDBG("YACBAC1SDDAS FLICKER params [%d] are loaded from TUNEUP file!", i, 0, 0);
#if 0        
            yacbac1sddas_params_tbl.flicker[i].num_params = YACBAC1SDDAS_CFG_FLICKER_MAX_PARAMS;
            yacbac1sddas_params_tbl.flicker[i].params = &yacbac1sddas_cfg_flicker_params[i][0];
            SKYCDBG("YACBAC1SDDAS FLICKER params [%d] are loaded from RO data!", i, 0, 0);
#endif
	}
    }
#endif
    SKYCDBG("<<%s ()", __func__, 0, 0);
    return;
}

static int yacbac1sddas_set_tune_value(const struct sensor_cfg_data *scfg)
{
	int32_t rc = 0;
	int32_t loop_count = 0;
	int32_t i = 0;
	int32_t last_size = 0;
	int32_t copy_size = 0;
	uint8_t * pKTune_stream = NULL;
	uint8_t * pUTune_stream = NULL;

if(yacbac1sddas_tup_state != YACBAC1SDDAS_TUNE_STATE_LOAD_VALUE)
{
	SKYCDBG("%s start\n",__func__);
	
	pKTune_stream = kmalloc(scfg->cfg.tune_value.file_size, GFP_KERNEL);
	SKYCDBG("pKTune_stream =%x\n", pKTune_stream);
	if (!pKTune_stream) {
		SKYCERR("failed to malloc.\n");
		rc = -ENOMEM;
		goto update_fw_user_fail;
	}

	//유저인터페이스에서 튜닝 파일 스트림을 얻어옴	
	pUTune_stream = scfg->cfg.tune_value.pfile_stream;
	//페이지 단위 사이즈로 나누어 loop_count 저장
	loop_count = (scfg->cfg.tune_value.file_size / 4096) + 1; 
	last_size = scfg->cfg.tune_value.file_size % 4096;
	copy_size = 4096;

	SKYCDBG("scfg->cfg.tune_value.pfile_stream=%x, scfg->cfg.tune_value.file_size=%d\n", scfg->cfg.tune_value.pfile_stream, scfg->cfg.tune_value.file_size);
	for(i = 0; i < loop_count; i++)
	{
		//마지막 루프일 경우 남은 사이즈로 변경
		if(i == (loop_count-1))
			copy_size = last_size;
		else
			copy_size = 4096;
		
		if (copy_from_user(pKTune_stream + i*copy_size, (void *)pUTune_stream +  i*copy_size, copy_size))
		{                
			rc = -EFAULT;
			goto update_fw_user_fail;
        	}
		//SKYCDBG(" i =%d, loop_count=%d, copy_size=%d, pKTune_stream=%x\n", i, loop_count, copy_size, pKTune_stream);
	}

	//파일 스트림에서 개별 튜닝값을 배열에 로딩
	yacbac1sddas_init_params_tbl(pKTune_stream);

	yacbac1sddas_tup_state = YACBAC1SDDAS_TUNE_STATE_LOAD_VALUE;
}
	
#if 0	
	if (pKTune_stream)
		kfree(pKTune_stream);
#endif
	SKYCDBG("%s done.\n", __func__);
	return 0;

update_fw_user_fail:
	if (pKTune_stream)
	{
		kfree(pKTune_stream);
		pKTune_stream = NULL;
	}
	SKYCDBG("%s error. rc=%d\n", __func__, rc);
	return rc;
}
#endif


static int yacbac1sddas_probe_init_done( const struct msm_camera_sensor_info *data)
{
	SKYCDBG("%s start\n",__func__);
#ifdef F_SKYCAM_GPIO_SET
	yacbac1sddas_gpio_set(data->sensor_reset, 0);
#else
	gpio_direction_output(data->sensor_reset, 0);
	gpio_free(data->sensor_reset);
#endif
	SKYCDBG("%s end\n",__func__);
	
	return 0;
}

static int yacbac1sddas_sensor_init_probe(const struct msm_camera_sensor_info *data)
{
	int32_t rc;

	SKYCDBG("%s start\n", __func__);

	rc = yacbac1sddas_reset(data, ON);
	if (rc < 0) {
		SKYCERR("reset failed!\n");
		goto init_probe_fail;
	}

	mdelay(1);
#if 1 //리셋 이후에 standby pin 컨트롤을 해야 정상적인 standby mode로 전환 할 수 있음	
#ifdef F_SKYCAM_GPIO_SET
	yacbac1sddas_gpio_set(CAM_MAIN_STANDBY, ON);
#else
    	rc = gpio_request(CAM_MAIN_STANDBY, "yacbac1sddas");
    	if (!rc) {					
    		gpio_direction_output(CAM_MAIN_STANDBY, ON);
    		SKYCDBG("CAM_MAIN_STANDBY ON SUCCESS\n");
    	}
        else
    		SKYCERR("CAM_MAIN_STANDBY ON FAIL\n");     
#endif
#endif		

	mdelay(5);

	/* Need to fps regisetre setting */
#ifdef F_SKYCAM_TUP_LOAD_FILE
	if(yacbac1sddas_tup_mode == YACBAC1SDDAS_TUNE_STATE_TUNNING_MODE_ON)
		return rc;
#endif	

	rc = yacbac1sddas_reg_init();
	if (rc < 0)
		goto init_probe_fail;

	SKYCDBG("%s end\n",__func__);
	return rc;

init_probe_fail:
	SKYCERR("%s init_probe_fail\n",__func__);	
	return rc;
}


static int yacbac1sddas_sensor_init(const struct msm_camera_sensor_info *data)
{


	int32_t rc = 0;
	uint16_t fps = 0;

	SKYCDBG("%s start\n", __func__);
	
#ifdef F_SKYCAM_DRIVER_COMPLETE
	int cam_loop_count =0;
	do{
		mdelay(5);
		if(cam_loop_count++>100)
		{
		
			break;
		}
		SKYCDBG(" bBACK_cam_on wait %d \n",5*cam_loop_count);
	}while(bBACK_cam_on);
	
#endif	
	
	rc = yacbac1sddas_power(ON);
	if(rc)
	{
		rc = -ENOENT;
		SKYCERR(" yacbac1sddas_power failed rc = %d\n", rc);
		goto init_fail;
	}

	yacbac1sddas_ctrl = kzalloc(sizeof(struct yacbac1sddas_ctrl_t), GFP_KERNEL);
	if( !yacbac1sddas_ctrl)
	{
		SKYCERR("yacbac1sddas_init failed!\n");
		rc = -ENOMEM;
		goto init_fail;
	}


	if(data)
		yacbac1sddas_ctrl->sensordata = data;

	/* enable mclk first */
	msm_camio_clk_rate_set(YACBAC1SDDAS_DEFAULT_CLOCK_RATE);
	mdelay(20);

	msm_camio_camif_pad_reg_reset();
	mdelay(20);
	
	rc = yacbac1sddas_sensor_init_probe(data);
	if( rc < 0)
		goto init_fail;

#ifdef F_SKYCAM_DRIVER_COMPLETE	
	bFRONT_cam_on=true;
#endif

init_done:
	SKYCDBG("%s end\n", __func__);
	return rc;
	
init_fail:
	SKYCERR("%s init_fail\n", __func__);
	yacbac1sddas_probe_init_done(data);
	kfree(yacbac1sddas_ctrl);
	return rc;
}


static int yacbac1sddas_init_client(struct i2c_client *client)
{
	/* Initialize the MSM_CAMI2C Chip */
	init_waitqueue_head(&yacbac1sddas_wait_queue);
	return 0;
}


static int32_t yacbac1sddas_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cfg_data;
	int32_t rc = 0;

	if( copy_from_user(&cfg_data, (void *)argp, sizeof(struct sensor_cfg_data)))
		return -EFAULT;

	SKYCDBG("yacbac1sddas_ioctl, cfgtype = %d, mode %d\n", cfg_data.cfgtype, cfg_data.mode);

	//mutex_lock(&yacbac1sddas_mut);

	switch(cfg_data.cfgtype) // If need more function. Add the case
	{
		case CFG_SET_MODE:
			rc = yacbac1sddas_set_sensor_mode(cfg_data.mode);
			break;
#ifdef F_SKYCAM_FIX_CFG_EFFECT
		case CFG_SET_EFFECT:
			rc = yacbac1sddas_set_effect(cfg_data.cfg.effect);
			break;
#endif
#ifdef F_SKYCAM_FIX_CFG_BRIGHTNESS
		case CFG_SET_BRIGHTNESS:
			rc = yacbac1sddas_set_brightness(cfg_data.cfg.brightness);
			SKYCDBG("%s: CFG_SET_BRIGHTNESS, rc = %d\n", __func__, rc);
			break;
#endif
#ifdef F_SKYCAM_FIX_CFG_PREVIEW_FPS
		case CFG_SET_PREVIEW_FPS:
			rc = yacbac1sddas_set_preview_fps(cfg_data.cfg.preview_fps);
			SKYCDBG("%s: CFG_SET_PREVIEW_FPS, rc = %d\n", __func__, rc);
			break;
#endif
#ifdef F_SKYCAM_FIX_CFG_WB
		case CFG_SET_WB:
			SKYCDBG("%s   whitebalance=%d\n", __func__, cfg_data.cfg.wb);
			rc = yacbac1sddas_set_wb(cfg_data.cfg.wb);
			break;
#endif
#ifdef F_SKYCAM_FIX_CFG_ANTIBANDING
		case CFG_SET_ANTIBANDING:
			SKYCDBG("%s: CFG_SET_ANTIBANDING, rc=%d\n", __func__, rc);
			rc = yacbac1sddas_set_antibanding(cfg_data.cfg.antibanding);		
			break;
#endif			
#ifdef F_SKYCAM_TUP_LOAD_FILE
		case CFG_SET_TUNE_VALUE:
			SKYCDBG("%s: CFG_SET_TUNE_VALUE, rc=%d\n", __func__, rc);
			rc = yacbac1sddas_set_tune_value(&cfg_data);
			break;
#endif
#ifdef F_SKYCAM_FIX_CFG_EXPOSURE
		case CFG_SET_EXPOSURE_MODE:
			SKYCDBG("%s: CFG_SET_EXPOSURE_MODE, rc=%d\n", __func__, rc);			
			rc = yacbac1sddas_set_exposure_mode(cfg_data.cfg.exposure);
			break;
#endif
#ifdef F_SKYCAM_FIX_CFG_REFLECT
		case CFG_SET_REFLECT:
			SKYCDBG("%s: CFG_SET_REFLECT, rc=%d\n", __func__, rc);			
			rc = yacbac1sddas_set_reflect(cfg_data.cfg.reflect);
			break;
#endif

		default:
			rc = -EINVAL;
			break;
	}
	//mutex_unlock(&yacbac1sddas_mut);

	if (!rc)
		SKYCERR("%s succeeded! type=%d, mode=%d\n",
			__func__, cfg_data.cfgtype, cfg_data.mode);
	
	return rc;
}

int yacbac1sddas_sensor_release(void)
{
	int rc = -EBADF;
	SKYCDBG("%s is called\n", __func__);
	//mutex_lock(&yacbac1sddas_sem);

	yacbac1sddas_reset(yacbac1sddas_ctrl->sensordata, OFF);

	rc = yacbac1sddas_power(OFF);
	if(rc < 0)
		goto sensor_release_fail;

	kfree(yacbac1sddas_ctrl);
	yacbac1sddas_ctrl = NULL;

#ifdef F_SKYCAM_TUP_LOAD_FILE
	//튜닝값 로딩 체크 flag
	yacbac1sddas_tup_state = YACBAC1SDDAS_TUNE_STATE_NONE;
	//yacbac1sddas_done_set_tune_load = FALSE;
#endif

	//mutex_lock(&yacbac1sddas_sem);

	SKYCDBG("%s done.\n", __func__);
#ifdef F_SKYCAM_DRIVER_COMPLETE	
	bFRONT_cam_on=false;
#endif	
	return 0;

sensor_release_fail:
	SKYCERR("%s failed! (%d)\n", __func__, rc);	
	return rc;
}


static int yacbac1sddas_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

	int rc = 0;
	SKYCDBG("yacbac1sddas_probe called\n");

	if ( !i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
		SKYCERR("i2c_check_faunctionality failed\n");
		goto probe_failure;
	}

	yacbac1sddas_sensorw = kzalloc(sizeof(struct yacbac1sddas_work), GFP_KERNEL);
	if( !yacbac1sddas_sensorw)
	{
		SKYCERR("kzalloc failed.\n");
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, yacbac1sddas_sensorw);
	yacbac1sddas_init_client(client);
	yacbac1sddas_client = client;
	mdelay(50);

	SKYCDBG("yacbac1sddas_probe successed! rc = %d\n", rc);
	return 0;

probe_failure:
	SKYCERR("yacbac1sddas_probe failed! rc = %d\n" ,rc);
	return rc;
	
}

static const struct i2c_device_id yacbac1sddas_i2c_id[] = 
{
		{"yacbac1sddas", 0 },
		{ },
};

static struct i2c_driver yacbac1sddas_i2c_driver = {
	.id_table = yacbac1sddas_i2c_id,
	.probe = yacbac1sddas_i2c_probe,
	.remove = __exit_p(yacbac1sddas_i2c_remove),
	.driver = {	
		.name = "yacbac1sddas",
	},
};

/* Need to check this module  */

static int32_t yacbac1sddas_init_i2c(void)
{
	int32_t rc = 0;
	SKYCDBG("%s start\n", __func__);

	rc = i2c_add_driver(&yacbac1sddas_i2c_driver);
	SKYCDBG("%s rc = %d\n",__func__, rc);
	if( IS_ERR_VALUE(rc))
		goto init_i2c_fail;

	SKYCDBG("%s end\n", __func__);
	return 0;

init_i2c_fail:
	SKYCERR("%s failed! (%d)\n", __func__, rc);
	return rc;
}

static int yacbac1sddas_sensor_probe(const struct msm_camera_sensor_info *info, struct msm_sensor_ctrl *s)
{
#if 0
	int rc = 0;
	SKYCDBG("%s start",__func__);

#if 1
	rc =yacbac1sddas_init_i2c(); // need to i2c initialize rotuine
	if( rc < 0 || yacbac1sddas_client == NULL)
	{
		SKYCERR("%s rc = %d, yacbac1sddas_client = %d\n", __func__, rc, yacbac1sddas_client);
		goto probe_init_fail;
	}
#else
	rc = i2c_add_driver(&yacbac1sddas_i2c_driver);
	if ( rc < 0 || yacbac1sddas_client == NULL)
	{
		rc = -ENOTSUPP;
		goto probe_done;
	}
#endif


	rc = yacbac1sddas_power(ON); // Make VGA camera power module
	if (rc)
		goto probe_init_fail;

	msm_camio_clk_rate_set(YACBAC1SDDAS_DEFAULT_CLOCK_RATE);
	mdelay(20);

	rc = yacbac1sddas_sensor_init_probe(info);
	if( rc < 0)
		goto probe_init_fail;

	s->s_init = yacbac1sddas_sensor_init;
	s->s_release = yacbac1sddas_sensor_release;
	s->s_config = yacbac1sddas_sensor_config;
	//yacbac1sddas_probe_init_done(info);

	rc = yacbac1sddas_power(OFF);
	SKYCDBG("%s successed!\n", __func__);
	return 0;

probe_init_fail:
	rc = yacbac1sddas_power(OFF);
	SKYCERR("%s failed!\n", __func__);
	return rc;
#else
	int rc = 0;
	SKYCDBG("%s start\n",__func__);

	rc =yacbac1sddas_init_i2c(); // need to i2c initialize rotuine
	if( rc < 0 || yacbac1sddas_client == NULL)
	{
		SKYCERR("%s rc = %d, yacbac1sddas_client = %d\n", __func__, rc, yacbac1sddas_client);
		goto probe_init_fail;
	}

#if 0 // for i2c test
	rc = yacbac1sddas_power(ON); // Make VGA camera power module
	if (rc)
		goto probe_init_fail;

	msm_camio_clk_rate_set(YACBAC1SDDAS_DEFAULT_CLOCK_RATE);
	mdelay(20);

	rc = yacbac1sddas_sensor_init_probe(info);
	if( rc < 0)
		goto probe_init_fail;
	yacbac1sddas_probe_init_done(info);
#if 0
	rc = yacbac1sddas_power(OFF);
#endif
	SKYCDBG("%s successed!\n", __func__);
#endif
	s->s_init = yacbac1sddas_sensor_init;
	s->s_release = yacbac1sddas_sensor_release;
	s->s_config = yacbac1sddas_sensor_config;
	s->s_mount_angle  = 90;
	//yacbac1sddas_probe_init_done(info);
	return 0;

probe_init_fail:
#if 0
	rc = yacbac1sddas_power(OFF);
#endif
	SKYCERR("%s failed!\n", __func__);
	return rc;
#endif
}
	
	

static int __yacbac1sddas_probe(struct platform_device *pdev)
{
	SKYCDBG("%s start\n", __func__);
	return msm_camera_drv_start(pdev,yacbac1sddas_sensor_probe);
}

static struct platform_driver msm_camera_driver = {
	.probe = __yacbac1sddas_probe,
	.driver = {
	  	.name ="msm_camera_yacbac1sddas",
		.owner = THIS_MODULE,
		},
};

static int __init yacbac1sddas_init(void)
{
	SKYCDBG("%s start\n", __func__);
	return platform_driver_register(&msm_camera_driver);
}

void yacbac1sddas_exit(void)
{
	SKYCDBG("%s is called.\n", __func__);
	i2c_del_driver(&yacbac1sddas_i2c_driver);
}

module_init(yacbac1sddas_init);
module_exit(yacbac1sddas_exit);
