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
#include "mt9t111.h"
#include <mach/vreg.h>

#ifdef F_SKYCAM_FIX_CFG_LED_MODE
#include "../smd_rpcrouter.h"
#include "../proc_comm.h"
#endif
#ifdef F_SKYCAM_TUP_LOAD_FILE
#include "camsensor_mt9t111_tuneup.h"
#endif
#define F_SKYCAM_INIT_ON 0
#define F_SKYCAM_LED_SET 1
/* Micron MT9T111 Registers and their values */
/* Sensor Core Registers */
#define MT9T111_REG_MCU_ADDR      0x098E
#define MT9T111_REG_MCU_DATA      0x0990
#define  REG_MT9T111_MODEL_ID 0x3000
//#define  MT9T111_MODEL_ID     0x1580

/*  SOC Registers Page 1  */
/*#define  REG_MT9T111_SENSOR_RESET     0x301A
#define  REG_MT9T111_STANDBY_CONTROL  0x3202
#define  REG_MT9T111_MCU_BOOT         0x3386
*/

#define SENSOR_DEBUG 0

//#define MT9T111_I2C_2

#define MT9T111_I2C_RETRY	2
#define MT9T111_I2C_MPERIOD	200
#define MT9T111_SNAPSHOT_RETRY 	200
#define MT9T111_PREVIEW_RETRY 	50
#ifdef F_SKYCAM_FIX_CFG_REFLECT
#define MT9T111_POLLING_RETRY	 	30
#endif

#define MT9T111_DEFAULT_CLOCK_RATE 24000000

#ifdef F_SKYCAM_DRIVER_COMPLETE
bool bBACK_cam_on = false;
EXPORT_SYMBOL(bBACK_cam_on);
extern bFRONT_cam_on;
#endif

struct mt9t111_work {
	struct work_struct work;
};

static struct  mt9t111_work *mt9t111_sensorw;
static struct  i2c_client *mt9t111_client;

struct mt9t111_ctrl_t {
	const struct msm_camera_sensor_info *sensordata;
};

static int multi_count;
static struct mt9t111_ctrl_t *mt9t111_ctrl = NULL;

static DECLARE_WAIT_QUEUE_HEAD(mt9t111_wait_queue);
DECLARE_MUTEX(mt9t111_sem);
static int16_t mt9t111_effect = CAMERA_EFFECT_OFF;

struct mt9t111_vreg_t {
	const char *name;
	unsigned short mvolt;
};

#define ON	1
#define OFF	0
#define CAM_LDO_EN 22	// 2.8 CAM_A
#define CAM_3M_STANDBY 3
#define CAM_VGA_STANDBY 96



static struct mt9t111_vreg_t mt9t111_vreg[] = {	
	{
		.name = "rftx",	/* 1.8V_CAM --> VREG_GP1-CAM (out14)*/
		.mvolt = 1800,       
	},
	{
		.name = "gp4",	/* 2.8V_CAM_IO --> VREG_GP4-AUX1 (out9)*/
		.mvolt = 2800,       
	},
	{
		.name = "gp1",	/* 2.8V_CAM_AF --> VREG_RUIM1 (out15)*/
		.mvolt = 2800,
	}
};

#ifdef F_SKYCAM_FIX_CFG_LED_MODE
#define PM_LIBPROG	0x30000061
#define PM_LIBVERS	0x00010001


#define PM_FLASH_LED_SET_CURRENT_PROC 17 
#define PM_FLASH_LED_SET_MODE_PROC 18 
#define PM_FLASH_LED_SET_POLARITY_PROC 19 
#define PM_SECURE_MPP_CONFIG_DIGITAL_INPUT_PROC 44
static struct msm_rpc_endpoint *pm_lib_ep;
static int CompVer = PM_LIBVERS;

int led_auto;

typedef enum
{
  PM_MPP_1,
  PM_MPP_2,
  PM_MPP_3,
  PM_MPP_4,
  PM_MPP_5,
  PM_MPP_6,
  PM_MPP_7,
  PM_MPP_8,
  PM_MPP_9,
  PM_MPP_10,
  PM_MPP_11,
  PM_MPP_12,
  PM_MPP_13,
  PM_MPP_14,
  PM_MPP_15,
  PM_MPP_16,
  PM_MPP_17,
  PM_MPP_18,
  PM_MPP_19,
  PM_MPP_20,
  PM_MPP_21,
  PM_MPP_22,
  PM_MPP_INVALID,
  PM_NUM_MPP_HAN = PM_MPP_4 + 1,
  PM_NUM_MPP_KIP = PM_MPP_4 + 1,
  PM_NUM_MPP_EPIC = PM_MPP_4 + 1,
  PM_NUM_MPP_PM7500 = PM_MPP_22 + 1,  /* Max number of MPP's for PM7500 */
  PM_NUM_MPP_PM6650 = PM_MPP_12 + 1,  /* Max number of MPP's for PM6650 */
  PM_NUM_MPP_PM6658 = PM_MPP_12 + 1,
  PM_NUM_MPP_PANORAMIX = PM_MPP_2 + 1,/* Max number of MPP's for PANORAMIX and PM6640 */
  PM_NUM_MPP_PM6640 = PM_NUM_MPP_PANORAMIX,
  PM_NUM_MPP_PM6620 = PM_NUM_MPP_PANORAMIX
}pm_mpp_which_type;

typedef enum
{
  PM_MPP__DLOGIC__LVL_MSME,
  PM_MPP__DLOGIC__LVL_MSMP,
  PM_MPP__DLOGIC__LVL_RUIM,
  PM_MPP__DLOGIC__LVL_MMC,
  PM_MPP__DLOGIC__LVL_VDD,
  PM_MPP__DLOGIC__LVL_INVALID
}pm_mpp_dlogic_lvl_type;

typedef enum
{
  PM_MPP__DLOGIC_IN__DBUS_NONE,
  PM_MPP__DLOGIC_IN__DBUS1,
  PM_MPP__DLOGIC_IN__DBUS2,
  PM_MPP__DLOGIC_IN__DBUS3,
  PM_MPP__DLOGIC_IN__DBUS_INVALID
}pm_mpp_dlogic_in_dbus_type;

typedef enum
{
  PM_FLASH_LED_MODE__MANUAL,
  PM_FLASH_LED_MODE__DBUS1,
  PM_FLASH_LED_MODE__DBUS2,
  PM_FLASH_LED_MODE__DBUS3,
  PM_FLASH_LED_MODE__INVALID
}pm_flash_led_mode_type;

typedef enum
{
  PM_FLASH_LED_POL__ACTIVE_HIGH,
  PM_FLASH_LED_POL__ACTIVE_LOW,
  PM_FLASH_LED_POL__INVALID
}pm_flash_led_pol_type;

typedef struct {	
	struct rpc_request_hdr	hdr;
	uint32_t	val;	// cp use 1byte, but we have to use 4bytes, WHY?
} msm_pm_set_flash_current_req;

typedef struct {	
	struct rpc_request_hdr	hdr;
	uint32_t mode;	// cp use 1byte, but we have to use 4bytes, WHY?
} msm_pm_set_flash_mode_req;

typedef struct {	
	struct rpc_request_hdr	hdr;
	uint32_t	pol;	// cp use 1byte, but we have to use 4bytes, WHY?
} msm_pm_set_flash_polality_req;

typedef struct {
	struct rpc_request_hdr	hdr;
	uint32_t	mpp;
	uint32_t	level;	
	uint32_t	dbus;	
} msm_pm_secure_mpp_config_digital_input_req;

typedef struct {
	struct rpc_reply_hdr	hdr;
	uint32_t	err_flag;
} msm_pm_set_flash_current_rep;
#endif

#ifdef F_SKYCAM_TUP_LOAD_FILE
static mt9t111_tune_state_type mt9t111_tup_state = MT9T111_TUNE_STATE_NONE;
static mt9t111_tune_mode_type mt9t111_tup_mode = MT9T111_TUNE_STATE_TUNNING_MODE_ON;
static mt9t111_params_tbl_t mt9t111_params_tbl;
#endif
/*=============================================================
	EXTERNAL DECLARATIONS
==============================================================*/
extern struct mt9t111_reg mt9t111_regs;

/*
================================================================================
LOCAL API DECLARATIONS
================================================================================
*/
static int32_t mt9t111_i2c_read_word(unsigned short raddr, unsigned short *rdata);
#ifdef F_SKYCAM_TUP_LOAD_FILE
static u_int32_t mt9t111_i2c_write_params
(
    mt9t111_cfg_param_t *params,
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
static bool mt9t111_gpio_set(unsigned int gpio_num, bool gpio_on)
{
	int rc =0;
    	rc = gpio_request(gpio_num, "mt9t111");
    	if (!rc) {					
    		gpio_direction_output(gpio_num, gpio_on);
    		SKYCDBG("GPIO [%d] SET SUCCESS\n", gpio_num);
		gpio_free(gpio_num);
    	}
        else
        {
    		SKYCERR("GPIO [%d] SET FAIL, retry\n", gpio_num);        
		gpio_free(gpio_num);
		
	    	rc = gpio_request(gpio_num, "mt9t111");
	    	if (!rc) {					
	    		gpio_direction_output(gpio_num, gpio_on);
	    		SKYCDBG("GPIO [%d] SET SUCCESS\n", gpio_num);
			gpio_free(gpio_num);
	    	}
	        else
	    		SKYCERR("GPIO [%d] SET FAIL\n", gpio_num);    
	 }	
}
#endif

static int mt9t111_reset(const struct msm_camera_sensor_info *dev,  int set)
{
	int rc = 0;

	rc = gpio_request(dev->sensor_reset, "mt9t111");
		SKYCDBG("[2.5_gpio_request(dev->sensor_reset) = %d]\n", dev->sensor_reset);

	if (!rc) {
		rc = gpio_direction_output(dev->sensor_reset, 0);
			SKYCDBG("[3_reset pin = %d, value = LOW]\n", dev->sensor_reset);
		//mdelay(20);
		if(set)
		{
		rc = gpio_direction_output(dev->sensor_reset, set);
			SKYCDBG("[4_reset pin = %d, value = HIGH]\n", dev->sensor_reset);
		}
		gpio_free(dev->sensor_reset);
	}

	//if(set == OFF && dev->sensor_reset)

	return rc;
}

static int32_t mt9t111_i2c_txdata(unsigned short saddr,
	unsigned char *txdata, int length)
{
	uint32_t i = 0;
	int32_t rc = 0;
	
	struct i2c_msg msg[] = {
		{
			.addr = saddr,
			.flags = 0,
			.len = length,
			.buf = txdata,
		},
	};

#if SENSOR_DEBUG
	if (length == 2)
		SKYCDBG("msm_io_i2c_w: 0x%04x 0x%04x\n",
			*(u16 *) txdata, *(u16 *) (txdata + 2));
	else if (length == 4)
		SKYCDBG("msm_io_i2c_w: 0x%04x\n", *(u16 *) txdata);
	else
		SKYCDBG("msm_io_i2c_w: length = %d\n", length);
#endif
#if 0
	if (i2c_transfer(mt9t111_client->adapter, msg, 1) < 0) {
		SKYCDBG("mt9t111_i2c_txdata failed\n");
		return -EIO;
	}

	return 0;
#endif
	for (i = 0; i < MT9T111_I2C_RETRY; i++) {
		rc = i2c_transfer(mt9t111_client->adapter, msg, 1);
		if (rc >= 0) {			
			return 0;
		}
		SKYCERR("%s: tx retry. [%02x.%02x.%02x] len=%d rc=%d\n", __func__,saddr, *txdata, *(txdata + 1), length, rc);
		msleep(MT9T111_I2C_MPERIOD);
	}
	return rc;
	//return -EIO;
}

static int32_t mt9t111_i2c_write(unsigned short saddr,
	unsigned short waddr, unsigned short wdata, enum mt9t111_width width)
{
	int32_t rc = -EIO;
	unsigned char buf[4];

	memset(buf, 0, sizeof(buf));
	switch (width) {
	case WORD_LEN: {
#ifdef MT9T111_I2C_2
		buf[0] = (0x098E & 0xFF00)>>8;
		buf[1] = (0x098E & 0x00FF);
		buf[2] = (waddr & 0xFF00)>>8;
		buf[3] = (waddr & 0x00FF);

		rc = mt9t111_i2c_txdata(saddr, buf, 4);

		buf[0] = (0x0990 & 0xFF00)>>8;
		buf[1] = (0x0990 & 0x00FF);
		buf[2] = (wdata & 0xFF00)>>8;
		buf[3] = (wdata & 0x00FF);

		rc = mt9t111_i2c_txdata(saddr, buf, 4);

#else
		buf[0] = (waddr & 0xFF00)>>8;
		buf[1] = (waddr & 0x00FF);
		buf[2] = (wdata & 0xFF00)>>8;
		buf[3] = (wdata & 0x00FF);

		rc = mt9t111_i2c_txdata(saddr, buf, 4);
#endif
	}
		break;

	case TRIPLE_LEN: {
		buf[0] = (waddr & 0xFF00)>>8;
		buf[1] = (waddr & 0x00FF);
		buf[2] = wdata;
		rc = mt9t111_i2c_txdata(saddr, buf, 3);
	}
		break;

	case BYTE_LEN: {
		buf[0] = waddr;
		buf[1] = wdata;
		rc = mt9t111_i2c_txdata(saddr, buf, 2);
	}
		break;

	default:
		break;
	}

	if (rc < 0)
		SKYCERR(
		"i2c_write failed, saddr= 0x%x, addr = 0x%x, val = 0x%x!\n",
		saddr, waddr, wdata);

	return rc;
}

static int32_t mt9t111_i2c_write_word(unsigned short waddr, unsigned short wdata)
{
	int32_t rc = -EIO;
	unsigned char buf[4];

	memset(buf, 0, sizeof(buf));

#ifdef MT9T111_I2C_2
	buf[0] = (0x098E & 0xFF00)>>8;
	buf[1] = (0x098E & 0x00FF);
	buf[2] = (waddr & 0xFF00)>>8;
	buf[3] = (waddr & 0x00FF);

	rc = mt9t111_i2c_txdata(mt9t111_client->addr, buf, 4);

	buf[0] = (0x0990 & 0xFF00)>>8;
	buf[1] = (0x0990 & 0x00FF);
	buf[2] = (wdata & 0xFF00)>>8;
	buf[3] = (wdata & 0x00FF);

	rc = mt9t111_i2c_txdata(mt9t111_client->addr, buf, 4);
#else
	buf[0] = (waddr & 0xFF00)>>8;
	buf[1] = (waddr & 0x00FF);
	buf[2] = (wdata & 0xFF00)>>8;
	buf[3] = (wdata & 0x00FF);

	rc = mt9t111_i2c_txdata(mt9t111_client->addr, buf, 4);
#endif

#ifdef F_SKYCAM_SENSOR_TUNEUP	
	//SKYCDBG("<<[WRITE WORD!] waddr=0x%x, wdata=0x%x\n", waddr, wdata);
#endif
	if (rc < 0)
		SKYCERR(
		"i2c_write failed, saddr= 0x%x, addr = 0x%x, val = 0x%x!\n",
		mt9t111_client->addr, waddr, wdata);

	return rc;
}

static int32_t mt9t111_i2c_write_a2d1(unsigned short waddr, unsigned char wdata)
{
	int32_t rc = -EIO;
	unsigned char buf[3];

	memset(buf, 0, sizeof(buf));

	buf[0] = (waddr & 0xFF00)>>8;
	buf[1] = (waddr & 0x00FF);
	buf[2] = wdata;
	
	rc = mt9t111_i2c_txdata(mt9t111_client->addr, buf, 3);
#ifdef F_SKYCAM_SENSOR_TUNEUP		
	//SKYCDBG("<<[=WRITE A2D1!=] waddr=0x%x, bdata=0x%x\n", waddr, wdata);
#endif
	if (rc < 0)
	{
		SKYCERR(
		"i2c_write failed, saddr= 0x%x, addr = 0x%x, val = 0x%x!\n",
		mt9t111_client->addr, waddr, wdata);
		rc = mt9t111_i2c_txdata(mt9t111_client->addr, buf, 3);
		if(rc < 0)
			SKYCERR(
			"Retry error i2c_write failed, saddr= 0x%x, addr = 0x%x, val = 0x%x!\n",
			mt9t111_client->addr, waddr, wdata);
	}
	return rc;
}

#ifdef CAMSENSOR_BURSTMODE
static int32_t mt9t111_i2c_write_8words(unsigned short waddr, unsigned short* wdata, unsigned short width)
{
	int32_t rc = -EIO;
	unsigned char buf[18]; //addr 2 bytes + data 16 bytes
	int byte_cnt, i=0, buf_loop_cnt=2;

	byte_cnt = width*2+2; //addr 2 bytes + data 16 bytes
	memset(buf, 0, sizeof(buf));

	buf[0] = (waddr & 0xFF00)>>8;
	buf[1] = (waddr & 0x00FF);
	buf_loop_cnt=2; 
	for(i=0; i<width; i++)
	{
		buf[buf_loop_cnt++] = ((*wdata) & 0xFF00)>>8;
		buf[buf_loop_cnt++] = ((*wdata) & 0x00FF);
		wdata++;
	}
	
	rc = mt9t111_i2c_txdata(mt9t111_client->addr, buf, byte_cnt);

#ifdef F_SKYCAM_SENSOR_TUNEUP	
	//SKYCDBG("<<[WRITE WORD!] waddr=0x%x, wdata=0x%x\n", waddr, wdata);
#endif
	if (rc < 0)
		SKYCERR(
		"i2c_write failed, saddr= 0x%x, addr = 0x%x, val = 0x%x!\n",
		mt9t111_client->addr, waddr, wdata);

	return rc;
}

static int32_t mt9t111_i2c_write_burst_table(
	struct mt9t111_i2c_burst_set const *reg_conf_tbl,
	int num_of_items_in_table)
{
	int i;
	int32_t rc = -EIO;
	
	for (i = 0; i < num_of_items_in_table; i++)
	{
	//addr 레지스터 쓰고
		rc = mt9t111_i2c_write_word(reg_conf_tbl->burst_addr.waddr, 
									reg_conf_tbl->burst_addr.wdata);
		if (rc < 0)
			SKYCERR(
			"i2c_write failed, saddr= 0x%x, addr = 0x%x, val = 0x%x!\n",
			mt9t111_client->addr, reg_conf_tbl->burst_addr.waddr, 
								reg_conf_tbl->burst_addr.wdata);
	//data 레지스터
		rc = mt9t111_i2c_write_8words(reg_conf_tbl->burst_data.waddr, 
	    								reg_conf_tbl->burst_data.wdata, 
	    								reg_conf_tbl->burst_data.width);
		if (rc < 0)
			SKYCERR(
			"i2c_write failed, saddr= 0x%x, addr = 0x%x, val = 0x%x!width[%d]\n",
			mt9t111_client->addr, reg_conf_tbl->burst_data.waddr, 
								reg_conf_tbl->burst_data.wdata,
								reg_conf_tbl->burst_data.width);
		reg_conf_tbl++;
	}
	return rc;
}
#endif

static int32_t mt9t111_i2c_write_table(
	struct mt9t111_i2c_reg_conf const *reg_conf_tbl,
	int num_of_items_in_table)
{
	int i;
	int32_t rc = -EIO;

	uint16_t poll_delay = 0;
       uint16_t poll_retry = 0;
       uint16_t poll_mcu_var = 0;
       uint16_t poll_data = 0;
       uint16_t poll_mask = 0;
       uint16_t retry_cnt = 0;
	uint16_t read_data = 0;

	for (i = 0; i < num_of_items_in_table; i++) 
	{		
#if 0 //polling	
		if(reg_conf_tbl->width == ZERO_LEN)
		{
			SKYCDBG("ZERO_LEN continue ADDR = 0x%x, VALUE = 0x%x\n",reg_conf_tbl->waddr, reg_conf_tbl->wdata);
			continue;
		}

		if(reg_conf_tbl->width == POLL_MCU_VAR)
		{
#if 0		
			poll_delay = (reg_conf_tbl[i+1].wdata & 0xff00) >> 8;
                     poll_retry = (reg_conf_tbl[i+1].wdata & 0x00ff);
	              poll_mcu_var = reg_conf_tbl[i].waddr;
	              poll_data = reg_conf_tbl[i+1].waddr;
	              poll_mask = reg_conf_tbl[i].wdata;
#else
			poll_mcu_var = reg_conf_tbl->waddr;
   		       poll_mask = reg_conf_tbl->wdata;	              
	              poll_data = (reg_conf_tbl+1)->waddr;
			poll_delay = ((reg_conf_tbl+1)->wdata & 0xff00) >> 8;
                     poll_retry = ((reg_conf_tbl+1)->wdata & 0x00ff);              
#endif

		       SKYCDBG("POLLING!! poll_delay=%x, poll_retry=%x, poll_mcu_var=%x, poll_data=%x, poll_mask=%x\n",poll_delay, poll_retry, poll_mcu_var, poll_data, poll_mask);
				  
			for (retry_cnt = 0; retry_cnt < poll_retry; retry_cnt++)
                	{
                		if (mt9t111_i2c_write_word(MT9T111_REG_MCU_ADDR, poll_mcu_var) < 0)
	                    {
	                        SKYCDBG("<<POLL_MCU_VAR mt9t111_i2c_write_word (-EIO)");
	                        return FALSE;
	                    }

	                    if (mt9t111_i2c_read_word(MT9T111_REG_MCU_DATA, &read_data) < 0)
	                    {
	                        SKYCDBG("<<POLL_MCU_VAR mt9t111_i2c_read_word (FALSE)");
	                        return FALSE;
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
	                    SKYCDBG("<<RETRY FAIL!! read_data = %x (FALSE)", read_data, 0, 0);
	                    return FALSE;
	              }

			//  reg_conf_tbl used 2 value, so add 2
			reg_conf_tbl++;
			reg_conf_tbl++;
		}
		else
		{
			rc = mt9t111_i2c_write(mt9t111_client->addr,
								reg_conf_tbl->waddr, reg_conf_tbl->wdata,
								reg_conf_tbl->width);

			if (rc < 0)
			{
				SKYCDBG("mt9t111_i2c_read failed!\n");
				break;
			}
#if 0		
			if((reg_conf_tbl->waddr == 0x337E)|| 
			((reg_conf_tbl->waddr == 0x098E)&&(reg_conf_tbl->wdata == 0xA401))||
			(reg_conf_tbl->waddr == 0xA401)||
			(reg_conf_tbl->waddr == 0xA805)||
			(reg_conf_tbl->waddr == 0xA409)||
			(reg_conf_tbl->waddr == 0x8404))
			SKYCDBG("brightness WRITE!!! ADDR = 0x%x, VALUE = 0x%x, width = %d\n",reg_conf_tbl->waddr, reg_conf_tbl->wdata, reg_conf_tbl->width);
#endif		
			if (reg_conf_tbl->mdelay_time != 0)
				mdelay(reg_conf_tbl->mdelay_time);

			reg_conf_tbl++;
		}
#else	
	switch(reg_conf_tbl->width )
	{
		case ZERO_LEN:
		{
			SKYCDBG("ZERO_LEN continue ADDR = 0x%x, VALUE = 0x%x\n",reg_conf_tbl->waddr, reg_conf_tbl->wdata);
			reg_conf_tbl++;			
			continue;
		}		
		case POLL_MCU_VAR:
		{
			poll_mcu_var = reg_conf_tbl->waddr;
   		       poll_mask = reg_conf_tbl->wdata;	              
	              poll_data = (reg_conf_tbl+1)->waddr;
			poll_delay = ((reg_conf_tbl+1)->wdata & 0xff00) >> 8;
                     poll_retry = ((reg_conf_tbl+1)->wdata & 0x00ff);              

		       SKYCDBG("POLLING!! poll_delay=%x, poll_retry=%x, poll_mcu_var=%x, poll_data=%x, poll_mask=%x\n",poll_delay, poll_retry, poll_mcu_var, poll_data, poll_mask);
				  
			for (retry_cnt = 0; retry_cnt < poll_retry; retry_cnt++)
                	{
                		rc = mt9t111_i2c_write_word(MT9T111_REG_MCU_ADDR, poll_mcu_var);
				if(rc<0) {
	                        SKYCERR("<<POLL_MCU_VAR mt9t111_i2c_write_word (-EIO)\n");
	                        return -EIO;
	                     }

	                     rc = mt9t111_i2c_read_word(MT9T111_REG_MCU_DATA, &read_data);
	                     if(rc<0) {
	                        SKYCERR("<<POLL_MCU_VAR mt9t111_i2c_read_word (-EIO)\n");
	                        return -EIO;
	                     }
	                    
	                    if ((read_data & poll_mask) != poll_data)
	                    {
	                        SKYCDBG("retry polling MCU variable... after sleeping %d ms", poll_delay);
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
	                    SKYCDBG("<<RETRY FAIL!! read_data = %x (-EIO)", read_data);
			      //return -EIO;
	              }

			// reg_conf_tbl used 2 value, so add 2
			reg_conf_tbl++;
			reg_conf_tbl++;
			i++;

			break;
		}
		case POLL_MCU_VAR_NOT:
		{
			poll_mcu_var = reg_conf_tbl->waddr;
   		       poll_mask = reg_conf_tbl->wdata;	              
	              poll_data = (reg_conf_tbl+1)->waddr;
			poll_delay = ((reg_conf_tbl+1)->wdata & 0xff00) >> 8;
                     poll_retry = ((reg_conf_tbl+1)->wdata & 0x00ff);              

		       SKYCDBG("POLLING!! poll_delay=%x, poll_retry=%x, poll_mcu_var=%x, poll_data=%x, poll_mask=%x\n",poll_delay, poll_retry, poll_mcu_var, poll_data, poll_mask);
				  
			for (retry_cnt = 0; retry_cnt < poll_retry; retry_cnt++)
                	{
                	      if (mt9t111_i2c_write_word(MT9T111_REG_MCU_ADDR, poll_mcu_var) < 0)
	                    {
	                        SKYCERR("<<POLL_MCU_VAR mt9t111_i2c_write_word (-EIO)\n");
	                        return -EIO;
	                    }

	                    if (mt9t111_i2c_read_word(MT9T111_REG_MCU_DATA, &read_data) < 0)
	                    {
	                        SKYCERR("<<POLL_MCU_VAR mt9t111_i2c_read_word (-EIO)\n");
	                        return -EIO;
	                    }

	                    if ((read_data & poll_mask) == poll_data)
	                    {
	                        SKYCDBG("retry polling MCU variable... after sleeping %d ms, read_data=%2x\n", poll_delay, read_data, 0);
	                        mdelay(poll_delay);
	                    }
	                    else
	                    {
	                        SKYCDBG("stop polling MCU variable... retried %d/%d time(s) (delay = %d ms), read_data=%2x\n", retry_cnt, poll_retry, poll_delay, read_data);
	                        break;
	                    }
			}

			if (retry_cnt == poll_retry)
	              {
	                    SKYCDBG("<<RETRY FAIL!! read_data = %x (-EIO)\n", read_data, 0, 0);
	                    //return -EIO;
	              }

			//  2개의 값을 이용하므로 +2를 해준다
			reg_conf_tbl++;
			reg_conf_tbl++;
			i++;

			break;
		}
		case BURST_WORD_START:
#ifdef CAMSENSOR_BURSTMODE			
			rc = mt9t111_i2c_write_burst_table(mt9t111_regs.init_patch_burst_settings, 
												mt9t111_regs.init_patch_burst_settings_size);
#endif
			if (rc < 0)
			{
				SKYCERR("mt9t111_i2c_read failed!\n");
				break;
			}

			if((++reg_conf_tbl)->width == BURST_WORD_END)
			{
				reg_conf_tbl++;
				SKYCDBG("<<BURST_WORD_END SUCCESS!!reg_conf_tbl.waddr=%x, reg_conf_tbl.wdata=%x\n", reg_conf_tbl->waddr, reg_conf_tbl->wdata);								
			}
			else
			{
				reg_conf_tbl++;
				reg_conf_tbl++;
				SKYCDBG("<<BURST_WORD_END FAIL!!reg_conf_tbl.waddr=%x, reg_conf_tbl.wdata=%x\n", reg_conf_tbl->waddr, reg_conf_tbl->wdata);				
			}
			i++;
			break;		

		default:
		{
			rc = mt9t111_i2c_write(mt9t111_client->addr,
								reg_conf_tbl->waddr, reg_conf_tbl->wdata,
								reg_conf_tbl->width);
			//SKYCDBG("I2C WRITE!!! ADDR = 0x%x, VALUE = 0x%x, width = %d, num_of_items_in_table=%d, i=%d\n",
			//	reg_conf_tbl->waddr, reg_conf_tbl->wdata, reg_conf_tbl->width, num_of_items_in_table, i);

			if (rc < 0)
			{
				SKYCERR("mt9t111_i2c_read failed!\n");
				return rc;
			}
		
			if (reg_conf_tbl->mdelay_time != 0)
				mdelay(reg_conf_tbl->mdelay_time);

			reg_conf_tbl++;

			break;
		}			
	}
#endif		
	}

	return rc;
}


static int mt9t111_i2c_rxdata(unsigned short saddr, int slength,
	unsigned char *rxdata, int length)
{
	uint32_t i = 0;
	int32_t rc = 0;
	
	struct i2c_msg msgs[] = {
	{
		.addr   = saddr,
		.flags = 0,
		.len   = slength,//   2, 
		.buf   = rxdata,
	},
	{
		.addr   = saddr,
		.flags = I2C_M_RD,
		.len   = length,
		.buf   = rxdata,
	},
	};

#if SENSOR_DEBUG
	if (length == 2)
		SKYCDBG("msm_io_i2c_r: 0x%04x 0x%04x\n",
			*(u16 *) rxdata, *(u16 *) (rxdata + 2));
	else if (length == 4)
		SKYCDBG("msm_io_i2c_r: 0x%04x\n", *(u16 *) rxdata);
	else
		SKYCDBG("msm_io_i2c_r: length = %d\n", length);
#endif
#if 0
	if (i2c_transfer(mt9t111_client->adapter, msgs, 2) < 0) {
		SKYCDBG("mt9t111_i2c_rxdata failed!\n");
		return -EIO;
	}

	return 0;
#endif
	for (i = 0; i < MT9T111_I2C_RETRY; i++) {
		rc = i2c_transfer(mt9t111_client->adapter, msgs, 2);
		if (rc >= 0) {			
			return 0;
		}
		SKYCERR("%s: tx retry. [%02x.%02x] len=%d rc=%d\n", __func__,saddr, *rxdata, length, rc);
		msleep(MT9T111_I2C_MPERIOD);
	}
	//SKYCDBG("%s end\n",__func__);
	return rc; //-EIO;//0;
}

static int32_t mt9t111_i2c_read(unsigned short   saddr,
	unsigned short raddr, unsigned short *rdata, enum mt9t111_width width)
{
	int32_t rc = 0;
	unsigned char buf[4];

	if (!rdata)
		return -EIO;

	memset(buf, 0, sizeof(buf));

	switch (width) {
	case WORD_LEN: {
#ifdef MT9T111_I2C_2
		buf[0] = (0x098E & 0xFF00)>>8;
		buf[1] = (0x098E & 0x00FF);
		buf[2] = (raddr & 0xFF00)>>8;
		buf[3] = (raddr & 0x00FF);

		rc = mt9t111_i2c_txdata(saddr, buf, 4);

		buf[0] = (0x0990 & 0xFF00)>>8;
		buf[1] = (0x0990 & 0x00FF);
		
		rc = mt9t111_i2c_rxdata(saddr, 2, buf, 2);
		if (rc < 0)
			return rc;

		*rdata = buf[0] << 8 | buf[1];

#else
		buf[0] = (raddr & 0xFF00)>>8;
		buf[1] = (raddr & 0x00FF);

		rc = mt9t111_i2c_rxdata(saddr, 2, buf, 2);
		if (rc < 0)
			return rc;

		*rdata = buf[0] << 8 | buf[1];
#endif
	}
		break;

	case TRIPLE_LEN: { 
		buf[0] = (raddr & 0xFF00)>>8;
		buf[1] = (raddr & 0x00FF);

		rc = mt9t111_i2c_rxdata(saddr, 2, buf, 1);
		if (rc < 0)
			return rc;

		*rdata = buf[0];
	}
		break;

	default:
		break;
	}

	if (rc < 0)
		SKYCERR("mt9t111_i2c_read failed!\n");

	return rc;
}

//#ifdef F_SKYCAM_TUP_LOAD_FILE
static int32_t mt9t111_i2c_read_word(unsigned short raddr, unsigned short *rdata)
{
	int32_t rc = 0;
	unsigned char buf[4];

	if (!rdata)
		return -EIO;

	memset(buf, 0, sizeof(buf));
	
#ifdef MT9T111_I2C_2
	buf[0] = (0x098E & 0xFF00)>>8;
	buf[1] = (0x098E & 0x00FF);
	buf[2] = (raddr & 0xFF00)>>8;
	buf[3] = (raddr & 0x00FF);

	rc = mt9t111_i2c_txdata(mt9t111_client->addr, buf, 4);

	buf[0] = (0x0990 & 0xFF00)>>8;
	buf[1] = (0x0990 & 0x00FF);
	
	rc = mt9t111_i2c_rxdata(mt9t111_client->addr, 2, buf, 2);
	if (rc < 0)
		return rc;

	*rdata = buf[0] << 8 | buf[1];
#else
	buf[0] = (raddr & 0xFF00)>>8;
	buf[1] = (raddr & 0x00FF);

	rc = mt9t111_i2c_rxdata(mt9t111_client->addr, 2, buf, 2);
	if (rc < 0)
		return rc;

	*rdata = buf[0] << 8 | buf[1];
#endif

	if (rc < 0)
		SKYCERR("mt9t111_i2c_read_word failed!\n");

	return rc;
}
//#endif

static int32_t mt9t111_i2c_read_table(
	struct mt9t111_i2c_reg_conf const *reg_conf_tbl,
	int num_of_items_in_table)
{
	int i;
	int32_t rc = -EIO;
	unsigned short read_buf = 0;

	for (i = 0; i < num_of_items_in_table; i++) {
		rc = mt9t111_i2c_read(mt9t111_client->addr,
			reg_conf_tbl->waddr, &read_buf,
			reg_conf_tbl->width);
		if (rc < 0)
			break;

		if(reg_conf_tbl->width == TRIPLE_LEN)
			SKYCDBG("READ!!! ADDR = 0x%x, VALUE = 0x%x, width = %d\n",reg_conf_tbl->waddr, (unsigned char)read_buf, reg_conf_tbl->width);
		else
			SKYCDBG("READ!!! ADDR = 0x%x, VALUE = 0x%x, width = %d\n",reg_conf_tbl->waddr, read_buf, reg_conf_tbl->width);
		reg_conf_tbl++;
	}

	return rc;
}

#ifdef F_SKYCAM_GPIO_SET
static int32_t mt9t111_power(int8_t on)
{
	struct vreg *vreg = NULL;
	uint32_t vnum = 0;
	uint32_t i = 0;
	int32_t rc = 0;

	SKYCDBG("%s start state = %d\n",__func__, on);

	if (on) 
	{
	mt9t111_gpio_set(CAM_LDO_EN, ON);
	mt9t111_gpio_set(CAM_VGA_STANDBY, 0);
	mt9t111_gpio_set(CAM_3M_STANDBY, 0); //active high == standby mode
	}
	else
	{
	mt9t111_gpio_set(CAM_LDO_EN, 0);
	mt9t111_gpio_set(CAM_VGA_STANDBY, 0);
	mt9t111_gpio_set(CAM_3M_STANDBY, 0); //active high but off
	}
	

	vnum = ARRAY_SIZE(mt9t111_vreg);

	for (i = 0; i < vnum; i++) {
		if(on)
			vreg = vreg_get(NULL, mt9t111_vreg[i].name);
		else
			vreg = vreg_get(NULL, mt9t111_vreg[vnum-i-1].name);
		
		if (IS_ERR(vreg)) {
			rc = -ENOENT;
			goto power_fail;
		}

		if (on) {			
			rc = vreg_set_level(vreg, mt9t111_vreg[i].mvolt);
			if (rc)
				goto power_fail;

			rc = vreg_enable(vreg);
			if (rc)
				goto power_fail;
		} else {
			{
				rc = vreg_disable(vreg);
				if (rc)
					goto power_fail;
			}
		}
	}

	SKYCDBG("%s end\n",__func__);
	return 0;

power_fail:
	SKYCDBG("%s failed! (%d)(%d)(%d)\n", __func__, rc, on, i);
	return rc;
}
#else
static int32_t mt9t111_power(int8_t on)
{
	struct vreg *vreg = NULL;
	uint32_t vnum = 0;
	uint32_t i = 0;
	int32_t rc = 0;

	SKYCDBG("%s start state = %d\n",__func__, on);

	if (on) 
	{
#if 0   //CAM_LDO_EN 핀이 0으로 떨어지지 않는현상. 추후확인.
		rc = gpio_tlmm_config(GPIO_CFG(CAM_LDO_EN, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA), GPIO_ENABLE);
		if (rc) SKYCDBG("%s: gpio_tlmm_config_on=%d\n",__func__, rc);
	
		gpio_set_value(CAM_LDO_EN, on);
		SKYCDBG(" gpio_get_value on (%d)=%d\n",CAM_LDO_EN, gpio_get_value(CAM_LDO_EN));	
		
		rc = gpio_tlmm_config(GPIO_CFG(CAM_3M_STANDBY, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), GPIO_ENABLE);
		if (rc) SKYCDBG("%s: gpio_tlmm_config_on=%d\n",__func__, rc);
		gpio_set_value(CAM_3M_STANDBY, 0);
		
		rc = gpio_tlmm_config(GPIO_CFG(CAM_VGA_STANDBY, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), GPIO_ENABLE);
		if (rc) SKYCDBG("%s: gpio_tlmm_config_on=%d\n",__func__, rc);
		gpio_set_value(CAM_VGA_STANDBY, 0);
#else
    	rc = gpio_request(CAM_LDO_EN, "mt9t111");
    	if (!rc) {					
    		gpio_direction_output(CAM_LDO_EN, on);
    		SKYCDBG("LDO ON SUCCESS\n");
    	}
        else
    		SKYCERR("LDO ON FAIL\n");        
//		gpio_free(CAM_LDO_EN);


    	rc = gpio_request(CAM_VGA_STANDBY, "mt9t111");
    	if (!rc) {					
    		gpio_direction_output(CAM_VGA_STANDBY, 0);
    		SKYCDBG("CAM_VGA_STANDBY ON SUCCESS\n");
    	}
        else
       {
    		SKYCERR("CAM_VGA_STANDBY ON FAIL\n");        
		gpio_free(CAM_VGA_STANDBY);
	    	rc = gpio_request(CAM_VGA_STANDBY, "mt9t111");
	    	if (!rc) {					
	    		gpio_direction_output(CAM_VGA_STANDBY, 0);
	    		SKYCDBG("CAM_VGA_STANDBY ON SUCCESS\n");
	    	}
	        else
	    		SKYCERR("CAM_VGA_STANDBY ON FAIL\n");   
	}

    	rc = gpio_request(CAM_3M_STANDBY, "mt9t111");
    	if (!rc) {					
    		gpio_direction_output(CAM_3M_STANDBY, 0);
//    		gpio_direction_output(CAM_3M_STANDBY, on);
    		SKYCDBG("CAM_3M_STANDBY ON SUCCESS\n");
    	}
        else
	{
    		SKYCERR("CAM_3M_STANDBY ON FAIL\n");        
		gpio_free(CAM_3M_STANDBY);
	    	if (!rc) {					
	    		gpio_direction_output(CAM_3M_STANDBY, 0);
//	    		gpio_direction_output(CAM_3M_STANDBY, on);
	    		SKYCDBG("CAM_3M_STANDBY ON SUCCESS\n");
	    	}
	        else
	    		SKYCERR("CAM_3M_STANDBY ON FAIL\n");        
	}

#endif
	}
	else
	{
#if 0   //CAM_LDO_EN 핀이 0으로 떨어지지 않는현상. 추후확인.
		gpio_set_value(CAM_LDO_EN, on);//AF_EN
		SKYCDBG(" gpio_get_value off (%d)=%d\n",CAM_LDO_EN, gpio_get_value(CAM_LDO_EN));	

		rc = gpio_tlmm_config(GPIO_CFG(CAM_LDO_EN, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), GPIO_DISABLE);
		if (rc) SKYCDBG("%s: CAM_LDO_EN gpio_tlmm_config_off=%d\n",__func__, rc);

		gpio_set_value(CAM_VGA_STANDBY, on);
		rc = gpio_tlmm_config(GPIO_CFG(CAM_VGA_STANDBY, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), GPIO_DISABLE);
		if (rc) SKYCDBG("%s: gpio_tlmm_config_on=%d\n",__func__, rc);

		gpio_set_value(CAM_3M_STANDBY, on);
		rc = gpio_tlmm_config(GPIO_CFG(CAM_3M_STANDBY, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), GPIO_DISABLE);
		if (rc) SKYCDBG("%s: gpio_tlmm_config_on=%d\n",__func__, rc);
#else
		gpio_direction_output(CAM_LDO_EN, on);
		SKYCDBG("LDO OFF SUCCESS\n");

		gpio_direction_output(CAM_VGA_STANDBY, on);
		SKYCDBG("CAM_VGA_STANDBY OFF SUCCESS\n");

		gpio_direction_output(CAM_3M_STANDBY, on);
		SKYCDBG("CAM_3M_STANDBY OFF SUCCESS\n");
        
		gpio_free(CAM_LDO_EN);
		gpio_free(CAM_VGA_STANDBY);
		gpio_free(CAM_3M_STANDBY);
#endif		
		
	}
	

	vnum = ARRAY_SIZE(mt9t111_vreg);

	for (i = 0; i < vnum; i++) {
		if(on)
			vreg = vreg_get(NULL, mt9t111_vreg[i].name);
		else
			vreg = vreg_get(NULL, mt9t111_vreg[vnum-i-1].name);
		
		if (IS_ERR(vreg)) {
			rc = -ENOENT;
			goto power_fail;
		}

		if (on) {			
			rc = vreg_set_level(vreg, mt9t111_vreg[i].mvolt);
			if (rc)
				goto power_fail;

			rc = vreg_enable(vreg);
			if (rc)
				goto power_fail;
		} else {
			{
				rc = vreg_disable(vreg);
				if (rc)
					goto power_fail;
			}
		}
	}

	SKYCDBG("%s end\n",__func__);
	return 0;

power_fail:
	SKYCDBG("%s failed! (%d)(%d)(%d)\n", __func__, rc, on, i);
	return rc;
}

#endif

static int mt9t111_reg_init(void)
{
	int32_t array_length;
	int32_t i;
	int32_t rc = 0;
#ifdef F_SKYCAM_TUP_LOAD_FILE
	// 튜닝 파일값
	if((mt9t111_tup_state == MT9T111_TUNE_STATE_LOAD_VALUE) && (mt9t111_params_tbl.init.num_params > 0))	
	{
		mt9t111_i2c_write_params(mt9t111_params_tbl.init.params,
								mt9t111_params_tbl.init.num_params);
	}
	// static 값
	else
#endif
       {
	rc = mt9t111_i2c_write_table(&mt9t111_regs.init_settings[0],
					mt9t111_regs.init_settings_size);
       }
	if (rc < 0)
		return rc;

	return 0;
}

#ifdef F_SKYCAM_FIX_CFG_EFFECT
static int mt9t111_set_effect(int effect)
{
	uint16_t reg_addr;
	uint16_t reg_val;
	int rc = 0;

	SKYCDBG("%s start\n",__func__);

	if(effect < CAMERA_EFFECT_OFF || effect >= CAMERA_EFFECT_MAX){
		SKYCERR("%s error. effect=%d\n", __func__, effect);
		return -EINVAL;
	}
#ifdef F_SKYCAM_TUP_LOAD_FILE
	// 튜닝 파일값
	if((mt9t111_tup_state == MT9T111_TUNE_STATE_LOAD_VALUE) && (mt9t111_params_tbl.effect[effect].num_params > 0))	
	{
		mt9t111_i2c_write_params(mt9t111_params_tbl.effect[effect].params,
								mt9t111_params_tbl.effect[effect].num_params);
	}
	// static 값
	else
#endif	
	{
	rc = mt9t111_i2c_write_table(mt9t111_regs.effect_cfg_settings[effect],
					mt9t111_regs.effect_cfg_settings_size);
	}

	if (rc < 0)
	{
		SKYCERR("CAMERA_WB I2C FAIL!!! return~~\n");
		return rc;
	}		

	mt9t111_effect = effect;
	/* Refresh Sequencer */	

	SKYCDBG("%s end\n",__func__);
	
	return rc;
}
#endif

#ifdef F_SKYCAM_FIX_CFG_WB
static int32_t mt9t111_set_whitebalance (int32_t whitebalance)
{
	
	int32_t rc = 0;
	int8_t m_wb = 0;
		
	SKYCDBG("%s start : whitebalance=%d\n",__func__, whitebalance);
#ifdef F_SKYCAM_TUP_LOAD_FILE
	// 튜닝 파일값
	if((mt9t111_tup_state == MT9T111_TUNE_STATE_LOAD_VALUE) && (mt9t111_params_tbl.wb[whitebalance-1].num_params > 0))	
	{
		mt9t111_i2c_write_params(mt9t111_params_tbl.wb[whitebalance-1].params,
								mt9t111_params_tbl.wb[whitebalance-1].num_params);
	}
	// static 값
	else
#endif		
	{
	rc = mt9t111_i2c_write_table(mt9t111_regs.wb_cfg_settings[whitebalance-1],
					mt9t111_regs.wb_cfg_settings_size);
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


#ifdef F_SKYCAM_FIX_CFG_BRIGHTNESS
static int32_t mt9t111_set_brightness(int32_t brightness)
{
	int32_t rc = 0;
	int i = 0;
	SKYCDBG("%s start~ receive brightness = %d\n",__func__, brightness);

	if ((brightness < 0) || (brightness >= 9)) {
		SKYCERR("%s error. brightness=%d\n", __func__, brightness);
		return -EINVAL;
	}
#ifdef F_SKYCAM_TUP_LOAD_FILE
	// 튜닝 파일값
	if((mt9t111_tup_state == MT9T111_TUNE_STATE_LOAD_VALUE) && (mt9t111_params_tbl.bright[brightness].num_params > 0))	
	{
		SKYCDBG("mt9t111_tup_state = %d, mt9t111_params_tbl.bright[0].num_params=%d\n",
			mt9t111_tup_state, brightness);
		mt9t111_i2c_write_params(mt9t111_params_tbl.bright[brightness].params,
								mt9t111_params_tbl.bright[brightness].num_params);
	}
	// static 값
	else
#endif
	{
	rc = mt9t111_i2c_write_table(mt9t111_regs.bright_cfg_settings[brightness],	
					mt9t111_regs.bright_cfg_settings_size);
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

#ifdef F_SKYCAM_FIX_CFG_LED_MODE
static int32_t mt9t111_set_led_mode(int32_t led_mode)
{
	/* off, auto, on */	
	unsigned short flash_current = NULL;
	int rc = 0;
	
	msm_pm_set_flash_current_req req;
	msm_pm_set_flash_current_rep rep;

	SKYCDBG("%s: led_mode=%d\n", __func__, led_mode);
	if ((led_mode < 0) || (led_mode >= 3)) {
		SKYCERR("%s: -EINVAL, led_mode=%d\n", __func__, led_mode);
		return -EINVAL;
	}
			
	switch(led_mode)
	{
		case 0:
			flash_current = 0;			
			break;
		case 1:
			flash_current = 0;			
			SKYCDBG("PM_FLASH_LED_SET_CURRENT_PROC rc = %d\n", rc);
			break;
		case 2:
			flash_current = 100;	
			break;
		default:
			flash_current = 0;			
			break;			
	}
	
	req.val = cpu_to_be32(flash_current);
	rc = msm_rpc_call_reply(pm_lib_ep,
			PM_FLASH_LED_SET_CURRENT_PROC,
			&req, sizeof(req), 
			&rep, sizeof(rep), 
			1000);		
		
	return 0;
}
#endif

#ifdef F_SKYCAM_ADD_CFG_READ_REG
static unsigned short mt9t111_read_reg(unsigned short raddr)
{
	int32_t rc = 0;
	unsigned short read_buf = 0xcccc;
	
      rc = mt9t111_i2c_write(mt9t111_client->addr,
	  	0x098E, raddr, WORD_LEN);

      if (rc < 0)
      {
		SKYCDBG("CAMERA_AUTO_FOCUS I2C FAIL_2!!! return~~\n");
		read_buf = 0xcccc;
		return read_buf;
      } 
		
	rc = mt9t111_i2c_read(mt9t111_client->addr,
			0x0990, &read_buf,
			WORD_LEN);
	if (rc < 0)
	{
		SKYCDBG("AUTO FOCUS READ I2C FAIL!!! return~~\n");
		read_buf = 0xcccc;
		return read_buf;
	}

	SKYCDBG("%s end : 0x%x register read data = %d\n", __func__, raddr, read_buf);

	return read_buf;
}
#endif

#ifdef F_SKYCAM_FIX_CFG_AF
static int32_t mt9t111_set_auto_focus(void)
{
#define AF_POLL_PERIOD	50
#define AF_POLL_RETRY	50

	int32_t rc = 0;
	int32_t i = 0;
	unsigned short read_buf = 0;
#ifdef F_SKYCAM_FIX_CFG_LED_MODE	
	int8_t nbrightness = 0;
#endif	

#ifdef F_SKYCAM_FIX_CFG_LED_MODE			
	if(led_auto == 1)
	{			
		rc = mt9t111_i2c_write(mt9t111_client->addr,
		0x098E, 0xB804,WORD_LEN);
		rc = mt9t111_i2c_read(mt9t111_client->addr,
		0xB804, &nbrightness,TRIPLE_LEN);
		SKYCDBG("%s SENSOR_SNAPSHOT_MODE nbrightness =0x%x\n",__func__, nbrightness);
		if(nbrightness <= 0x38)
			mt9t111_set_led_mode(2);
	}
#endif	
	
//	SKYCDBG("%s start\n",__func__);

	rc = mt9t111_i2c_write_table(&mt9t111_regs.autofocus_trigger_cfg_settings[0],
					mt9t111_regs.autofocus_trigger_cfg_settings_size);	
	if (rc < 0)
	{
		SKYCDBG("CAMERA_AUTO_FOCUS I2C FAIL_1!!! return~~\n");
		return rc;
	}		

#if 1 //test 비디오 포커스 테스트용 
	for (i = 0; i < AF_POLL_RETRY; i++) 
	{
#if 0 //def F_SKYCAM_ADD_CFG_READ_REG
		read_buf = mt9t111_read_reg(0xB019);
#else
//AF값을 읽을 때마다 읽은 번지를 써줘야해서 위치 옴김     
      rc = mt9t111_i2c_write(mt9t111_client->addr,
      0x098E, 0xB019,
      WORD_LEN);

      if (rc < 0)
      {
        SKYCDBG("CAMERA_AUTO_FOCUS I2C FAIL_2!!! return~~\n");
        return rc;
      } 
		
		rc = mt9t111_i2c_read(mt9t111_client->addr,
				0x0990, &read_buf,
				WORD_LEN);
		if (rc < 0)
		{
			SKYCDBG("AUTO FOCUS READ I2C FAIL!!! return~~\n");
#ifdef F_SKYCAM_FIX_CFG_LED_MODE
			if(led_auto == 1 && nbrightness <= 0x38)
				mt9t111_set_led_mode(0);
#endif
			return rc;
		}

		SKYCDBG("AF polling num = %d,  0xB019 = %d\n", i, read_buf);
#endif
		if(read_buf == 0)
			break;
      mdelay(AF_POLL_PERIOD);
	}

	/* add delay here to view several frames after AF is done 
	  * to display frame after focus done. */
	mdelay(AF_POLL_PERIOD*2);

	SKYCDBG("%s end\n",__func__);
#ifdef F_SKYCAM_FIX_CFG_LED_MODE
	if(led_auto == 1 && nbrightness <= 0x38)
		mt9t111_set_led_mode(0);
#endif	

#endif

	return rc;
}
#endif

#ifdef F_SKYCAM_FIX_CFG_ISO
static int32_t mt9t111_set_iso(int32_t iso)
{
	/* auto, N/A, ISO100, ISO200, ISO400, ISO800, ISO1600, ISO3200 */
	uint8_t data[] = {0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
	int32_t rc = 0;

#if 0	
//	if(mode == SENSOR_SNAPSHOT_MODE) return 0;

	if ((iso < 0) || (iso >= sizeof(data)) || (iso == 1))
		return -EINVAL;

	SKYCDBG("%s  iso = %d\n",__func__, iso);

	rc = mt9t111_i2c_write(mt9t111_client->addr, 0x17, data[iso],WORD_LEN);
#endif
	
	return rc;
}
#endif

#ifdef F_SKYCAM_FIX_CFG_SCENE_MODE
uint8_t pre_scene_mode = MT9T111_CFG_SCENE_MODE_OFF;

static int32_t mt9t111_set_scene_mode(int32_t scene_mode)
{
	/* +off, +auto, +portrait, +landscape, indoor, +night, +sports, dawn,
	 * snow, autumn, text, +sunset, backlight, beach, party */
	//uint8_t data[] = {0x00, 0x03, 0x05, 0x09, 0x0b, 0x0d, 0x0f, 0x11, 
	//	0x13, 0x15, 0x1b, 0x1d, 0x1f, 0x21, 0x23};

	int32_t rc = 0;

	if ((scene_mode < 0) || (scene_mode > MT9T111_CFG_SCENE_MODE_MAX))
	{
		SKYCERR("%s not support scene mode  = %d\n",__func__,scene_mode);
		return 0;//-EINVAL;
	}

#ifdef F_SKYCAM_TUP_LOAD_FILE	
	if((mt9t111_tup_state == MT9T111_TUNE_STATE_LOAD_VALUE) && (mt9t111_params_tbl.scene[scene_mode].num_params > 0))	
	{
		mt9t111_i2c_write_params(mt9t111_params_tbl.scene[scene_mode].params,
								mt9t111_params_tbl.scene[scene_mode].num_params);
	}
	// static 값
	else
#endif
SKYCERR("%s scene mode  = %d, pre[%d]\n",__func__,scene_mode,pre_scene_mode);
#if 1 // 설정 시에는 항상 off해야 하므로 먼저 off를 실행하고, 첫 실행시는 off이므로 상관없다.
     if((scene_mode != pre_scene_mode)||(scene_mode==MT9T111_CFG_SCENE_MODE_OFF))
	{
	switch(pre_scene_mode)
	{ //off
		case MT9T111_CFG_SCENE_MODE_PORTRAIT:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_portrait_mode_cfg_settings[0],	
						mt9t111_regs.scene_portrait_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_LANDSCAPE:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_landscape_mode_cfg_settings[0],	
						mt9t111_regs.scene_landscape_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_INDOOR:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_indoor_mode_cfg_settings[0],	
						mt9t111_regs.scene_indoor_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_SPORTS:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_sports_mode_cfg_settings[0],	
						mt9t111_regs.scene_sports_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_NIGHT:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_night_mode_cfg_settings[0],	
						mt9t111_regs.scene_night_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_BEACH:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_beach_mode_cfg_settings[0],	
						mt9t111_regs.scene_beach_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_WINTER:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_snow_mode_cfg_settings[0],	
						mt9t111_regs.scene_snow_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_SUNSET:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_sunset_mode_cfg_settings[0],	
						mt9t111_regs.scene_sunset_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_TEXT:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_text_mode_cfg_settings[0],	
						mt9t111_regs.scene_text_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_PARTY:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_fireworks_mode_cfg_settings[0],	
						mt9t111_regs.scene_fireworks_mode_cfg_settings_size);
			break;
		}
	  }
		 msleep(1);
     	}
// 앞에서 off를 먼저 하고 들어오도록 한다. 
	{
	switch(scene_mode)
	{ //on
		case MT9T111_CFG_SCENE_MODE_PORTRAIT:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_portrait_mode_cfg_settings[1],	
						mt9t111_regs.scene_portrait_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_LANDSCAPE:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_landscape_mode_cfg_settings[1],	
						mt9t111_regs.scene_landscape_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_INDOOR:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_indoor_mode_cfg_settings[1],	
						mt9t111_regs.scene_indoor_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_SPORTS:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_sports_mode_cfg_settings[1],	
						mt9t111_regs.scene_sports_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_NIGHT:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_night_mode_cfg_settings[1],	
						mt9t111_regs.scene_night_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_BEACH:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_beach_mode_cfg_settings[1],	
						mt9t111_regs.scene_beach_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_WINTER:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_snow_mode_cfg_settings[1],	
						mt9t111_regs.scene_snow_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_SUNSET:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_sunset_mode_cfg_settings[1],	
						mt9t111_regs.scene_sunset_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_TEXT:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_text_mode_cfg_settings[1],	
						mt9t111_regs.scene_text_mode_cfg_settings_size);
			break;
		}
		case MT9T111_CFG_SCENE_MODE_PARTY:
		{
		rc = mt9t111_i2c_write_table(mt9t111_regs.scene_fireworks_mode_cfg_settings[1],	
						mt9t111_regs.scene_fireworks_mode_cfg_settings_size);
			break;
		}
	  }
	msleep(1);
	pre_scene_mode =scene_mode; //현재값 저장
     	}
#else
	{
	rc = mt9t111_i2c_write_table(mt9t111_regs.scene_mode_cfg_settings[scene_mode],	
					mt9t111_regs.scene_mode_cfg_settings_size);
	}
#endif
	if (rc < 0)
	{
		SKYCERR("%s FAIL!!! return~~\n",__func__);
		return rc;
	}		

	SKYCDBG("%s (scene_mode=%d)end\n",__func__, scene_mode);

	return rc;
}
#endif

#ifdef F_SKYCAM_FIX_CFG_FOCUS_STEP
static int32_t mt9t111_set_focus_step(int32_t focus_step)
{
	int32_t rc = 0;

	if ((focus_step < 0) || (focus_step >= MT9T111_CFG_FOCUS_STEP_MAX))
	{
		SKYCERR("%s FAIL!!! return~~  focus_step = %d\n",__func__,focus_step);
		return 0;//-EINVAL;
	}
#ifdef F_SKYCAM_TUP_LOAD_FILE
	if((mt9t111_tup_state == MT9T111_TUNE_STATE_LOAD_VALUE) && (mt9t111_params_tbl.focus_step[0].num_params > 0))	
	{
		mt9t111_i2c_write_params(mt9t111_params_tbl.focus_step[focus_step].params,
								mt9t111_params_tbl.focus_step[focus_step].num_params);
	}
	// static 값
	else
#endif	
	{
	rc = mt9t111_i2c_write_table(mt9t111_regs.focus_step_cfg_settings[focus_step],	
					mt9t111_regs.focus_step_cfg_settings_size);
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

#ifdef F_SKYCAM_ADD_CFG_ANTISHAKE
static int32_t mt9t111_set_antishake(int32_t antishake)
{
	int32_t rc = 0;

	if ((antishake < 0) || (antishake >= MT9T111_CFG_ANTISHAKE_MAX))
	{
		SKYCERR("%s FAIL!!! return~~  antishake = %d\n",__func__,antishake);
		return 0;//-EINVAL;
	}
#ifdef F_SKYCAM_TUP_LOAD_FILE
	if((mt9t111_tup_state == MT9T111_TUNE_STATE_LOAD_VALUE) && (mt9t111_params_tbl.antishake[antishake].num_params > 0))	
	{
		mt9t111_i2c_write_params(mt9t111_params_tbl.antishake[antishake].params,
								mt9t111_params_tbl.antishake[antishake].num_params);
	}
	// static 값
	else
#endif	
	{
		rc = mt9t111_i2c_write_table(mt9t111_regs.antishake_cfg_settings[antishake],	
					mt9t111_regs.antishake_cfg_settings_size);
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

#ifdef F_SKYCAM_FIX_CFG_EXPOSURE
static int32_t mt9t111_set_exposure_mode(int32_t exposure)
{
	int32_t rc = 0;

	SKYCDBG("%s  exposure = %d\n",__func__, exposure);

	if ((exposure < 0) || (exposure >= MT9T111_CFG_EXPOSURE_MAX))
	{
		SKYCERR("%s FAIL!!! return~~  exposure = %d\n",__func__,exposure);
		return 0;//-EINVAL;
	}
#ifdef F_SKYCAM_TUP_LOAD_FILE
	if((mt9t111_tup_state == MT9T111_TUNE_STATE_LOAD_VALUE) && (mt9t111_params_tbl.exposure[exposure].num_params > 0))	
	{
		mt9t111_i2c_write_params(mt9t111_params_tbl.exposure[exposure].params,
								mt9t111_params_tbl.exposure[exposure].num_params);
	}
	// static 값
	else
#endif		
	{
	rc = mt9t111_i2c_write_table(mt9t111_regs.exposure_cfg_settings[exposure],
					mt9t111_regs.exposure_cfg_settings_size);
	}

	if (rc < 0)
	{
		SKYCERR("CAMERA_EFFECT_SEPIA I2C FAIL!!! return~~\n");
		return rc;
	}		
	
	SKYCDBG("%s end\n",__func__);

	return rc;
}
#endif

#ifdef F_SKYCAM_FIX_CFG_REFLECT
static int32_t mt9t111_set_reflect(int32_t reflect)
{
	int32_t rc = 0;
	int32_t i = 0;
	int8_t npolling = -1;

	SKYCDBG("%s  reflect = %d\n",__func__, reflect);

	if ((reflect < 0) || (reflect >= MT9T111_CFG_REFLECT_MAX))
	{
		SKYCERR("%s FAIL!!! return~~  reflect = %d\n",__func__,reflect);
		return 0;//-EINVAL;
	}
#ifdef F_SKYCAM_TUP_LOAD_FILE
	if((mt9t111_tup_state == MT9T111_TUNE_STATE_LOAD_VALUE) && (mt9t111_params_tbl.reflect[reflect].num_params > 0))	
	{
		mt9t111_i2c_write_params(mt9t111_params_tbl.reflect[reflect].params,
								mt9t111_params_tbl.reflect[reflect].num_params);
	}
	// static 값
	else
#endif		
	{
		rc = mt9t111_i2c_write_table(mt9t111_regs.reflect_cfg_settings[reflect],
					mt9t111_regs.reflect_cfg_settings_size);
	}

	for (i = 0; i < MT9T111_POLLING_RETRY; i++) {
		rc = mt9t111_i2c_read(mt9t111_client->addr,
		0x8404, &npolling,TRIPLE_LEN);
		if (rc < 0)
		{
			SKYCERR("mt9t111_snapshot_config read  FAIL!!! return~~\n");
			//return rc;
		}
		SKYCDBG("%s: retry npolling = %x, count = %d\n", __func__,npolling, i);
		if (npolling == 0) {				
			return 0;
		}		
		msleep(20);
	}	
 
	if (rc < 0)
	{
		SKYCERR("CAMERA_SET_REFLECT I2C FAIL!!! return~~\n");
		return rc;
	}		
	
	SKYCDBG("%s end\n",__func__);

	return rc;
}
#endif

#ifdef F_SKYCAM_ADD_CFG_CAF
static int mt9t111_set_continuous_af(int32_t caf)
{
#if 1
	return 0;
#else
	int32_t rc = 0;

	SKYCDBG("%s  continuous_af = %d\n",__func__, caf);

	if(caf == 1)
	{
		rc = mt9t111_i2c_write_table(mt9t111_regs.caf_cfg_settings,
					mt9t111_regs.caf_cfg_setting_size);	
	}
	else
	{
		rc = mt9t111_i2c_write_table(mt9t111_regs.fastmode_cfg_settings,
					mt9t111_regs.fastmode_cfg_setting_size);
	}

	SKYCDBG("%s end\n", __func__);
	return rc;
#endif
}
#endif

#ifdef F_SKYCAM_CFG_AF_SPOT
static int32_t mt9t111_focus_set_center(void)
{
    int32_t rc = 0;
	//normal - center focus fix
	/// [AF Center Fix]
	rc =	mt9t111_i2c_write_word(0x098E, 0x3005); // MCU_ADDRESS [AF window X start]
	rc =	mt9t111_i2c_write_word(0x0990, 0x136);
	if (rc < 0) {
	SKYCERR("CAMERA_AUTO_FOCUS I2C FAIL_1!!! return~~\n");
	return rc;
	}
	rc =	mt9t111_i2c_write_word(0x098E, 0x3007); // MCU_ADDRESS [AF window Y start]
	rc =	mt9t111_i2c_write_word(0x0990, 0x00E3);
	if (rc < 0) {
	SKYCERR("CAMERA_AUTO_FOCUS I2C FAIL_1!!! return~~\n");
	return rc;
	}
	rc =	mt9t111_i2c_write_word(0x098E, 0x3009); // MCU_ADDRESS [AF window Width]
	rc =	mt9t111_i2c_write_word(0x0990, 0x0068);
	if (rc < 0) {
	SKYCERR("CAMERA_AUTO_FOCUS I2C FAIL_1!!! return~~\n");
	return rc;
	}
	rc =	mt9t111_i2c_write_word(0x098E, 0x300B); // MCU_ADDRESS [AF window Height]
	rc =	mt9t111_i2c_write_word(0x0990, 0x004E);
	if (rc < 0) {
	SKYCERR("CAMERA_AUTO_FOCUS I2C FAIL_1!!! return~~\n");
	return rc;
	}
	rc =	mt9t111_i2c_write_word(0x098E, 0x3020); // MCU_ADDRESS [AF weight High]
	rc =	mt9t111_i2c_write_word(0x0990, 0x557D);
	if (rc < 0) {
	SKYCERR("CAMERA_AUTO_FOCUS I2C FAIL_1!!! return~~\n");
	return rc;
	}
	rc =	mt9t111_i2c_write_word(0x098E, 0x3022); // MCU_ADDRESS [AF weight Low]
	rc =	mt9t111_i2c_write_word(0x0990, 0x7D55);
	if (rc < 0) {
	SKYCERR("CAMERA_AUTO_FOCUS I2C FAIL_1!!! return~~\n");
	return rc;
	}
	//focus모드 변경시에는 항상 아래 refresh 코드 타도록 해야 변경 사항이 정상 적용된다. 20110408
	rc =	mt9t111_i2c_write_word(0x098E, 0x8400); // MCU_ADDRESS(Refresh)
	rc =	mt9t111_i2c_write_word(0x0990, 0x0005); //06 // MCU_DATA_0
	if (rc < 0) {
		SKYCERR("CAMERA_AUTO_FOCUS I2C FAIL_1!!! return~~\n");
		return rc;
	}
	SKYCDBG("mt9t111_focus_set_center() FOCUS CENTER \n");
	return 0;
}
#endif

#ifdef F_SKYCAM_FIX_CFG_FOCUS_RECT
static int32_t mt9t111_set_focus_rect(uint32_t focus_rect) //, int focus_mode
{
	uint16_t xleft, xright, ytop, ybottom, xc, yc;
	uint16_t nwidth, nheight;
	int32_t rc = 0;

	SKYCDBG("%s  focus_rect = %d\n",__func__, focus_rect);
#ifdef F_SKYCAM_CFG_AF_SPOT
	xleft = focus_rect>>16;
	ytop = focus_rect&0xffff;

	if((xleft ==-1)&&(ytop == -1))
	{
		rc = mt9t111_focus_set_center();
		SKYCDBG("mt9t111_set_focus_rect() FOCUS CENTER \n");
		return rc;
	}

	//현재 좌표는 480x320 이므로 프리뷰 사이즈인 640x480으로 변환한다. 
//하드코딩이므로 차후 변경 필요 
//아래 두줄 비정상 동작함
//	xleft = (int16_t)(xc*(1.33)); 
//	ytop = (int16_t)(yc*(1.50));

	nwidth = xleft + 80;
	nheight = ytop + 80;
	if(nwidth > 640) nwidth = 640;
	if(nheight > 480) nheight = 480;
	SKYCDBG("%s  xleft = %d, ytop = %d, nwidth = %d, nheight = %d\n",__func__, xleft, ytop, nwidth, nheight);

	// Spot focus
	/// [AF Window & Size for SF feature ON]
	rc =	mt9t111_i2c_write_word(0x098E, 0x3005); // MCU_ADDRESS [AF window X start]
	rc =	mt9t111_i2c_write_word(0x0990, xleft);
	if (rc < 0) {
	SKYCERR("CAMERA_AUTO_FOCUS I2C FAIL_1!!! return~~\n");
	return rc;
	}
	rc =	mt9t111_i2c_write_word(0x098E, 0x3007); // MCU_ADDRESS [AF window Y start]
	rc =	mt9t111_i2c_write_word(0x0990, ytop);
	if (rc < 0) {
	SKYCERR("CAMERA_AUTO_FOCUS I2C FAIL_1!!! return~~\n");
	return rc;
	}
	rc =	mt9t111_i2c_write_word(0x098E, 0x3009); // MCU_ADDRESS [AF window Width]
	rc =	mt9t111_i2c_write_word(0x0990, nwidth);
	if (rc < 0) {
	SKYCERR("CAMERA_AUTO_FOCUS I2C FAIL_1!!! return~~\n");
	return rc;
	}
	rc =	mt9t111_i2c_write_word(0x098E, 0x300B); // MCU_ADDRESS [AF window Height]
	rc =	mt9t111_i2c_write_word(0x0990, nheight);
	if (rc < 0) {
	SKYCERR("CAMERA_AUTO_FOCUS I2C FAIL_1!!! return~~\n");
	return rc;
	}
	rc =	mt9t111_i2c_write_word(0x098E, 0x3020); // MCU_ADDRESS [AF weight High]
	rc =	mt9t111_i2c_write_word(0x0990, 0xffff);
	if (rc < 0) {
	SKYCERR("CAMERA_AUTO_FOCUS I2C FAIL_1!!! return~~\n");
	return rc;
	}
	rc =	mt9t111_i2c_write_word(0x098E, 0x3022); // MCU_ADDRESS [AF weight Low]
	rc =	mt9t111_i2c_write_word(0x0990, 0xffff);
	if (rc < 0) {
	SKYCERR("CAMERA_AUTO_FOCUS I2C FAIL_1!!! return~~\n");
	return rc;
	}
	//focus모드 변경시에는 항상 아래 refresh 코드 타도록 해야 변경 사항이 정상 적용된다. 20110408
	rc =	mt9t111_i2c_write_word(0x098E, 0x8400); // MCU_ADDRESS(Refresh)
	rc =	mt9t111_i2c_write_word(0x0990, 0x0005); //06 // MCU_DATA_0
	if (rc < 0) {
		SKYCERR("CAMERA_AUTO_FOCUS I2C FAIL_1!!! return~~\n");
		return rc;
	}
#endif
	
    return rc;
}
#endif

#ifdef F_SKYCAM_ADD_CFG_DIMENSION
static int32_t mt9t111_set_dimension(struct dimension_cfg *dimension)
{
	memcpy(&mt9t111_dimension, dimension, sizeof(struct dimension_cfg));
	SKYCDBG("%s: preview=%dx%d, snapshot=%dx%d\n", __func__,
		dimension->prev_dx, dimension->prev_dy,
		dimension->pict_dx, dimension->pict_dy);
	return 0;
}
#endif

#ifdef F_SKYCAM_FIX_CFG_ANTIBANDING
static int32_t mt9t111_set_antibanding(int32_t antibanding)
{
	int32_t rc = 0;

	if ((antibanding < 0) || (antibanding >= MT9T111_CFG_FLICKER_MAX))
	{
		SKYCERR("%s FAIL!!! return~~  antibanding = %d\n",__func__,antibanding);
		return 0;//-EINVAL;
	}
#ifdef F_SKYCAM_TUP_LOAD_FILE
	if((mt9t111_tup_state == MT9T111_TUNE_STATE_LOAD_VALUE) && (mt9t111_params_tbl.flicker[antibanding].num_params > 0))	
	{
		mt9t111_i2c_write_params(mt9t111_params_tbl.flicker[antibanding].params,
								mt9t111_params_tbl.flicker[antibanding].num_params);
	}
	// static 값
	else
#endif		
	{
	rc = mt9t111_i2c_write_table(mt9t111_regs.flicker_cfg_settings[antibanding],	
					mt9t111_regs.flicker_cfg_settings_size);
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


#ifdef F_SKYCAM_FIX_CFG_PREVIEW_FPS
static int32_t mt9t111_set_preview_fps(int32_t preview_fps)
{
	/* 0 : variable 30fps, 1 ~ 30 : fixed fps */
	/* default: variable 8 ~ 30fps */
	uint8_t data = 0;
	int32_t rc = 0;

	if ((preview_fps < C_SKYCAM_MIN_PREVIEW_FPS) || 
		(preview_fps > C_SKYCAM_MAX_PREVIEW_FPS)) {
		SKYCERR("%s: -EINVAL, preview_fps=%d\n", 
			__func__, preview_fps);
		return -EINVAL;
	}

	SKYCDBG("%s: preview_fps=%d\n", __func__, preview_fps);
#ifdef F_SKYCAM_TUP_LOAD_FILE
	if((mt9t111_tup_state == MT9T111_TUNE_STATE_LOAD_VALUE) && (mt9t111_params_tbl.ffps[preview_fps-5].num_params > 0))	
	{
		mt9t111_i2c_write_params(mt9t111_params_tbl.ffps[preview_fps-5].params,
								mt9t111_params_tbl.ffps[preview_fps-5].num_params);
	}
	// static 값
	else
#endif
	{	
	rc = mt9t111_i2c_write_table(&mt9t111_regs.preview_fps_cfg_settings[preview_fps-5],
					mt9t111_regs.preview_fps_cfg_settings_size);
	}

	SKYCDBG("%s end rc = %d\n",__func__, rc);

	return 0;
}
#endif
#ifdef F_SKYCAM_TUP_LOAD_FILE
static u_int32_t mt9t111_i2c_write_params
(
    mt9t111_cfg_param_t *params,
    u_int32_t num_params
)
{
    u_int16_t read_data = 0;
    u_int16_t write_data = 0;
    u_int8_t write_data_byte = 0;	
    u_int32_t i = 0;

    SKYCDBG(">>%s ", __func__);
    SKYCDBG("- params = 0x%08x, num_params = %d\n", params, num_params, 0);

    if ((params == NULL) || (num_params == 0))
    {
        SKYCERR("Invalid param! [params = 0x%08x][num_params = %d]\n", (u_int32_t)params, num_params, 0);
        return -EIO;    
    }

    for (i = 0; i < num_params; i++)
    {
        switch(params[i].op)
        {
            case CAMOP_NOP:
                SKYCDBG("WRITE: NOP\n", 0, 0, 0);
                break;

            case CAMOP_DELAY:
                SKYCDBG("WRITE: DELAY (%dms)\n", params[i].data, 0, 0);
                mdelay(params[i].data);
                break;

            case CAMOP_WRITE:
                if (mt9t111_i2c_write_word(params[i].addr, params[i].data) < 0)
                {
                    SKYCERR("<<%s (-EIO)\n", __func__, 0, 0);
                    return -EIO;
                }
                break;
	     case CAMOP_WRITE_DATA1:
		 //배열의 2바이트 데이터를 1바이트로 캐스팅
 	 	 write_data_byte = (unsigned char)params[i].data;
		 if (mt9t111_i2c_write_a2d1(params[i].addr, write_data_byte) < 0)
                {
                    SKYCERR("<<%s (-EIO)\n", __func__, 0, 0);
                    return -EIO;
                }
                break;
            case CAMOP_WRITE_BIT:
            {
                u_int16_t bit_pos = 0;
                u_int16_t bit_val = 0;

                if (mt9t111_i2c_read_word(params[i].addr, &read_data) < 0)
                {
                    SKYCERR("<<%s (-EIO)", __func__, 0, 0);
                    return -EIO;
                }

                bit_pos = (params[i].data & 0xff00) >> 8;
                bit_val = (params[i].data & 0x00ff);

                if (bit_val != 0) // set to 1
                {
                    write_data = read_data | (0x0001 << bit_pos);
                    SKYCDBG("set 0x%04x[%d] to 1\n", params[i].addr, bit_pos, 0);
                }
                else // set to 0
                {
                    write_data = read_data & (~(0x0001 << bit_pos));
                    SKYCDBG("set 0x%04x[%d] to 0\n", params[i].addr, bit_pos, 0);
                }

                if (mt9t111_i2c_write_word(params[i].addr, write_data) < 0)
                {
                    SKYCERR("<<%s (-EIO)\n", __func__, 0, 0);
                    return -EIO;
                }
                break;
            }

            case CAMOP_POLL_REG:
            {
                u_int16_t poll_delay = 0;
                u_int16_t poll_retry = 0;
                u_int16_t poll_reg = 0;
                u_int16_t poll_data = 0;
                u_int16_t poll_mask = 0;
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
                    if (mt9t111_i2c_read_word(poll_reg, &read_data) < 0)
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
                    SKYCERR("<<%s (-EIO)", __func__, 0, 0);
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
                    if (mt9t111_i2c_write_word(MT9T111_REG_MCU_ADDR, poll_mcu_var) < 0)
                    {
                        SKYCERR("<<%s (-EIO)", __func__, 0, 0);
                        return -EIO;
                    }

                    if (mt9t111_i2c_read_word(MT9T111_REG_MCU_DATA, &read_data) < 0)
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
                SKYCERR("Invalid op code! [op = %d]", params[i].op, 0, 0);
                return -EIO;
                break;
        }

    }

    SKYCDBG("<<%s (TRUE)", __func__, 0, 0);
    return TRUE;
}

static void mt9t111_init_params_tbl (int8_t *stream)
{
#if defined(F_SKYCAM_SENSOR_TUNEUP)
    mt9t111_cfg_param_t *params = NULL;
    u_int32_t num_params = 0;
#endif /* F_SKYCAM_SENSOR_TUNEUP */
    u_int32_t i = 0;

    SKYCDBG(">>%s ()", __func__, 0, 0);

#if defined(F_SKYCAM_SENSOR_TUNEUP)
    camsensor_mt9t111_tup_init(stream);
    num_params = camsensor_mt9t111_tup_get_init_params(&params);
    if ((num_params > 0) && (params != NULL))
    {
        mt9t111_params_tbl.init.num_params = num_params;
        mt9t111_params_tbl.init.params = params;
        //SKYCDBG("MT9T111 INIT params are loaded from TUNEUP file!\n", 0, 0, 0);
    }
    else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
    {
        mt9t111_params_tbl.init.num_params = num_params;
        mt9t111_params_tbl.init.params = params;
        //SKYCDBG("MT9T111 INIT params are loaded from TUNEUP file!\n", 0, 0, 0);
#if 0		
        mt9t111_params_tbl.init.num_params = sizeof(mt9t111_cfg_init_params) / sizeof(mt9t111_cfg_param_t);
        mt9t111_params_tbl.init.params = &mt9t111_cfg_init_params[0];
        SKYCDBG("MT9T111 INIT params are loaded from RO data!", 0, 0, 0);
#endif		
    }

    for (i = 0; i < MT9T111_CFG_WB_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_mt9t111_tup_get_wb_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            mt9t111_params_tbl.wb[i].num_params = num_params;
            mt9t111_params_tbl.wb[i].params = params;
            //SKYCDBG("MT9T111 WB params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {        
            mt9t111_params_tbl.wb[i].num_params = num_params;
            mt9t111_params_tbl.wb[i].params = params;
            //SKYCDBG("MT9T111 WB params [%d] are loaded from TUNEUP file!\n", i, 0, 0);        
#if 0        
            mt9t111_params_tbl.wb[i].num_params = MT9T111_CFG_WB_MAX_PARAMS;
            mt9t111_params_tbl.wb[i].params = &mt9t111_cfg_wb_params[i][0];
            SKYCDBG("MT9T111 WB params [%d] are loaded from RO data!", i, 0, 0);
#endif			
        }
    }

    for (i = 0; i < MT9T111_CFG_BRIGHT_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_mt9t111_tup_get_bright_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            mt9t111_params_tbl.bright[i].num_params = num_params;
            mt9t111_params_tbl.bright[i].params = params;
            //SKYCDBG("MT9T111 BRIGHT params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {
      	     mt9t111_params_tbl.bright[i].num_params = num_params;
            mt9t111_params_tbl.bright[i].params = params;
            //SKYCDBG("MT9T111 BRIGHT params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
#if 0        
            mt9t111_params_tbl.bright[i].num_params = MT9T111_CFG_BRIGHT_MAX_PARAMS;
            mt9t111_params_tbl.bright[i].params = &mt9t111_cfg_bright_params[i][0];
            SKYCDBG("MT9T111 BRIGHT params [%d] are loaded from RO data!", i, 0, 0);
#endif			
        }

    }

    for (i = 0; i < MT9T111_CFG_EXPOSURE_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_mt9t111_tup_get_exposure_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            mt9t111_params_tbl.exposure[i].num_params = num_params;
            mt9t111_params_tbl.exposure[i].params = params;
            //SKYCDBG("MT9T111 EXPOSURE params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {
            mt9t111_params_tbl.exposure[i].num_params = num_params;
            mt9t111_params_tbl.exposure[i].params = params;
            //SKYCDBG("MT9T111 EXPOSURE params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
#if 0        
            mt9t111_params_tbl.exposure[i].num_params = MT9T111_CFG_EXPOSURE_MAX_PARAMS;
            mt9t111_params_tbl.exposure[i].params = &mt9t111_cfg_exposure_params[i][0];
            SKYCDBG("MT9T111 EXPOSURE params [%d] are loaded from RO data!", i, 0, 0);
#endif			
        }

    }

    for (i = MT9T111_CFG_FFPS_MIN; i <= MT9T111_CFG_FFPS_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_mt9t111_tup_get_ffps_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            mt9t111_params_tbl.ffps[i - MT9T111_CFG_FFPS_MIN].num_params = num_params;
            mt9t111_params_tbl.ffps[i - MT9T111_CFG_FFPS_MIN].params = params;
            //SKYCDBG("MT9T111 fixed FPS params [%d] are loaded from TUNEUP file!\n", i - MT9T111_CFG_FFPS_MIN, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {
            mt9t111_params_tbl.ffps[i - MT9T111_CFG_FFPS_MIN].num_params = num_params;
            mt9t111_params_tbl.ffps[i - MT9T111_CFG_FFPS_MIN].params = params;
            //SKYCDBG("MT9T111 fixed FPS params [%d] are loaded from TUNEUP file!\n", i - MT9T111_CFG_FFPS_MIN, 0, 0);
#if 0        
            mt9t111_params_tbl.ffps[i - MT9T111_CFG_FFPS_MIN].num_params = MT9T111_CFG_FFPS_MAX_PARAMS;
            mt9t111_params_tbl.ffps[i - MT9T111_CFG_FFPS_MIN].params = &mt9t111_cfg_ffps_params[i - MT9T111_CFG_FFPS_MIN][0];
            SKYCDBG("MT9T111 fixed FPS params [%d] are loaded from RO data!", i - MT9T111_CFG_FFPS_MIN, 0, 0);
#endif			
        }
    }

    for (i = 0; i < MT9T111_CFG_REFLECT_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_mt9t111_tup_get_reflect_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            mt9t111_params_tbl.reflect[i].num_params = num_params;
            mt9t111_params_tbl.reflect[i].params = params;
            //SKYCDBG("MT9T111 REFLECT params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {
            mt9t111_params_tbl.reflect[i].num_params = num_params;
            mt9t111_params_tbl.reflect[i].params = params;
            //SKYCDBG("MT9T111 REFLECT params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
#if 0        
            mt9t111_params_tbl.reflect[i].num_params = MT9T111_CFG_FFPS_MAX_PARAMS;
            mt9t111_params_tbl.reflect[i].params = &mt9t111_cfg_reflect_params[i][0];
            SKYCDBG("MT9T111 REFLECT params [%d] are loaded from RO data!", i, 0, 0);
#endif			
        }
    }

    for (i = 0; i < MT9T111_CFG_EFFECT_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_mt9t111_tup_get_effect_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            mt9t111_params_tbl.effect[i].num_params = num_params;
            mt9t111_params_tbl.effect[i].params = params;
            //SKYCDBG("MT9T111 EFFECT params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {
            mt9t111_params_tbl.effect[i].num_params = num_params;
            mt9t111_params_tbl.effect[i].params = params;
            //SKYCDBG("MT9T111 EFFECT params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
#if 0        
            mt9t111_params_tbl.effect[i].num_params = MT9T111_CFG_EFFECT_MAX_PARAMS;
            mt9t111_params_tbl.effect[i].params = &mt9t111_cfg_effect_params[i][0];
            SKYCDBG("MT9T111 EFFECT params [%d] are loaded from RO data!", i, 0, 0);
#endif
	}
    }
	
for (i = 0; i < MT9T111_CFG_FLICKER_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_mt9t111_tup_get_flicker_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            mt9t111_params_tbl.flicker[i].num_params = num_params;
            mt9t111_params_tbl.flicker[i].params = params;
            //SKYCDBG("MT9T111 FLICKER params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {
            mt9t111_params_tbl.flicker[i].num_params = num_params;
            mt9t111_params_tbl.flicker[i].params = params;
	     //SKYCDBG("MT9T111 FLICKER params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
#if 0        
            mt9t111_params_tbl.flicker[i].num_params = MT9T111_CFG_FLICKER_MAX_PARAMS;
            mt9t111_params_tbl.flicker[i].params = &mt9t111_cfg_flicker_params[i][0];
            SKYCDBG("MT9T111 FLICKER params [%d] are loaded from RO data!", i, 0, 0);
#endif
	}
    }

for (i = 0; i < MT9T111_CFG_ANTISHAKE_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_mt9t111_tup_get_antishake_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            mt9t111_params_tbl.antishake[i].num_params = num_params;
            mt9t111_params_tbl.antishake[i].params = params;
            //SKYCDBG("MT9T111 ANTISHAKE params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {
            mt9t111_params_tbl.antishake[i].num_params = num_params;
            mt9t111_params_tbl.antishake[i].params = params;
            //SKYCDBG("MT9T111 ANTISHAKE params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
#if 0        
            mt9t111_params_tbl.antishake[i].num_params = MT9T111_CFG_ANTISHAKE_MAX_PARAMS;
            mt9t111_params_tbl.antishake[i].params = &mt9t111_cfg_antishake_params[i][0];
            SKYCDBG("MT9T111 ANTISHAKE params [%d] are loaded from RO data!", i, 0, 0);
#endif
	}
    }

for (i = 0; i < MT9T111_CFG_FOCUS_STEP_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_mt9t111_tup_get_focus_step_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            mt9t111_params_tbl.focus_step[i].num_params = num_params;
            mt9t111_params_tbl.focus_step[i].params = params;
            //SKYCDBG("MT9T111 FOCUS_STEP params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {
            mt9t111_params_tbl.focus_step[i].num_params = num_params;
            mt9t111_params_tbl.focus_step[i].params = params;
            //SKYCDBG("MT9T111 FOCUS_STEP params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
#if 0        
            mt9t111_params_tbl.focus_step[i].num_params = MT9T111_CFG_FOCUS_STEP_MAX_PARAMS;
            mt9t111_params_tbl.focus_step[i].params = &mt9t111_cfg_focus_step_params[i][0];
            SKYCDBG("MT9T111 FOCUS_STEP params [%d] are loaded from RO data!", i, 0, 0);
#endif
	}
    }

for (i = 0; i < MT9T111_CFG_SCENE_MODE_MAX; i++)
    {
#if defined(F_SKYCAM_SENSOR_TUNEUP)
        num_params = camsensor_mt9t111_tup_get_scene_params(i, &params);
        if ((num_params > 0) && (params != NULL))
        {
            mt9t111_params_tbl.scene[i].num_params = num_params;
            mt9t111_params_tbl.scene[i].params = params;
            //SKYCDBG("MT9T111 SCENE params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
        }
        else
#endif /* F_SKYCAM_SENSOR_TUNEUP */
        {
            mt9t111_params_tbl.scene[i].num_params = num_params;
            mt9t111_params_tbl.scene[i].params = params;
            //SKYCDBG("MT9T111 SCENE params [%d] are loaded from TUNEUP file!\n", i, 0, 0);
#if 0        
            mt9t111_params_tbl.scene[i].num_params = MT9T111_CFG_SCENE_MAX_PARAMS;
            mt9t111_params_tbl.scene[i].params = &mt9t111_cfg_scene_params[i][0];
            SKYCDBG("MT9T111 SCENE params [%d] are loaded from RO data!", i, 0, 0);
#endif
	}
    }    

    SKYCDBG("<<%s ()", __func__, 0, 0);
    return;
}
#endif
#ifdef F_SKYCAM_TUP_LOAD_FILE
static int mt9t111_set_tune_value(const struct sensor_cfg_data *scfg)
{
	int32_t rc = 0;
	int32_t loop_count = 0;
	int32_t i = 0;
	int32_t last_size = 0;
	int32_t copy_size = 0;
	uint8_t * pKTune_stream = NULL;
	uint8_t * pUTune_stream = NULL;
	
if(mt9t111_tup_state != MT9T111_TUNE_STATE_LOAD_VALUE)
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
SKYCDBG(" i =%d, loop_count=%d, copy_size=%d, pKTune_stream=%x\n", i, loop_count, copy_size, pKTune_stream);
/* sdcard/튜닝.txt 가 없을때 내장값 적용 테스트용 //you4me20110106
       if(pKTune_stream==NULL) 
        {
            mt9t111_tup_state = MT9T111_TUNE_STATE_NONE;
            rc = -EFAULT;
		goto update_fw_user_fail;
        }
*/       
	//파일 스트림에서 개별 튜닝값을 배열에 로딩
	mt9t111_init_params_tbl(pKTune_stream);

	mt9t111_tup_state = MT9T111_TUNE_STATE_LOAD_VALUE;
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
	SKYCERR("%s error. rc=%d\n", __func__, rc);
	return rc;
}
#endif
#if 0
static int mt9t111_set_tup_params(const struct sensor_cfg_data *scfg)
{
}
#endif

static int32_t mt9t111_video_config(void)
{
	int32_t rc = 0;
	int8_t npolling = -1;
	int i;
	
	/* set preview resolution to 1280x960 */
	SKYCDBG("%s start\n",__func__);
#ifdef F_SKYCAM_TUP_LOAD_FILE
	if(mt9t111_tup_state == MT9T111_TUNE_STATE_LOAD_VALUE)
	{		
		rc = mt9t111_reg_init();
		if (rc < 0)
		{
			SKYCERR("mt9t111_i2c_write_table FAIL!!! return~~\n");
			return rc;
		}		
	}
	else
#endif		
	{
	rc = mt9t111_i2c_write_table(&mt9t111_regs.preview_cfg_settings[0],
					mt9t111_regs.preview_cfg_settings_size);
	}

#if 0
	mt9t111_i2c_read_table(&mt9t111_regs.preview_cfg_settings[0],
					mt9t111_regs.preview_cfg_settings_size);
#endif
#if 0
	for (i = 0; i < MT9T111_PREVIEW_RETRY; i++) 
	{
		rc = mt9t111_i2c_read(mt9t111_client->addr,	0x8404, &npolling,TRIPLE_LEN);
		if (rc < 0)
		{
			SKYCERR("mt9t111_snapshot_config read  FAIL!!! return~~\n");
			//return rc;
		}
		SKYCDBG("%s: retry npolling = %x, count = %d\n", __func__,npolling, i);

		if (npolling == 0) {	
			//msleep(300);
			return 0;
		}		
		msleep(20);
	}
#endif

	if (rc < 0)
	{
		SKYCERR("mt9t111_i2c_write_table FAIL!!! return~~\n");
		return rc;
	}
	SKYCDBG("%s end rc = %d\n",__func__, rc);

	return 0;
}

static int32_t mt9t111_snapshot_config(void)
{
	int32_t rc = 0;
	int8_t npolling = -1;
	int i;

	/* set snapshot resolution to 2592x1944 */
	SKYCDBG("%s start\n",__func__);
	rc = mt9t111_i2c_write_table(&mt9t111_regs.snapshot_cfg_settings[0],
					mt9t111_regs.snapshot_cfg_settings_size);
#if 0 //Preview와 Snapshot의 색감차이로 인해 polling동작을 레지스터 값에서 수정	
	for (i = 0; i < MT9T111_SNAPSHOT_RETRY; i++) {
		rc = mt9t111_i2c_read(mt9t111_client->addr,
		0x8404, &npolling,TRIPLE_LEN);
		if (rc < 0)
		{
			SKYCERR("mt9t111_snapshot_config read  FAIL!!! return~~\n");
			//return rc;
		}
		SKYCDBG("%s: retry npolling = %x, count = %d\n", __func__,npolling, i);
		if (npolling == 0) {	
			//msleep(300);
			return 0;
		}		
		msleep(20);
	}	
#endif	
#if 0
	mt9t111_i2c_read_table(&mt9t111_regs.snapshot_cfg_settings[0],
					mt9t111_regs.snapshot_cfg_settings_size);
#endif
	if (rc < 0)
	{
		SKYCERR("mt9t111_i2c_write_table FAIL!!! return~~\n");
		return rc;
	}

	SKYCDBG("%s end rc = %d\n",__func__, rc);
	
	return 0;
}

static int32_t mt9t111_set_sensor_mode(int mode)
{
	uint16_t clock;
	long rc = 0;
#ifdef F_SKYCAM_FIX_CFG_LED_MODE	
	int8_t nbrightness = 0;
#endif	

	SKYCDBG("%s start\n",__func__);
	switch (mode) {
	case SENSOR_PREVIEW_MODE:
		rc = mt9t111_video_config();
		multi_count = 0;
		SKYCDBG("mt9t111_video_config end multi count =%d, rc = %d\n",  multi_count, rc);
		break;

	case SENSOR_SNAPSHOT_MODE:
		/* Switch to lower fps for Snapshot */		
#ifdef F_SKYCAM_FIX_CFG_LED_MODE		
		if(led_auto == 1) {		
			rc = mt9t111_i2c_write(mt9t111_client->addr,
			0x098E, 0xB804,WORD_LEN);
			rc = mt9t111_i2c_read(mt9t111_client->addr,
			0xB804, &nbrightness,TRIPLE_LEN);
			SKYCDBG("%s SENSOR_SNAPSHOT_MODE nbrightness =0x%x\n",__func__, nbrightness);
			if(nbrightness <= 0x38)				
				mt9t111_set_led_mode(2);
		}
#endif		
		if(multi_count ==0)
			rc = mt9t111_snapshot_config();
		
		SKYCDBG("mt9t111_snapshot_config end multi count =%d, rc = %d\n", multi_count, rc);
		multi_count++;
		
		break;

	case SENSOR_RAW_SNAPSHOT_MODE:
		/* Setting the effect to CAMERA_EFFECT_OFF */
		rc = mt9t111_snapshot_config();		
		break;
	default:
		SKYCERR("mt9t111_set_sensor_mode DEFAULT FAIL[EINVAL] = %d\n", EINVAL);
		return -EINVAL;
	}

	SKYCDBG("%s end rc = %d\n",__func__, rc);
	return rc;
}

static int mt9t111_probe_init_done( const struct msm_camera_sensor_info *data)
{
int rc =0;
	SKYCDBG("%s start\n",__func__);
#ifdef F_SKYCAM_GPIO_SET
	mt9t111_gpio_set(data->sensor_reset, 0);
#else
	gpio_direction_output(data->sensor_reset, 0);
	gpio_free(data->sensor_reset);
	SKYCDBG("%s end\n",__func__);
#endif

	return 0;
}

static int mt9t111_sensor_init_probe(const struct msm_camera_sensor_info *data)
{
	uint16_t model_id = 0;
	int rc = 0;

	SKYCDBG("init entry \n");
	rc = mt9t111_reset(data, ON);
	if (rc < 0) {
		SKYCERR("reset failed!\n");
		goto init_probe_fail;
	}
	mdelay(5);

#ifdef F_SKYCAM_TUP_LOAD_FILE
	if(mt9t111_tup_mode == MT9T111_TUNE_STATE_TUNNING_MODE_ON)
		return rc;
#endif	

	rc = mt9t111_reg_init();
	if (rc < 0)
		goto init_probe_fail;

	return rc;

init_probe_fail:
	SKYCERR("%s failed! (%d)\n", __func__, rc);
	return rc;
}

static int mt9t111_sensor_init(const struct msm_camera_sensor_info *data)
{
	int rc = 0;
	SKYCDBG("%s start\n", __func__);
#ifdef F_SKYCAM_DRIVER_COMPLETE
	int cam_loop_count =0;
	do{
		mdelay(5);
		if(cam_loop_count++>100)
		{
		
			break;
		}
		SKYCDBG(" bFRONT_cam_on wait %d \n",5*cam_loop_count);
	}while(bFRONT_cam_on);
	
#endif	
	rc = mt9t111_power(ON);
	if (rc) {
		rc = -ENOENT;
		SKYCERR(" mt9t111_power failed rc=%d\n",rc);
		goto init_fail;
	}
	
	mt9t111_ctrl = kzalloc(sizeof(struct mt9t111_ctrl_t), GFP_KERNEL);
	SKYCDBG(" mt9t111_ctrl [ kzalloc ] mt9t111_ctrl=%x\n",mt9t111_ctrl);
	if (!mt9t111_ctrl) {
		SKYCERR("mt9t111_init failed!\n");
		rc = -ENOMEM;
		goto init_done;
	}

	if (data)
		mt9t111_ctrl->sensordata = data;

	/* Input MCLK = 24MHz */
	msm_camio_clk_rate_set(MT9T111_DEFAULT_CLOCK_RATE);
	mdelay(5);

	msm_camio_camif_pad_reg_reset();

	rc = mt9t111_sensor_init_probe(data);
	SKYCDBG(" mt9t111_sensor_init_probe(data); rc =%d\n",rc);
	if (rc < 0) {
		SKYCERR("mt9t111_sensor_init failed!\n");
		goto init_fail;
	}
#ifdef F_SKYCAM_DRIVER_COMPLETE	
	bBACK_cam_on=true;
#endif

init_done:
	SKYCDBG("%s init_done\n",__func__);
	return rc;

init_fail:
	SKYCERR("%s failed! (%d)\n", __func__, rc);
	mt9t111_probe_init_done(data);
	SKYCERR(" %s failed!init done!!!\n");
	kfree(mt9t111_ctrl);
	return rc;
}

static int mt9t111_init_client(struct i2c_client *client)
{
	/* Initialize the MSM_CAMI2C Chip */
	init_waitqueue_head(&mt9t111_wait_queue);
	return 0;
}

static int mt9t111_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cfg_data;
	int32_t   rc = 0;

	SKYCDBG("%s start\n",__func__);
	if (copy_from_user(&cfg_data,
			(void *)argp,
			sizeof(struct sensor_cfg_data)))
		return -EFAULT;

	/* down(&mt9t111_sem); */

	SKYCDBG("mt9t111_ioctl, cfgtype = %d, mode = %d\n",
		cfg_data.cfgtype, cfg_data.mode);

		switch (cfg_data.cfgtype) {
		case CFG_SET_MODE:
			rc = mt9t111_set_sensor_mode(cfg_data.mode);
			SKYCDBG("mt9t111_set_sensor_mode OK! rc = [%d], cfg_data.mode = [%d]\n", rc, cfg_data.mode);
			break;
#ifdef F_SKYCAM_FIX_CFG_EFFECT
		case CFG_SET_EFFECT:
			rc = mt9t111_set_effect(cfg_data.cfg.effect);
			SKYCDBG("mt9t111_set_effect OK! rc = [%d], cfg_data.mode = [%d], cfg_data.cfg.effect =[%d]\n", rc, cfg_data.mode, cfg_data.cfg.effect);			
			break;
#endif
#ifdef F_SKYCAM_FIX_CFG_WB
		case CFG_SET_WB:			
			rc = mt9t111_set_whitebalance(cfg_data.cfg.wb);
			SKYCDBG("mt9t111_set_whitebalance OK! rc = [%d], cfg_data.mode = [%d], cfg_data.cfg.whitebalance =[%d]\n", rc, cfg_data.mode, cfg_data.cfg.wb);
			break;
#endif					
#ifdef F_SKYCAM_FIX_CFG_BRIGHTNESS
		case CFG_SET_BRIGHTNESS:
			rc = mt9t111_set_brightness(cfg_data.cfg.brightness);
			SKYCDBG("mt9t111_set_brightness OK! rc = [%d], cfg_data.cfg.brightness =[%d]\n", rc, cfg_data.cfg.brightness);
			break;
#endif			
#ifdef F_SKYCAM_FIX_CFG_LED_MODE
		case CFG_SET_LED_MODE:
			if(cfg_data.cfg.led_mode == 1)  led_auto = 1;
			else led_auto = 0;

			rc = mt9t111_set_led_mode(cfg_data.cfg.led_mode);
			SKYCDBG("mt9t111_set_led_mode OK! rc = [%d], cfg_data.cfg.led_mode =[%d]\n", rc, cfg_data.cfg.led_mode);
		break;
#endif
#ifdef F_SKYCAM_FIX_CFG_AF
	case CFG_AUTO_FOCUS:		
			rc = mt9t111_set_auto_focus();
			SKYCDBG("%s(CFG_AUTO_FOCUS), rc =%d\n", __func__, rc);
		break;
#endif
#ifdef F_SKYCAM_FIX_CFG_ISO
	case CFG_SET_ISO:
		rc = mt9t111_set_iso(cfg_data.cfg.iso);
		SKYCDBG("%s(CFG_SET_ISO), rc=%d\n", __func__, rc);
		break;	
#endif
#ifdef F_SKYCAM_FIX_CFG_SCENE_MODE
	case CFG_SET_SCENE_MODE:
		rc = mt9t111_set_scene_mode(cfg_data.cfg.scene_mode);
		SKYCDBG("%s(CFG_SET_SCENE_MODE), rc=%d\n", __func__, rc);
		break;	
#endif
#ifdef F_SKYCAM_FIX_CFG_FOCUS_STEP
	case CFG_SET_FOCUS_STEP:
		rc = mt9t111_set_focus_step(cfg_data.cfg.focus_step);
		SKYCDBG("%s(CFG_SET_FOCUS_STEP), rc=%d\n", __func__, rc);
		break;
#endif
#ifdef F_SKYCAM_ADD_CFG_ANTISHAKE
	case CFG_SET_ANTISHAKE:
		rc = mt9t111_set_antishake(cfg_data.cfg.antishake);
		SKYCDBG("%s(CFG_SET_ANTISHAKE), rc=%d\n", __func__, rc);
		break;
#endif
#ifdef F_SKYCAM_FIX_CFG_EXPOSURE
		case CFG_SET_EXPOSURE_MODE:
			rc = mt9t111_set_exposure_mode(cfg_data.cfg.exposure);
			SKYCDBG("mt9t111_set_exposure_mode OK! rc = [%d], cfg_data.cfg.exposure =[%d]\n", rc, cfg_data.cfg.exposure);
			break;
#endif
#ifdef F_SKYCAM_FIX_CFG_FOCUS_RECT
	case CFG_SET_FOCUS_RECT:
		rc = mt9t111_set_focus_rect(cfg_data.cfg.focus_rect);
		SKYCDBG("%s(CFG_SET_FOCUS_RECT), rc=%d\n", __func__, rc);
		break;
#endif	
#ifdef F_SKYCAM_FIX_CFG_PREVIEW_FPS
	case CFG_SET_PREVIEW_FPS:
			rc = mt9t111_set_preview_fps(cfg_data.cfg.preview_fps);
			SKYCDBG("mt9t111_set_exposure_mode OK! rc = [%d], cfg_data.cfg.preview_fps =[%d]\n", rc, cfg_data.cfg.preview_fps);
			break;
#endif
#ifdef F_SKYCAM_FIX_CFG_ANTIBANDING
	case CFG_SET_ANTIBANDING:
		rc = mt9t111_set_antibanding(cfg_data.cfg.antibanding);
		SKYCDBG("%s: CFG_SET_ANTIBANDING, rc=%d\n", __func__, rc);
		break;
#endif
#ifdef F_SKYCAM_TUP_LOAD_FILE
		case CFG_SET_TUNE_VALUE:
			rc = mt9t111_set_tune_value(&cfg_data);
			break;
#endif
#ifdef F_SKYCAM_FIX_CFG_REFLECT
		case CFG_SET_REFLECT:
			rc = mt9t111_set_reflect(cfg_data.cfg.reflect);
			SKYCDBG("mt9t111_set_reflect OK! rc = [%d], cfg_data.cfg.reflect =[%d]\n", rc, cfg_data.cfg.reflect);
			break;
#endif
#ifdef F_SKYCAM_ADD_CFG_CAF
		case CFG_SET_CONTINUOUS_AF:
			rc = mt9t111_set_continuous_af(cfg_data.cfg.continuous_af);
			SKYCDBG("mt9t111_set_continuous_af OK! rc = [%d], cfg_data.cfg.continuous_af =[%d]\n", rc, cfg_data.cfg.continuous_af);			
			break;
#endif
#ifdef F_SKYCAM_ADD_CFG_READ_REG
	case CFG_GET_REG:		
			cfg_data.cfg.focus_rect = mt9t111_read_reg(cfg_data.cfg.focus_rect & 0xffff);
			
			copy_to_user((void *)argp,
					&cfg_data,
					sizeof(struct sensor_cfg_data));

			
			SKYCDBG("%s(CFG_GET_REG), rc =%d\n", __func__, cfg_data.cfg.focus_rect);
		break;		
#endif
		default:
			//rc = -EINVAL;			
			rc = 0;
			SKYCERR("mt9t111_video_config DEFAULT FAIL!!![EINVAL] rc = [%d]\n", EINVAL);
			break;
		}

	/* up(&mt9t111_sem); */
	SKYCDBG("%s end rc = %d\n",__func__, rc);
	return rc;
}

static int mt9t111_sensor_release(void)
{	
	int32_t rc = 0;
	SKYCDBG("%s start\n",__func__);
#ifdef F_SKYCAM_FIX_CFG_LED_MODE	
	mt9t111_set_led_mode(0);
#endif

       mt9t111_reset(mt9t111_ctrl->sensordata, OFF);

	rc = mt9t111_power(OFF);
	if (rc)
		goto sensor_release_fail;

	/* down(&mt9t111_sem); */
	kfree(mt9t111_ctrl);
	mt9t111_ctrl = NULL;
	/* up(&mt9t111_sem); */
#ifdef F_SKYCAM_TUP_LOAD_FILE
	//튜닝값 로딩 체크 flag
	mt9t111_tup_state = MT9T111_TUNE_STATE_NONE;
	//mt9t111_done_set_tune_load = FALSE;
	//mt9t111_done_set_tune_value = FALSE;
	//mt9t111_tup_mode = MT9T111_TUNE_STATE_TUNNING_MODE_OFF;
#endif
	SKYCDBG("%s end rc = %d\n",__func__, rc);
#ifdef F_SKYCAM_DRIVER_COMPLETE	
	bBACK_cam_on=false;
#endif
	return rc;

sensor_release_fail:
	SKYCERR("%s failed! (%d)\n", __func__, rc);
	return rc;
}

static int mt9t111_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		rc = -ENOTSUPP;
		goto probe_failure;
	}

	mt9t111_sensorw = kzalloc(sizeof(struct mt9t111_work), GFP_KERNEL);
	if (!mt9t111_sensorw) {
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, mt9t111_sensorw);
	mt9t111_init_client(client);
	mt9t111_client = client;

	SKYCDBG("mt9t111_probe succeeded!\n");

	return 0;

probe_failure:
	kfree(mt9t111_sensorw);
	mt9t111_sensorw = NULL;
	SKYCERR("mt9t111_probe failed!\n");
	return rc;
}

static const struct i2c_device_id mt9t111_i2c_id[] = {
	{ "mt9t111", 0},
	{ },
};

static struct i2c_driver mt9t111_i2c_driver = {
	.id_table = mt9t111_i2c_id,
	.probe  = mt9t111_i2c_probe,
	.remove = __exit_p(mt9t111_i2c_remove),
	.driver = {
		.name = "mt9t111",
	},
};

static int32_t mt9t111_init_i2c(void)
{
	int32_t rc = 0;

	SKYCDBG("%s start\n",__func__);

	rc = i2c_add_driver(&mt9t111_i2c_driver);
	SKYCDBG("%s mt9t111_i2c_driver rc = %d\n",__func__, rc);
	if (IS_ERR_VALUE(rc))
		goto init_i2c_fail;

	SKYCDBG("%s end\n",__func__);
	return 0;

init_i2c_fail:
	SKYCERR("%s failed! (%d)\n", __func__, rc);
	return rc;
}

static int mt9t111_sensor_probe(const struct msm_camera_sensor_info *info,
				struct msm_sensor_ctrl *s)
{
	int rc = 0;
	SKYCDBG("%s start\n",__func__);
	
	rc = mt9t111_init_i2c();	
	if (rc < 0 || mt9t111_client == NULL)
	{
		SKYCERR("%s rc = %d, mt9t111_client = %x\n",__func__, rc, mt9t111_client);
		goto probe_init_fail;
	}

#if 0 // for i2c test
	rc = mt9t111_power(ON); // Make 3M camera power module
	if (rc)
		goto probe_init_fail;
	msm_camio_clk_rate_set(MT9T111_DEFAULT_CLOCK_RATE);
	mdelay(20);

	rc = mt9t111_sensor_init_probe(info);
	if( rc < 0)
		goto probe_init_fail;

	rc = mt9t111_power(OFF);
	SKYCDBG("%s successed!\n", __func__);
#endif

	s->s_init = mt9t111_sensor_init;
	s->s_release = mt9t111_sensor_release;
	s->s_config  = mt9t111_sensor_config;
	s->s_mount_angle  = 0;
#ifdef F_SKYCAM_FIX_CFG_LED_MODE
	pm_lib_ep = msm_rpc_connect(PM_LIBPROG, CompVer, 0);
	if (IS_ERR(pm_lib_ep)) {
		printk(KERN_ERR "%s: init rpc failed! rc = %ld\n",
			__func__, PTR_ERR(pm_lib_ep));
		rc = PTR_ERR(pm_lib_ep);
	}
	SKYCDBG("FLASH init rpc  succeeded! pm_lib_ep = %x\n", pm_lib_ep);
#endif	

	return 0;

probe_init_fail:
	SKYCERR("%s Failed!\n", __func__);
	return 0;
}

static int __mt9t111_probe(struct platform_device *pdev)
{
	return msm_camera_drv_start(pdev, mt9t111_sensor_probe);
}

static struct platform_driver msm_camera_driver = {
	.probe = __mt9t111_probe,
	.driver = {
		.name = "msm_camera_mt9t111",
		.owner = THIS_MODULE,
	},
};

static int __init mt9t111_init(void)
{
	SKYCDBG("%s start\n", __func__);
	return platform_driver_register(&msm_camera_driver);
}

void mt9t111_exit(void)
{
	SKYCDBG("%s is called.\n", __func__);
	i2c_del_driver(&mt9t111_i2c_driver);
}

module_init(mt9t111_init);
module_exit(mt9t111_exit);
