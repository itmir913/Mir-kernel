/* Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora Forum nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * Alternatively, provided that this notice is retained in full, this software
 * may be relicensed by the recipient under the terms of the GNU General Public
 * License version 2 ("GPL") and only version 2, in which case the provisions of
 * the GPL apply INSTEAD OF those given above.  If the recipient relicenses the
 * software under the GPL, then the identification text in the MODULE_LICENSE
 * macro must be changed to reflect "GPLv2" instead of "Dual BSD/GPL".  Once a
 * recipient changes the license terms to the GPL, subsequent recipients shall
 * not relicense under alternate licensing terms, including the BSD or dual
 * BSD/GPL terms.  In addition, the following license statement immediately
 * below and between the words START and END shall also then apply when this
 * software is relicensed under the GPL:
 *
 * START
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 and only version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * END
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*
 * MSM architecture driver to reset the modem
 */

#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

#include <mach/sky_smem.h>

#include "sky_sys_reset.h"
#include "smd_private.h"
#include "proc_comm.h"

#define DEBUG
/* #undef DEBUG */
#ifdef DEBUG
#define D(x...) printk(x)
#else
#define D(x...) do {} while (0)
#endif

typedef enum {
    USER_RESET = 0x00000000,
    SW_RESET = 0x00000001,
    PDL_RESET = 0x00000002,
}SYS_RST_RESET_TYPE_E;

typedef enum{
  MAIN_LCD_BL_OFF = 0x00000000,
  MAIN_LCD_BL_ON  = 0x0A0F090F,
}SYS_RST_LCD_BL_STATE_E;

typedef enum{
  RST_LCD_BL_OFF=0x00000000,
  RST_LCD_BL_ON =0x00000001,
  RST_LCD_BL_USER=0x00000002,
}SYS_RST_LCD_BL_E;

typedef struct{
  uint32_t reason;
  uint32_t reset;
  uint32_t backlight;
  uint32_t silent_boot;
  char log_buffer[512];
}SYS_RST_SMEM_T;

typedef SYS_RST_SMEM_T  smem_id_vendor0_type;

static volatile smem_id_vendor0_type *smem_id_vendor0_ptr;
// Inserted by DS2_dscheon for Power OnOff reason Info
static uint32_t rst_prev_reason, rst_prev_reset, rst_prev_backlight, rst_prev_silent_boot;

extern unsigned int msm_fb_get_offset(void);


#ifdef FEATURE_SKY_CHG_LOGO
static uint32_t rst_charging_state;
enum{
  SKY_CHG_NONE_MODE              = 0x00000000,
  SKY_CHG_ONLINE_NORMAL_MODE     = 0x12345678,
  SKY_CHG_OFFLINE_CHARGING_MODE  = 0x20302030,
  SKY_CHG_POWER_OFF_MODE         = 0x34303430,
  SKY_CHG_RESET_MODE             = 0x32153215
};

typedef struct {
  uint32_t chg_magic_num;
  uint32_t chg_boot_mode;
 // add if needed..
}SKY_CHG_SMEM_T;

typedef SKY_CHG_SMEM_T smem_id_vendor2_type;
static volatile smem_id_vendor2_type *smem_id_vendor2_ptr;

uint32_t sky_sys_rst_Get_ChargingMode(void)
{
  uint32_t mode = 0;
  if(smem_id_vendor2_ptr == NULL)
  {
    smem_id_vendor2_ptr = (smem_id_vendor2_type*)smem_alloc(SMEM_ID_VENDOR2, sizeof(smem_id_vendor2_type));
  } 
  if(smem_id_vendor2_ptr->chg_boot_mode == (uint32_t)SKY_CHG_OFFLINE_CHARGING_MODE)
    mode = 1;

  return mode;
}
EXPORT_SYMBOL(sky_sys_rst_Get_ChargingMode);

void sky_sys_rst_Set_ChargingMode(uint32_t mode)
{
  if(smem_id_vendor2_ptr == NULL)
  {
    smem_id_vendor2_ptr = (smem_id_vendor2_type*)smem_alloc(SMEM_ID_VENDOR2, sizeof(smem_id_vendor2_type));
  }
  
  smem_id_vendor2_ptr->chg_boot_mode = mode;
}
EXPORT_SYMBOL(sky_sys_rst_Set_ChargingMode);

#endif


/*
* ** FUNCTION DEFINATION ***
*/
uint32_t sky_sys_rst_GetResetReason(void)
{
	if(smem_id_vendor0_ptr == NULL)
	{
		smem_id_vendor0_ptr = (smem_id_vendor0_type*)smem_alloc(SMEM_ID_VENDOR0, sizeof(smem_id_vendor0_type));
	} 
	return smem_id_vendor0_ptr->reason;
}
EXPORT_SYMBOL(sky_sys_rst_GetResetReason);

uint32_t sky_sys_rst_GetReset(void)
{
	if(smem_id_vendor0_ptr == NULL)
	{
		smem_id_vendor0_ptr = (smem_id_vendor0_type*)smem_alloc(SMEM_ID_VENDOR0, sizeof(smem_id_vendor0_type));
	} 
	return smem_id_vendor0_ptr->reset;
}
EXPORT_SYMBOL(sky_sys_rst_GetReset);

uint32_t sky_sys_rst_GetLcdBLStatus(void)
{
	if(smem_id_vendor0_ptr == NULL)
	{
		smem_id_vendor0_ptr = (smem_id_vendor0_type*)smem_alloc(SMEM_ID_VENDOR0, sizeof(smem_id_vendor0_type));
	} 
	return smem_id_vendor0_ptr->backlight;
}
EXPORT_SYMBOL(sky_sys_rst_GetLcdBLStatus);

uint32_t sky_sys_rst_GetSilentBoot(void)
{
	if(smem_id_vendor0_ptr == NULL)
	{
		smem_id_vendor0_ptr = (smem_id_vendor0_type*)smem_alloc(SMEM_ID_VENDOR0, sizeof(smem_id_vendor0_type));
	} 
	return smem_id_vendor0_ptr->silent_boot;
}
EXPORT_SYMBOL(sky_sys_rst_GetSilentBoot);

char *sky_sys_rst_GetLogBuffer(void)
{
	if(smem_id_vendor0_ptr == NULL)
	{
		smem_id_vendor0_ptr = (smem_id_vendor0_type*)smem_alloc(SMEM_ID_VENDOR0, sizeof(smem_id_vendor0_type));
	} 
	return (char*)&smem_id_vendor0_ptr->log_buffer[0];
}
EXPORT_SYMBOL(sky_sys_rst_GetLogBuffer);

int sky_sys_rst_SetSwReset(uint32_t *reason)
{
	int r;	
	uint32_t type; 

	if(smem_id_vendor0_ptr == NULL)
	{
		smem_id_vendor0_ptr = (smem_id_vendor0_type*)smem_alloc(SMEM_ID_VENDOR0, sizeof(smem_id_vendor0_type));
	} 

	type = SMEM_PROC_COMM_CUSTOMER_CMD1_SET_SW_RESET;
	r = msm_proc_comm(PCOM_CUSTOMER_CMD1, &type, reason);
	if(r >= 0)
	{
		smem_id_vendor0_ptr->reset = SW_RESET;	
		if(reason)
			smem_id_vendor0_ptr->reason = *reason;
		else
			smem_id_vendor0_ptr->reason = 0x00000000;
	}
	return r;
}
EXPORT_SYMBOL(sky_sys_rst_SetSwReset);

int sky_sys_rst_SetUserReset(uint32_t *reason)
{
	int r;	
	uint32_t type; 

	if(smem_id_vendor0_ptr == NULL)
	{
		smem_id_vendor0_ptr = (smem_id_vendor0_type*)smem_alloc(SMEM_ID_VENDOR0, sizeof(smem_id_vendor0_type));
	} 

	type = SMEM_PROC_COMM_CUSTOMER_CMD1_SET_USER_RESET;
	r = msm_proc_comm(PCOM_CUSTOMER_CMD1, &type, reason);
	if(r >= 0)
	{
		smem_id_vendor0_ptr->reset = USER_RESET;	
		if(reason)
			smem_id_vendor0_ptr->reason = *reason;
		else
			smem_id_vendor0_ptr->reason = 0x00000000;
	}
	return r;
}
EXPORT_SYMBOL(sky_sys_rst_SetUserReset);

int sky_sys_rst_SetLcdBLStatus(uint32_t eBrightness)
{
	int r;	
	uint32_t type; 

	if(smem_id_vendor0_ptr == NULL)
	{
		smem_id_vendor0_ptr = (smem_id_vendor0_type*)smem_alloc(SMEM_ID_VENDOR0, sizeof(smem_id_vendor0_type));
	} 

	type = SMEM_PROC_COMM_CUSTOMER_CMD1_SET_BL_STATUS;
	r = msm_proc_comm(PCOM_CUSTOMER_CMD1, &type, &eBrightness);
	if(r >= 0)
	{
		if(!eBrightness) {
			smem_id_vendor0_ptr->backlight = RST_LCD_BL_OFF;//MAIN_LCD_BL_OFF;	
		} else {
			smem_id_vendor0_ptr->backlight = RST_LCD_BL_ON;//MAIN_LCD_BL_ON;
		}
	}
	return r;
}
EXPORT_SYMBOL(sky_sys_rst_SetLcdBLStatus);

void sky_sys_rst_SetSilentBoot(uint32_t mode)
{
	if(smem_id_vendor0_ptr == NULL)
	{
		smem_id_vendor0_ptr = (smem_id_vendor0_type*)smem_alloc(SMEM_ID_VENDOR0, sizeof(smem_id_vendor0_type));
	} 
	smem_id_vendor0_ptr->silent_boot = mode;
}
EXPORT_SYMBOL(sky_sys_rst_SetSilentBoot);

int sky_sys_rst_SwReset(uint32_t *reason)
{
	int r;	
	uint32_t type; 

	type = SMEM_PROC_COMM_CUSTOMER_CMD1_SW_RESET;
	r = msm_proc_comm(PCOM_CUSTOMER_CMD1, &type, reason);

	return r;
}
EXPORT_SYMBOL(sky_sys_rst_SwReset);

int sky_sys_rst_UserReset(uint32_t *reason)
{
	int r;	
	uint32_t type; 

	type = SMEM_PROC_COMM_CUSTOMER_CMD1_USER_RESET;
	r = msm_proc_comm(PCOM_CUSTOMER_CMD1, &type, reason);

	return r;
}
EXPORT_SYMBOL(sky_sys_rst_UserReset);

int sky_sys_rst_SwReset_imm(uint32_t *reason)
{
	int r;	
	uint32_t type; 

	type = SMEM_PROC_COMM_CUSTOMER_CMD1_SW_RESET_IMM;
	r = msm_proc_comm(PCOM_CUSTOMER_CMD1, &type, reason);

	return r;
}
EXPORT_SYMBOL(sky_sys_rst_SwReset_imm);

int sky_sys_rst_UserReset_imm(uint32_t *reason)
{
	int r;	
	uint32_t type; 

	type = SMEM_PROC_COMM_CUSTOMER_CMD1_USER_RESET_IMM;
	r = msm_proc_comm(PCOM_CUSTOMER_CMD1, &type, reason);

	return r;
}
EXPORT_SYMBOL(sky_sys_rst_UserReset_imm); 

void sky_sys_rst_PowerDown(void)
{
	uint32_t type; 

	type = SMEM_PROC_COMM_CUSTOMER_CMD1_POWER_DOWN;
	msm_proc_comm(PCOM_CUSTOMER_CMD1, &type, 0);
}
EXPORT_SYMBOL(sky_sys_rst_PowerDown);

void sky_sys_rst_NotiToMARM(uint32_t *reason)
{
	int r;
	uint32_t type;
    uint32_t offset;
	if(smem_id_vendor0_ptr == NULL)
	{
		smem_id_vendor0_ptr = (smem_id_vendor0_type*)smem_alloc(SMEM_ID_VENDOR0, sizeof(smem_id_vendor0_type));
	} 

/// test wjmin 0908 temporary block because msm_fb_get_offset() is not defined
#if 0
    offset = msm_fb_get_offset();
	type = SMEM_PROC_COMM_CUSTOMER_CMD1_FB_OFFSET;
	r = msm_proc_comm(PCOM_CUSTOMER_CMD1, &type, &offset);
#endif
	type = SMEM_PROC_COMM_CUSTOMER_CMD1_NOTI_TO_MARM;
	r = msm_proc_comm(PCOM_CUSTOMER_CMD1, &type, reason);
	if(r >= 0)
	{
		smem_id_vendor0_ptr->reset = SW_RESET;	
		if(reason)
			smem_id_vendor0_ptr->reason = *reason;
		else
			smem_id_vendor0_ptr->reason = 0x00000000;
	}
}
EXPORT_SYMBOL(sky_sys_rst_NotiToMARM);

#if 1  // FEATURE_MAX8899
int vreg_set_msmc2_dvs_level(unsigned plevel)
{
  int r;
  uint32_t type; 

  type = SMEM_PROC_COMM_CUSTOMER_CMD1_MSMC2_DVS_LEVEL;
  r = msm_proc_comm(PCOM_CUSTOMER_CMD1, &type, &plevel);

  return r;
}
EXPORT_SYMBOL(vreg_set_msmc2_dvs_level);
#endif

void sky_sys_rst_set_prev_reset_info(void)
{
	rst_prev_reason = sky_sys_rst_GetResetReason();
	rst_prev_reset = sky_sys_rst_GetReset();
	rst_prev_backlight = sky_sys_rst_GetLcdBLStatus();
	rst_prev_silent_boot = sky_sys_rst_GetSilentBoot();
#ifdef FEATURE_SKY_CHG_LOGO
  rst_charging_state = sky_sys_rst_Get_ChargingMode();
#endif
}
EXPORT_SYMBOL(sky_sys_rst_set_prev_reset_info);

int sky_sys_rst_read_proc_reset_info
	(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	len = sprintf(page, "Reason: 0x%x\n", rst_prev_reason);
	len += sprintf(page + len , "Reset: %d\n", rst_prev_reset);
	len += sprintf(page + len, "Backlight: %d\n", rst_prev_backlight);
	len += sprintf(page + len, "SilentBoot: %d\n", rst_prev_silent_boot);
#ifdef FEATURE_SKY_CHG_LOGO
  len += sprintf(page + len, "OffChargingMode: %d\n", rst_charging_state);
#endif

	// After this is called, silent_boot_mode must have to be 0.
	sky_sys_rst_SetSilentBoot(0);
	return len;
}
EXPORT_SYMBOL(sky_sys_rst_read_proc_reset_info);
// Inserted by DS2_dscheon for Power OnOff reason Info
int sky_sys_rst_write_proc_reset_info(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int len;
	char tbuffer[2];

	if(count > 1 )
		len = 1;
	
	memset(tbuffer, 0x00, 2);

	if(copy_from_user(tbuffer, buffer, len))
		return -EFAULT;
	
	tbuffer[len] = '\0';

	if(tbuffer[0] >= '0' && tbuffer[0] <= '9')
		rst_prev_reset = tbuffer[0] - '0';

#ifdef FEATURE_SKY_CHG_LOGO
  if(tbuffer[0] == 'A' ){
    printk("sky_sys_rst_Set_ChargingMode\n");
    sky_sys_rst_Set_ChargingMode(SKY_CHG_ONLINE_NORMAL_MODE);
  }
#endif

	return len;
}
EXPORT_SYMBOL(sky_sys_rst_write_proc_reset_info);

bool sky_sys_rst_is_silent_boot_mode(void)
{
	if(smem_id_vendor0_ptr == NULL)
	{
		smem_id_vendor0_ptr = (smem_id_vendor0_type*)smem_alloc(SMEM_ID_VENDOR0, sizeof(smem_id_vendor0_type));
	} 

	if(smem_id_vendor0_ptr->silent_boot)
		return true;
	return false;
}
EXPORT_SYMBOL(sky_sys_rst_is_silent_boot_mode);

