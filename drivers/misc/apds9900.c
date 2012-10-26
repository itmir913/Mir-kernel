/*
 *  apds9900.c - Linux kernel modules for ambient light + proximity sensor
 *
 *  Copyright (C) 2010 Lee Kai Koon <kai-koon.lee@avagotech.com>
 *  Copyright (C) 2010 Avago Technologies
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <mach/vreg.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <linux/earlysuspend.h> //ds2 team shs DS. add hearder file
#include <linux/syscalls.h>
#include <mach/gpio.h>
#include <linux/apds9900.h>
#include <linux/jiffies.h>

#define APDS9900_DRV_NAME	"apds9900"
#define DRIVER_VERSION		"1.0.0"

// #if EF18K_BDVER_GE(EF18K_PT10) || EF18B_BDVER_GE(EF18B_PT10)
#ifdef CONFIG_EF31S_32K_BOARD
#define APDS9900_VREG_STR	"wlan"	// L13
#define APDS9900_VREG_VOLT	2850
#elif EF18K_PT10 //EF18K_BDVER_GE(EF18K_PT10) || EF18B_BDVER_GE(EF18B_PT10)
#define APDS9900_VREG_STR	"wlan"	// L13
#define APDS9900_VREG_VOLT	2850
#else
#error "[apds9900.c] BOARD_VER is not defined."
#endif

/* -------------------------------------------------------------------- */
/* debug option */
/* -------------------------------------------------------------------- */
#define SENSOR_APDS9900_DBG_ENABLE

//#define SENSOR_APDS9900_DBG_FUNC
//#define SENSOR_APDS9900_DBG_DATA
#ifdef SENSOR_APDS9900_DBG_ENABLE
#define dbg(fmt, args...)   printk("[APDS9900] " fmt, ##args)
#else
#define dbg(fmt, args...)
#endif

#ifdef SENSOR_APDS9900_DBG_FUNC
#define dbg_func_in()       dbg("[FUNC_IN] %s\n", __func__)
#define dbg_func_out()      dbg("[FUNC_OUT] %s\n", __func__)
#define dbg_line()          dbg("[LINE] %d(%s)\n", __LINE__, __func__)
#else
#define dbg_func_in()       
#define dbg_func_out()    
#define dbg_line()          
#endif

#ifdef SENSOR_APDS9900_DBG_FUNC
#define dbg_data(fmt, args...)          printk("[APDS9900] " fmt, ##args)
#else
#define dbg_data(fmt, args...)          
#endif


/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/* SKY BOARD FEATURE */
/* -------------------------------------------------------------------- */
#ifdef CONFIG_EF31S_32K_BOARD
#define APDS9900_ACTIVE_NEW
//#define APDS9900_FROYO_UPDATE
#define APDS9900_PS_INT_N				35
#define APDS9900_THRESHOLD_TUNNING
#else
#define APDS9900_PS_INT_N				42
#endif

#define APDS9900_PS_IRQ					gpio_to_irq(APDS9900_PS_INT_N)
#define APDS9900_PS_INT_THRESHOLD		500
#define APDS9900_PS_INT_THRESHOLD_LOWER		400
#define APDS9900_PS_INT_THR_SHIFT		4 //ds2 team shs DS. add DEFINE
#define HYSTERESIS_PS_BUF_SIZE			7
#define HYSTERESIS_LS_BUF_SIZE			4
/* -------------------------------------------------------------------- */
/* APDS9900 PROTOCOL */
/* -------------------------------------------------------------------- */
#define CMD_BYTE	0x80
#define CMD_WORD	0xA0
#define CMD_SPECIAL	0xE0

#define APDS9900_ENABLE_REG	(CMD_BYTE+0x00)
#define APDS9900_ATIME_REG	(CMD_BYTE+0x01)
#define APDS9900_PTIME_REG		(CMD_BYTE+0x02)
#define APDS9900_WTIME_REG	(CMD_BYTE+0x03)
#define APDS9900_AILTL_REG		(CMD_BYTE+0x04)
#define APDS9900_AILTH_REG		(CMD_BYTE+0x05)
#define APDS9900_AIHTL_REG		(CMD_BYTE+0x06)
#define APDS9900_AIHTH_REG	(CMD_BYTE+0x07)
#define APDS9900_PILTL_REG		(CMD_BYTE+0x08)
#define APDS9900_PILTH_REG		(CMD_BYTE+0x09)
#define APDS9900_PIHTL_REG		(CMD_BYTE+0x0A)
#define APDS9900_PIHTH_REG		(CMD_BYTE+0x0B)
#define APDS9900_PERS_REG		(CMD_BYTE+0x0C)
#define APDS9900_CONFIG_REG	(CMD_BYTE+0x0D)
#define APDS9900_PPCOUNT_REG	(CMD_BYTE+0x0E)
#define APDS9900_CONTROL_REG	(CMD_BYTE+0x0F)
#define APDS9900_REV_REG		(CMD_BYTE+0x11)
#define APDS9900_ID_REG		(CMD_BYTE+0x12)
#define APDS9900_STATUS_REG	(CMD_BYTE+0x13)
#define APDS9900_CDATAL_REG	(CMD_BYTE+0x14)
#define APDS9900_CDATAH_REG	(CMD_BYTE+0x15)
#define APDS9900_IRDATAL_REG	(CMD_BYTE+0x16)
#define APDS9900_IRDATAH_REG	(CMD_BYTE+0x17)
#define APDS9900_PDATAL_REG	(CMD_BYTE+0x18)
#define APDS9900_PDATAH_REG	(CMD_BYTE+0x19)


#define CMD_CLR_PS_INT	0xE5
#define CMD_CLR_ALS_INT	0xE6
#define CMD_CLR_PS_ALS_INT	0xE7

/*
 * Structs
 */

typedef enum { //ds2 team shs DS. add stats_e 
	E_DISABLE = 0,
	E_POWER_OFF = E_DISABLE,
	E_INCALL_OFF = E_DISABLE,
	E_UNAVAILABLE = E_DISABLE,
	E_HYSTERESIS_NOT = E_DISABLE,
	E_ENABLE = 1,
	E_POWER_ON = E_ENABLE,
	E_INCALL_ON = E_ENABLE,
	E_AVAILABLE = E_ENABLE,
	E_HYSTERESIS_ON = E_ENABLE,
} status_e;

typedef struct { //ds2 team shs DS. add register_t
	unsigned int enable;
	unsigned int atime;
	unsigned int ptime;
	unsigned int wtime;
	unsigned int ppcount;
	unsigned int control;
	unsigned int interrupt;
	unsigned int piltl; //DS2 Team Shs DS.proximity interrupt value.
	unsigned int pihtl;//DS2 Team Shs DS.proximity interrupt value.
	unsigned int pers;//DS2 Team Shs DS.proximity interrupt value.
	
} register_t;

typedef struct {  //ds2 team shs DS. add hysteresis_t
	u8 *buf;
	int index;
	axes_t lastaxes;
} hysteresis_t;


typedef struct {
	struct i2c_client	*client;
	struct mutex		i2clock;
	struct mutex		incalllock;
	status_e			power;
	status_e			enirq;
	status_e			available;
	status_e			incall;
	active_e			active; // 0:none / 1:proximity / 2:light / 3:proximity+light	
#ifdef APDS9900_ACTIVE_NEW
	active_e			active_app; // 0:none / 1:proximity / 2:light / 3:proximity+light	
#endif	

#ifdef APDS9900_THRESHOLD_TUNNING
	u16				threshold_value;
	u16				threshold_value_lower;
#endif
	register_t			reg;
	hysteresis_t		hysps;
	hysteresis_t		hysls;
	u8				ps_adc;//[DS 2 TEAM] SHS : this value is ps adc last value 
	u8 				ls_adc; //[DS 2 TEAM] SHS : this value is ls adc last value
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;	
#endif
} apds9900data_t;


/*
 * Proximity and Light Table
 */
/*
const axes_t APDS9900_TABLE_PROXIMITY[2] = {
	{	5	,	0	,	9	},	// [0] 0   <= adc < 9   : 5    cm
	{	0	,	1	,	16	},	// [1] 9   <= adc < 16  : 0    cm
};
const axes_t APDS9900_TABLE_LIGHT[13] = {
	{	0	,	0	,	1	},	// [0]   0 <= adc <   1 : 0    Lux
	{	10	,	0	,	2	},	// [0]   1 <= adc <   2 : 10   Lux
	{	50	,	0	,	7	},	// [0]   2 <= adc <   7 : 50   Lux
	{	100	,	0	,	16	},  // [0]   7 <= adc <  16 : 100  Lux
	{	200	,	1	,	26	},  // [1]  16 <= adc <  26 : 200  Lux
	{	300	,	1	,	36	},  // [1]  26 <= adc <  36 : 300  Lux
	{	400	,	1	,	46	},  // [1]  36 <= adc <  46 : 400  Lux
	{	500	,	1	,	57	},  // [1]  46 <= adc <  57 : 500  Lux
	{	600	,	1	,	67	},  // [1]  57 <= adc <  67 : 600  Lux
	{	700	,	1	,	77	},  // [1]  67 <= adc <  77 : 700  Lux
	{	800	,	1	,	87	},  // [1]  77 <= adc <  87 : 800  Lux
	{	900	,	2	,	97	},  // [2]  87 <= adc <  97 : 900  Lux
	{	1000,	2	,	127	},  // [2]  97 <= adc       : 1000 Lux
};
*/
/* const */axes_t APDS9900_TABLE_PROXIMITY[2] = {
	{	2	,	0	,	(APDS9900_PS_INT_THRESHOLD>>APDS9900_PS_INT_THR_SHIFT)	},	// [0]     adc <= (APDS9900_PS_INT_THRESHOLD>>APDS9900_PS_INT_THR_SHIFT) : 2cm
	{	0	,	1	,	(APDS9900_PS_INT_THRESHOLD_LOWER>>APDS9900_PS_INT_THR_SHIFT)	},					// [1] (APDS9900_PS_INT_THRESHOLD>>APDS9900_PS_INT_THR_SHIFT) < adc <= (0xFFFF>>APDS9900_PS_INT_THR_SHIFT): 0cm
};

const axes_t APDS9900_TABLE_LIGHT[12] = { //ds2 team shs DS. add APDS9900_TABLE_LIGHT
	{	10	,	0	,	1	},	// [0]       adc <=   1 : 10   Lux
	{	50	,	0	,	3	},	// [0]   1 < adc <=   3 : 50   Lux
	{	100	,	0	,	7	},	// [0]   3 < adc <=   7 : 100  Lux
	{	200	,	1	,	16	},  // [1]   7 < adc <=  16 : 200  Lux
	{	300	,	1	,	26	},  // [1]  16 < adc <=  26 : 300  Lux
	{	400	,	1	,	36	},  // [1]  26 < adc <=  36 : 400  Lux
	{	500	,	1	,	46	},  // [1]  36 < adc <=  46 : 500  Lux
	{	600	,	1	,	57	},  // [1]  46 < adc <=  57 : 600  Lux
	{	700	,	1	,	67	},  // [1]  57 < adc <=  67 : 700  Lux
	{	800	,	1	,	77	},  // [1]  67 < adc <=  77 : 800  Lux
	{	900	,	2	,	87	},  // [2]  77 < adc <=  87 : 900  Lux
	{	1000,	2	,	98	},  // [2]  87 < adc        : 1000 Lux
};

/*
 * Global data
 */

static  apds9900data_t apds9900data;

static struct wake_lock wakelock; //ds2 team shs DS. global wakelock (suspend using)
static bool wakestatus; //ds2 team shs DS. wakestatus value

#ifdef FEATURE_PLM_PROBLEM_NO_32
int is_first_start_apds9900 = false;
#endif

/*
 * Management functions
 */
static int apds9900_i2c_read(u8 reg, u8 *buf, int count);
static int apds9900_i2c_write(u8 reg, u8 *data, int len);
static int apds9900_i2c_command(u8 cmd);
static int apds9900_set_power(status_e pwr);
static status_e apds9900_get_available(void);
static int apds9900_init_reg(void);
static int apds9900_set_reg(u8 reg, u8 data);
static int apds9900_get_reg(u8 reg, u8 *data);
static int apds9900_set_cmd(u8 cmd);
static int apds9900_init_data(struct i2c_client *client);
static int apds9900_get_psval(u8* psval);
static int apds9900_get_lsval(u8* lsval);
static int apds9900_get_tbl_index(active_e type, u8 adc);
static int apds9900_set_irq(status_e enable);
static irqreturn_t apds9900_irq_handler(int irq, void *dev_id);
static void apds9900_work_f(struct work_struct *work);
static inline void apds9900_init_suspend(void);
static inline void apds9900_deinit_suspend(void);
static inline void apds9900_prevent_suspend(void);
#ifdef SENSOR_APDS9900_ISR_VER4
static inline void apds9900_prevent_suspend_by_isr(void);
/* static inline */void apds9900_allow_suspend(void);
#else
static inline void apds9900_allow_suspend(void);
#endif
static void apds9900_clear_hysteresis(active_e type);
static void apds9900_push_hysteresis(active_e type, u8 adc);
static status_e apds9900_get_hysresult(active_e type, axes_t* axes);

#ifdef CONFIG_HAS_EARLYSUSPEND
static void apds9900_early_suspend(struct early_suspend *handler);
static void apds9900_late_resume(struct early_suspend *handler);
#endif /* CONFIG_HAS_EARLYSUSPEND */

/* -------------------------------------------------------------------- */
/* External Functions */
/* -------------------------------------------------------------------- */
#ifdef APDS9900_ACTIVE_NEW
int apds9900_control_enable_real(active_e sensor_type, int enable);
int apds9900_control_enable(active_e sensor_type, int enable)
{
	int update = 0;

	if(sensor_type == E_ACTIVE_PROXIMITY){
		printk(KERN_INFO
			       "apds9900 apds9900_control_enable sensor_type  %d enable=%d \n",sensor_type,enable);
	}
	else{
		printk(KERN_ERR
			       "apds9900 apds9900_control_enable sensor_type  Error %d \n",sensor_type);
		return -1;
	}

	apds9900_control_enable_real(sensor_type,enable);

	
	if ( enable ) {
		apds9900data.active_app |= sensor_type;
	}
	else {
#if 1
		apds9900data.active_app =0;
#else
		if(apds9900data.active_app & sensor_type){
			apds9900data.active_app &= (~sensor_type);
		}
#endif

	}
	
}
int apds9900_control_enable_real(active_e sensor_type, int enable)
#else
int apds9900_control_enable(active_e sensor_type, int enable)
#endif

{
	int update = 0;
	int ret = 0;

	dbg_func_in();
	dbg_data("sensor_type = %d / enable = %d / available = %d\n", sensor_type, enable, apds9900data.available);
	
#ifdef FEATURE_PLM_PROBLEM_NO_32
		if((sensor_type == E_ACTIVE_PROXIMITY) && !is_first_start_apds9900 && enable)
		{
			is_first_start_apds9900 = true;
		}
#endif

	if ( apds9900data.available == E_UNAVAILABLE ) {
		printk("%s : apds9900data is unavailable\n", __func__);
		return -1;
	}
	
#if 1  // incall must be check before power control.....
#else
	if ( (!enable) && (sensor_type == E_ACTIVE_PROXIMITY) && (apds9900data.incall) ) return 0;
#endif


	dbg_data("current_active = %d / new_active = %d / enable = %d\n", apds9900data.active, sensor_type, update);

	if ( enable ) {
		if(apds9900data.active == E_ACTIVE_NONE) {
			update = 1;
		}
		apds9900data.active |= sensor_type;
	}
	else {
		if(apds9900data.active & sensor_type){
			apds9900data.active &= (E_ACTIVE_ALL & (~sensor_type));
			if(apds9900data.active == E_ACTIVE_NONE) update = 1;
		}
	}

	dbg_data("update = %d\n", update);

	if(update) {
		if (enable) {
			if( apds9900_set_power(E_POWER_ON))
				ret = -1;
			else
				ret = apds9900_init_reg();
		}
		else {
			ret = apds9900_set_power(E_POWER_OFF);
		}
	}

	if (enable) apds9900_clear_hysteresis(sensor_type);
#ifdef SENSOR_APDS9900_DBG_DATA	
	dbg_data("apds9900data.power = %d\n",		apds9900data.power);
	dbg_data("apds9900data.enirq = %d\n",		apds9900data.enirq);
	dbg_data("apds9900data.available = %d\n",	apds9900data.available);
	dbg_data("apds9900data.active = %d\n",		apds9900data.active);	
	dbg_data("apds9900data.reg.enable = 0x%02X\n",	apds9900data.reg.enable);
	dbg_data("apds9900data.reg.atime = 0x%02X\n",	apds9900data.reg.atime);
	dbg_data("apds9900data.reg.ptime = 0x%02X\n", apds9900data.reg.ptime);
	dbg_data("apds9900data.reg.wtime = 0x%02X\n", apds9900data.reg.wtime);
	dbg_data("apds9900data.reg.ppcount = 0x%02X\n",	apds9900data.reg.ppcount);
	dbg_data("apds9900data.reg.control = 0x%02X\n", apds9900data.reg.control);
	dbg_data("apds9900data.reg.interrupt = 0x%02X\n", apds9900data.reg.interrupt);	
#endif
	dbg_func_out();

	return ret;
}

static int apds9900_ls_calculate(int cdata, int irdata)
{
	int coef = 100;
	
    int luxValue=0;

    long IAC1, IAC2, IAC;

    IAC1 = (cdata - (2*irdata)) * coef;
    IAC2 = (80*cdata) - (140*irdata); // when (coef==100)

    if (IAC1 > IAC2)
        IAC = (int)(IAC1/coef);
    else if (IAC1 <= IAC2)
        IAC = (int)(IAC2/coef);
    else
        IAC = 0;

    //luxValue = (IAC*0.46*52)/((2.72*(256-psensor->atime))*1);
    luxValue = (int) ((879 * (int)(IAC/(256-apds9900data.reg.atime)) ) / coef);
    

    return luxValue;
}
#ifdef SENSOR_APDS9900_ISR_VER4
extern int restart_ps_sensor_polling_flag;
#endif
int apds9900_ps_measure(axes_t *val)
{
#if 1
	int ret = 0;
	u8 adc = 0;
	int thr = 0;
	int i;
	bool hysresult = 0;
	axes_t ex;
	int retrycnt = 10;

	val->x = -1;
	val->y = -1;
	val->z = -1;

	// 1. check apds9900 is available.
	if(!apds9900data.available) return -1;

	while(retrycnt) {
		ret = apds9900_get_psval(&adc);
		if(ret) return ret;

		ex.z = (int)adc;

		// hysteresis
		thr = (apds9900data.hysps.lastaxes.x) ? APDS9900_TABLE_PROXIMITY[0].z : APDS9900_TABLE_PROXIMITY[1].z;
		ex.x = (adc>=thr) ? 0 : 2;
		ex.y = (ex.x) ? 3 : 1;
		if( (apds9900data.hysps.lastaxes.x == ex.x) && (adc < APDS9900_TABLE_PROXIMITY[0].z) && (adc >APDS9900_TABLE_PROXIMITY[1].z) ) ex.y = 2;

		dbg_data("EX.X=%d / EX.Y=%d / EX.Z=%d / apds9900data.hys.index=%d\n", ex.x, ex.y, ex.z, apds9900data.hysps.index);


		if(apds9900data.hysps.index<(HYSTERESIS_PS_BUF_SIZE-1)) apds9900data.hysps.buf[apds9900data.hysps.index] = ex.y;
		else {
			for(i=1; i<=apds9900data.hysps.index; i++) {
				apds9900data.hysps.buf[i-1] = apds9900data.hysps.buf[i];
			}
			apds9900data.hysps.buf[apds9900data.hysps.index] = ex.y;
		}

		#ifdef SENSOR_APDS9900_DBG_DATA
		dbg_data("---------------------------------------------\n");
		for(i=0; i<=apds9900data.hysps.index; i++) {
			dbg_data("buf[%d] = %d\n", i, apds9900data.hysps.buf[i]);
		}
		dbg_data("---------------------------------------------\n");
		#endif


		apds9900data.hysps.buf[apds9900data.hysps.index] = ex.y;

		if(apds9900data.hysps.buf[0] != 2) {
			for(i=1; i<=apds9900data.hysps.index; i++) {
				if(apds9900data.hysps.buf[0] != apds9900data.hysps.buf[i]) break;
			}
			if(i==(apds9900data.hysps.index+1)) {
				hysresult = 1;
				//if(retrycnt) printk("retrycnt=%d\n", retrycnt);
				retrycnt = 0;
				break;
			}
		}

		if(apds9900data.hysps.index < (HYSTERESIS_PS_BUF_SIZE-1)) apds9900data.hysps.index++;

		retrycnt--;
		mdelay(10);
		
	}

	dbg_data("retrycnt=%d\n", retrycnt);

	val->x = (hysresult) ? ex.x : apds9900data.hysps.lastaxes.x;
	val->y = ex.y;
	val->z = ex.z;

	apds9900data.hysps.lastaxes.x = val->x;
	apds9900data.hysps.lastaxes.y = val->y;
	apds9900data.hysps.lastaxes.z = val->z;


#ifdef SENSOR_APDS9900_ISR_VER4
	if(restart_ps_sensor_polling_flag)
		dbg("apds9900_ps_measure : (%d,%d,%d)\n", val->x, val->y, val->z);
	else
#endif
		dbg_data("apds9900_ps_measure : (%d,%d,%d)\n", val->x, val->y, val->z);

	return ret;
#else
	u8 psval = 0;
	int ret = 0;
	int retry = 10;
	
	//dbg_func_in();
	
	do {
		ret = apds9900_get_psval(&psval);
		if (ret) {
			val->x = 255;
			val->y = 255;
			val->z = 255;
			return ret;
		}

		apds9900_push_hysteresis(E_ACTIVE_PROXIMITY, psval);
		if (apds9900_get_hysresult(E_ACTIVE_PROXIMITY, val) == E_HYSTERESIS_ON) {
			msleep(5); // 5ms sleep
			retry--;
		} else {
			retry = 0;
		}
	} while(retry);


	dbg_data("apds9900_ps_measure : (%d,%d,%d)\n", val->x, val->y, val->z);

	//dbg_func_out();

	return 0;
#endif
}

int apds9900_ls_measure(axes_t *val)
{
	u8 lsval = 0;
	int ret = 0;
	int retry = 10;
	
	//dbg_func_in();
	
	do {
		ret = apds9900_get_lsval(&lsval);
		if (ret) return ret;

		apds9900_push_hysteresis(E_ACTIVE_LIGHT, lsval);
		if (apds9900_get_hysresult(E_ACTIVE_LIGHT, val) == E_HYSTERESIS_ON) {
			msleep(5); // 10ms sleep
			retry--;
		} else {
			retry = 0;
		}
	} while(retry);

	dbg_data("apds9900_ls_measure : (%d,%d,%d)\n", val->x, val->y, val->z);

	//dbg_func_out();

	return ret;
}

/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/* Internal Functions */
/* -------------------------------------------------------------------- */

/* ------------- System Attribute ---------------*/
static ssize_t apds9900_show_incall(struct device *dev, struct device_attribute *attr, char *buf)
{
	char incall = 0;

	mutex_lock(&apds9900data.incalllock); //ds2 team shs calling stae read from application
	incall = apds9900data.incall;
	mutex_unlock(&apds9900data.incalllock);

	dbg_data("%s : incall = %d\n", __func__, incall);

    return sprintf(buf, "%d\n", incall);
}

static ssize_t apds9900_store_incall(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    u8 incall = (u8)simple_strtoul(buf, NULL, 10);

	dbg_data("%s : incall = %d\n", __func__, incall);
	
	mutex_lock(&apds9900data.incalllock); //ds2 team shs calling state save to device
	apds9900data.incall = incall ? E_INCALL_ON : E_INCALL_OFF;
	mutex_unlock(&apds9900data.incalllock);
	
    return count;
}

/* CTS - FAIL */
#ifdef FEATURE_SKY_CTS_TEST_2P3_R2
static DEVICE_ATTR(incall, S_IWUG | S_IRUG, apds9900_show_incall, apds9900_store_incall);
#else
static DEVICE_ATTR(incall, S_IWUGO | S_IRUGO, apds9900_show_incall, apds9900_store_incall);
#endif

#ifdef APDS9900_THRESHOLD_TUNNING
static ssize_t apds9900_show_threshold(struct device *dev, struct device_attribute *attr, char *buf)
{
	u16  threshold_value = 0;

	mutex_lock(&apds9900data.incalllock); //ds2 team shs calling stae read from application
	threshold_value = apds9900data.threshold_value;
	mutex_unlock(&apds9900data.incalllock);

	dbg_data("%s : incall = %d\n", __func__, threshold_value);

    return sprintf(buf, "%d\n", threshold_value);
}

static ssize_t apds9900_store_threshold(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    u16 threshold_value = (u16)simple_strtoul(buf, NULL, 10);

	dbg_data("%s : threshold_value = %d\n", __func__, threshold_value);
	
	mutex_lock(&apds9900data.incalllock); //ds2 team shs calling state save to device
#ifdef APDS9900_THRESHOLD_TUNNING
	apds9900data.threshold_value  = threshold_value; 
       APDS9900_TABLE_PROXIMITY[0].z  = (apds9900data.threshold_value>>APDS9900_PS_INT_THR_SHIFT);
#endif
	mutex_unlock(&apds9900data.incalllock);
	
    return count;
}

/* CTS - FAIL */
#ifdef FEATURE_SKY_CTS_TEST_2P3_R2
static DEVICE_ATTR(threshold, S_IWUG | S_IRUG, apds9900_show_threshold, apds9900_store_threshold);
#else
static DEVICE_ATTR(threshold, S_IWUGO | S_IRUGO, apds9900_show_threshold, apds9900_store_threshold);
#endif

static ssize_t apds9900_show_threshold_lower(struct device *dev, struct device_attribute *attr, char *buf)
{
	u16  threshold_value = 0;

	mutex_lock(&apds9900data.incalllock); //ds2 team shs calling stae read from application
	threshold_value = apds9900data.threshold_value_lower;
	mutex_unlock(&apds9900data.incalllock);

	dbg_data("%s : incall = %d\n", __func__, threshold_value);

    return sprintf(buf, "%d\n", threshold_value);
}

static ssize_t apds9900_store_threshold_lower(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    u16 threshold_value = (u16)simple_strtoul(buf, NULL, 10);

	if(threshold_value == 0)
		threshold_value = APDS9900_PS_INT_THRESHOLD_LOWER;
		
	dbg_data("%s : threshold_value = %d\n", __func__, threshold_value);
	
	mutex_lock(&apds9900data.incalllock); //ds2 team shs calling state save to device
#ifdef APDS9900_THRESHOLD_TUNNING
	apds9900data.threshold_value_lower  = threshold_value; 
       APDS9900_TABLE_PROXIMITY[1].z  = (apds9900data.threshold_value_lower>>APDS9900_PS_INT_THR_SHIFT);
#endif
	mutex_unlock(&apds9900data.incalllock);
	
    return count;
}

/* CTS - FAIL */
#ifdef FEATURE_SKY_CTS_TEST_2P3_R2
static DEVICE_ATTR(threshold_lower, S_IWUG | S_IRUG, apds9900_show_threshold_lower, apds9900_store_threshold_lower);
#else
static DEVICE_ATTR(threshold_lower, S_IWUGO | S_IRUGO, apds9900_show_threshold_lower, apds9900_store_threshold_lower);
#endif

#endif
static struct attribute *apds9900_attrs[] = {
	&dev_attr_incall.attr,
#ifdef APDS9900_THRESHOLD_TUNNING
	&dev_attr_threshold.attr,
	&dev_attr_threshold_lower.attr,	
#endif
	NULL,
};

static struct attribute_group apds9900_attr_group = {
	.attrs = apds9900_attrs,
};
/* -------------------------------------------------------------------- */

/* ------------- I2C Interface ---------------*/
static int apds9900_i2c_read(u8 reg, u8 *buf, int count)
{
	int rc1, rc2;
	int ret = 0; 

	//dbg_func_in();
	
	if ( apds9900data.client == NULL ) {
		printk("%s : apds9900data.client is NULL\n", __func__);
		return -ENODEV;
	}

	buf[0] = reg;
	
	//client->addr = APDS9900_SLAVE_ADDR;

	mutex_lock(&apds9900data.i2clock); 
	rc1 = i2c_master_send(apds9900data.client,  buf, 1);

	rc2 = i2c_master_recv(apds9900data.client, buf, count);
	mutex_unlock(&apds9900data.i2clock);

	if ( (rc1 != 1) || (rc2 != count ) ) {
		dev_err(&apds9900data.client->dev, "apds9900_i2c_read FAILED: read of register %d\n", reg);
		ret = -1;
	}

	//dbg_func_out();

	return ret;
}

static int apds9900_i2c_write(u8 reg, u8 *data, int len)
{
	u8  buf[20];
	int rc;
	int ret = 0;
	int i; 
	//dbg_func_in();
	
	if ( apds9900data.client == NULL ) {
		printk("%s : apds9900data.client is NULL\n", __func__);
		return -ENODEV;
	}
	
	buf[0] = reg;
	if (len >= 20) {
		dev_err(&apds9900data.client->dev, "apds9900_i2c_write FAILED: buffer size is limitted(20)\n");
		return -1;
	}
	for( i=0 ; i<len; i++ ) buf[i+1] = data[i];

	//client->addr = APDS9900_SLAVE_ADDR;

	mutex_lock(&apds9900data.i2clock);  
	rc = i2c_master_send(apds9900data.client, buf, len+1);
	mutex_unlock(&apds9900data.i2clock);
	if (rc != len+1) {
		dev_err(&apds9900data.client->dev, "apds9900_i2c_write FAILED: writing to reg %d\n", reg);
		ret = -1;
	}

	//dbg_func_out();
	
	return ret;
}

static int apds9900_i2c_command(u8 cmd)
{
	u8            buf[2];
	int           rc;
	int           ret = 0;

	dbg_func_in();
	
	if ( apds9900data.client == NULL ) {
		printk("%s : apds9900data.client is NULL\n", __func__);
		return -ENODEV;
	}
	
	buf[0] = cmd;
	//client->addr = APDS9900_SLAVE_ADDR;

	mutex_lock(&apds9900data.i2clock);
	rc = i2c_master_send(apds9900data.client, buf, 1);
	mutex_unlock(&apds9900data.i2clock);
	if (rc != 1) {
		dev_err(&apds9900data.client->dev, "apds9900_i2c_command FAILED: writing to command %d\n", cmd);
		ret = -1;
	}

	dbg_func_out();
	return ret;
}

/* -------------------------------------------------------------------- */

/* ------------- Calculate Measured Data ---------------*/
static int apds9900_get_tbl_index(active_e type, u8 adc)
{
	axes_t *tbl;
	int tbl_size=0;
	int i;
	int index = 0;

	// dbg_func_in();

	if ( type == E_ACTIVE_PROXIMITY ) {
		tbl = (axes_t *)APDS9900_TABLE_PROXIMITY;
		tbl_size = sizeof(APDS9900_TABLE_PROXIMITY)/sizeof(axes_t);
	}
	else if ( type == E_ACTIVE_LIGHT ) {
		tbl = (axes_t *)APDS9900_TABLE_LIGHT;
		tbl_size = sizeof(APDS9900_TABLE_LIGHT)/sizeof(axes_t);
	}

	if(tbl_size < 2) index = 0;
	else {
		for ( i=0; i<tbl_size; i++ ) {
			if (i == 0) {
				if ( (adc >= 0) && (adc <= tbl[i].z) ) break;
			}
			else {
				if ( (adc > tbl[i-1].z) && (adc <= tbl[i].z) ) break;
			}
		}
		if ( i<tbl_size ) index = i;
		else index = 0;
	}

	// dbg_func_out();

//	printk("apds9900_get_tbl_index tbl_size => [%d] @@@ index [ %d] \n", tbl_size, index);

	return index;
}

static int apds9900_get_psval(u8* psval)
{
	int ret = 0;
	u8 ph, pl;
	u16 pdata = 0;

	dbg_func_in();

	*psval = 0;
	ret = apds9900_get_reg(APDS9900_PDATAL_REG, &pl);
	if(ret) return ret;
	ret = apds9900_get_reg(APDS9900_PDATAH_REG, &ph);
	if(ret) return ret;

	pdata = (ph << 8) | pl;
	dbg_data("(((((((((((1)))))))))))ph=0x%02X | pl=0x%02X = 0x%02X\n", ph, pl, pdata);

	*psval = (u8)((pdata >> APDS9900_PS_INT_THR_SHIFT) & 0xFF);
	dbg_data("(((((((((((2)))))))))))ph=0x%02X | pl=0x%02X = [DS 2 TEAM] =>0x%02X\n", ph, pl, *psval);

	dbg_func_out();

	return ret;
}

static int apds9900_get_lsval(u8* lsval)
{
	int ret=0;
	int cdata, irdata;
	u8 adata_ls[4];

	dbg_func_in();

	ret=apds9900_get_reg(APDS9900_CDATAL_REG,&adata_ls[0]);
	ret=apds9900_get_reg(APDS9900_CDATAH_REG,&adata_ls[1]);
	ret=apds9900_get_reg(APDS9900_IRDATAL_REG,&adata_ls[2]);	
	ret=apds9900_get_reg(APDS9900_IRDATAH_REG,&adata_ls[3]);

	cdata=adata_ls[0] | (adata_ls[1]<<sizeof(unsigned char));
	irdata=adata_ls[2] | (adata_ls[3]<<sizeof(unsigned char));
#ifdef SENSOR_APDS9900_DBG_DATA
	dbg_data("--- cdata=%d / irdata=%d\n", cdata, irdata);
	dbg_data("apds9900data.power = %d\n",		apds9900data.power);
	dbg_data("apds9900data.enirq = %d\n",		apds9900data.enirq);
	dbg_data("apds9900data.available = %d\n",	apds9900data.available);
	dbg_data("apds9900data.active = %d\n",		apds9900data.active);	
	dbg_data("apds9900data.reg.enable = 0x%02X\n",	apds9900data.reg.enable);
	dbg_data("apds9900data.reg.atime = 0x%02X\n",	apds9900data.reg.atime);
	dbg_data("apds9900data.reg.ptime = 0x%02X\n", apds9900data.reg.ptime);
	dbg_data("apds9900data.reg.wtime = 0x%02X\n", apds9900data.reg.wtime);
	dbg_data("apds9900data.reg.ppcount = 0x%02X\n",	apds9900data.reg.ppcount);
	dbg_data("apds9900data.reg.control = 0x%02X\n", apds9900data.reg.control);
	dbg_data("apds9900data.reg.interrupt = 0x%02X\n", apds9900data.reg.interrupt);	
#endif
	*lsval=(u8) apds9900_ls_calculate(cdata, irdata);

	dbg_func_out();
	
	return ret;
}

/* -------------------------------------------------------------------- */


/* ------------- Interrupt and Handler ---------------*/
static DECLARE_WORK(apds9900_work, apds9900_work_f);
#if defined SENSOR_APDS9900_ISR_VER3
static int apds9900_irq_init(void)
{
	int ret = 0;

//	printk("%s : first_access =%d enable=%d\n", __func__, enable);
	disable_irq(APDS9900_PS_IRQ);

	// 1. config gpio to input
//	ret = gpio_direction_input(APDS9900_PS_INT_N);

	gpio_tlmm_config(GPIO_CFG(APDS9900_PS_INT_N, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),GPIO_CFG_ENABLE);
	gpio_request(APDS9900_PS_INT_N, "apds9900_ps_irq_pin");

	
//	if (ret) {
//		printk("%s : FAILED: gpio_direction_input(%d) ret=%d\n", __func__, APDS9900_PS_INT_N, ret);
//		return ret;
//	}

	// 2. request irq
	ret = request_irq( APDS9900_PS_IRQ,
			&apds9900_irq_handler,
			IRQF_TRIGGER_FALLING,
			"apds9900_ps_irq",
			0 );
	if (!ret)
		dbg("apds9900_set_irq: Start proximity Success\n");
	else
	{
		printk("%s : FAILED: request_irq ret=[  %d  ]\n", __func__, ret);
		return ret;
	}
	
	// 3. register irq wake
	ret = set_irq_wake(APDS9900_PS_IRQ, 1);
	if (ret) {
		printk("%s : FAILED: set_irq_wake ret=%d\n", __func__, ret);
		return ret;
	}

	disable_irq(APDS9900_PS_IRQ);
	
	return 0;

}
static int apds9900_set_irq(status_e enable)
{
	static int first_access=1;
	static status_e last_state =E_DISABLE;
	int ret = 0;

	if(apds9900data.power == E_POWER_OFF){
		printk("%s : apds9900data.power  =%d Error  \n", __func__, apds9900data.power);
		return -1;
	}
	else{
		// Disable APDS Int.
		ret = apds9900_set_reg(APDS9900_ENABLE_REG, 0x0F);
			if (ret<0) {
			printk("%s : FAILED: APDS9900_ENABLE_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}

		// Clear APDS Int.
	       ret = apds9900_set_cmd(CMD_CLR_PS_INT);
		if (ret) {
			printk("apds9900_set_irq FAILED: apds9900_i2c_command ret=%d\n", ret);
			return ret;
		}
	}


	if( first_access && enable){
		
		
		first_access = 0;

	}
	else if (first_access){
		// Nothing to do 		
		printk("%s : first_access =%d and disable ... Nothing to do \n", __func__, first_access);
	}

	if(last_state == enable){
		// Nothing to do 		
		printk("%s : last_state =%d enable =%d \n", __func__, last_state,enable);
	}

		printk("%s : apds9900data.hysps.lastaxes.x =%d \n", __func__, apds9900data.hysps.lastaxes.x);


	if(enable){

#ifdef SENSOR_APDS9900_ISR_VER4
		if(apds9900data.hysps.lastaxes.x == 2){
			
			ret = apds9900_set_reg(APDS9900_PILTL_REG, 0);
			if (ret<0) {
				printk("%s : FAILED: APDS9900_PIHTL_REG set_reg ret=%d\n", __func__, ret);
				return ret;
			}

			//[DS2 TEAM shs] : proximity irq high  threshold value setting (high field)
			ret = apds9900_set_reg(APDS9900_PILTH_REG, 0);
			if (ret<0) {
				printk("%s : FAILED: APDS9900_PIHTL_REG set_reg ret=%d\n", __func__, ret);
				return ret;
			}


			//[DS2 TEAM shs] : proximity irq low threshold value setting (low field)
			ret = apds9900_set_reg(APDS9900_PIHTL_REG, (apds9900data.threshold_value&0xFF));
			if (ret<0) {
				printk("%s : FAILED: APDS9900_PILTL_REG set_reg ret=%d\n", __func__, ret);
				return ret;
			}		
			//[DS2 TEAM shs] : proximity irq low threshold value setting  (high field)
			ret = apds9900_set_reg(APDS9900_PIHTH_REG, ((apds9900data.threshold_value>>8)&0xFF));
			if (ret<0) {
				printk("%s : FAILED: APDS9900_PILTL_REG set_reg ret=%d\n", __func__, ret);
				return ret;
			}		

		}
		else
#endif			
		{

		
#ifdef APDS9900_THRESHOLD_TUNNING
		ret = apds9900_set_reg(APDS9900_PILTL_REG, (apds9900data.threshold_value&0xFF));
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PIHTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}

		//[DS2 TEAM shs] : proximity irq high  threshold value setting (high field)
		ret = apds9900_set_reg(APDS9900_PILTH_REG, ((apds9900data.threshold_value>>8)&0xFF));
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PIHTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}
#else	   
		ret = apds9900_set_reg(APDS9900_PILTL_REG, (APDS9900_PS_INT_THRESHOLD&0xFF));
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PIHTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}

		//[DS2 TEAM shs] : proximity irq high  threshold value setting (high field)
		ret = apds9900_set_reg(APDS9900_PILTH_REG, ((APDS9900_PS_INT_THRESHOLD>>8)&0xFF));
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PIHTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}
#endif


			//[DS2 TEAM shs] : proximity irq low threshold value setting (low field)
			ret = apds9900_set_reg(APDS9900_PIHTL_REG, 0xFF);
			if (ret<0) {
				printk("%s : FAILED: APDS9900_PILTL_REG set_reg ret=%d\n", __func__, ret);
				return ret;
			}		
			//[DS2 TEAM shs] : proximity irq low threshold value setting  (high field)
			ret = apds9900_set_reg(APDS9900_PIHTH_REG, 0xFF);
			if (ret<0) {
				printk("%s : FAILED: APDS9900_PILTL_REG set_reg ret=%d\n", __func__, ret);
				return ret;
			}		
		}


              //[DS2 TEAM shs] : proximity irq set persistence (4-4)		
		ret = apds9900_set_reg(APDS9900_PERS_REG, 0x33);
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PERS_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}              

		//[DS2 TEAM shs] : clear interrupt register
		ret = apds9900_set_cmd(CMD_CLR_PS_INT);
		if (ret<0) {
			printk("%s : FAILED: CMD_CLR_PS_INT set_reg ret=%d\n", __func__, ret);
			return ret;
		}

		//[DS2 TEAM shs] :  enable proximity interrupt.
		ret = apds9900_set_reg(APDS9900_ENABLE_REG, 0x2F);
 		if (ret<0) {
			printk("%s : FAILED: APDS9900_ENABLE_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}

		enable_irq(APDS9900_PS_IRQ);
		
		printk("%s :  IRQ ENABLE \n",__func__);
		
	}
	else {
		
		disable_irq(APDS9900_PS_IRQ);
		
		apds9900_allow_suspend();

		printk("%s :  IRQ DISABLE \n",__func__);
	}

	last_state = apds9900data.enirq = enable;

	
}
#elif defined SENSOR_APDS9900_ISR_VER2	
static int apds9900_set_irq(status_e enable)
{
	static int first_access=1;
	static status_e last_state =E_DISABLE;
	int ret = 0;

	if(apds9900data.power == E_POWER_OFF){
		printk("%s : apds9900data.power  =%d Error  \n", __func__, apds9900data.power);
		return -1;
	}
	else{
		// Disable APDS Int.
		ret = apds9900_set_reg(APDS9900_ENABLE_REG, 0x0F);
			if (ret<0) {
			printk("%s : FAILED: APDS9900_ENABLE_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}

		// Clear APDS Int.
	       ret = apds9900_set_cmd(CMD_CLR_PS_INT);
		if (ret) {
			printk("apds9900_set_irq FAILED: apds9900_i2c_command ret=%d\n", ret);
			return ret;
		}
	}


	if( first_access && enable){
		
		printk("%s : first_access =%d enable=%d\n", __func__, first_access, enable);

		// 1. config gpio to input
		ret = gpio_direction_input(APDS9900_PS_INT_N);
		if (ret) {
			printk("%s : FAILED: gpio_direction_input(%d) ret=%d\n", __func__, APDS9900_PS_INT_N, ret);
			return ret;
		}

		// 2. request irq
		ret = request_irq( APDS9900_PS_IRQ,
				&apds9900_irq_handler,
				IRQF_TRIGGER_FALLING,
				"apds9900_ps_irq",
				0 );
		if (!ret)
			dbg("apds9900_set_irq: Start proximity Success\n");
		else
		{
			printk("%s : FAILED: request_irq ret=[  %d  ]\n", __func__, ret);
			return ret;
		}
		
		// 3. register irq wake
		ret = set_irq_wake(APDS9900_PS_IRQ, 1);
		if (ret) {
			printk("%s : FAILED: set_irq_wake ret=%d\n", __func__, ret);
			return ret;
		}
		
		first_access = 0;

	}
	else if (first_access){
		// Nothing to do 		
		printk("%s : first_access =%d and disable ... Nothing to do \n", __func__, first_access);
	}

	if(last_state == enable){
		// Nothing to do 		
		printk("%s : last_state =%d enable =%d \n", __func__, last_state,enable);
	}



	if(enable){

		
#ifdef APDS9900_THRESHOLD_TUNNING
		ret = apds9900_set_reg(APDS9900_PILTL_REG, (apds9900data.threshold_value&0xFF));
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PIHTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}

		//[DS2 TEAM shs] : proximity irq high  threshold value setting (high field)
		ret = apds9900_set_reg(APDS9900_PILTH_REG, ((apds9900data.threshold_value>>8)&0xFF));
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PIHTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}
#else	   
		ret = apds9900_set_reg(APDS9900_PILTL_REG, (APDS9900_PS_INT_THRESHOLD&0xFF));
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PIHTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}

		//[DS2 TEAM shs] : proximity irq high  threshold value setting (high field)
		ret = apds9900_set_reg(APDS9900_PILTH_REG, ((APDS9900_PS_INT_THRESHOLD>>8)&0xFF));
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PIHTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}
#endif

#ifdef APDS9900_THRESHOLD_TUNNING
		//[DS2 TEAM shs] : proximity irq low threshold value setting (low field)
		ret = apds9900_set_reg(APDS9900_PIHTL_REG, (apds9900data.threshold_value_lower&0xFF));
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PILTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}		
		//[DS2 TEAM shs] : proximity irq low threshold value setting  (high field)
		ret = apds9900_set_reg(APDS9900_PIHTH_REG, ((apds9900data.threshold_value_lower>>8)&0xFF));
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PILTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}		
#else
		//[DS2 TEAM shs] : proximity irq low threshold value setting (low field)
		ret = apds9900_set_reg(APDS9900_PIHTL_REG, 0xFF);
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PILTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}		
		//[DS2 TEAM shs] : proximity irq low threshold value setting  (high field)
		ret = apds9900_set_reg(APDS9900_PIHTH_REG, 0xFF);
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PILTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}		
#endif
              //[DS2 TEAM shs] : proximity irq set persistence (4-4)		
		ret = apds9900_set_reg(APDS9900_PERS_REG, 0x33);
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PERS_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}              

		//[DS2 TEAM shs] : clear interrupt register
		ret = apds9900_set_cmd(CMD_CLR_PS_INT);
		if (ret<0) {
			printk("%s : FAILED: CMD_CLR_PS_INT set_reg ret=%d\n", __func__, ret);
			return ret;
		}

		//[DS2 TEAM shs] :  enable proximity interrupt.
		ret = apds9900_set_reg(APDS9900_ENABLE_REG, 0x2F);
 		if (ret<0) {
			printk("%s : FAILED: APDS9900_ENABLE_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}

		enable_irq(APDS9900_PS_IRQ);
		
		printk("%s :  IRQ ENABLE \n",__func__);
		
	}
	else {
		
		disable_irq(APDS9900_PS_IRQ);
		
		apds9900_allow_suspend();

		printk("%s :  IRQ DISABLE \n",__func__);
	}

	last_state = apds9900data.enirq = enable;

	
}
#else
static int apds9900_set_irq(status_e enable)
{
	int ret = 0;

	dbg_func_in();

	dbg_data("[DS2 TEAM SHS]  : apds9900_set_irq : enable = %d / apds9900data.enirq = %d\n", enable, apds9900data.enirq);

	// Disable APDS Int.
	ret = apds9900_set_reg(APDS9900_ENABLE_REG, 0x0F);
		if (ret<0) {
		printk("%s : FAILED: APDS9900_ENABLE_REG set_reg ret=%d\n", __func__, ret);
		return ret;
	}

	// Clear APDS Int.
       ret = apds9900_set_cmd(CMD_CLR_PS_INT);
	if (ret) {
		printk("apds9900_set_irq FAILED: apds9900_i2c_command ret=%d\n", ret);
		return ret;
	}

	if ( enable ) {

		// 1. config gpio to input
		ret = gpio_direction_input(APDS9900_PS_INT_N);
		if (ret) {
			printk("%s : FAILED: gpio_direction_input(%d) ret=%d\n", __func__, APDS9900_PS_INT_N, ret);
			return ret;
		}

		// 2. request irq
		ret = request_irq( APDS9900_PS_IRQ,
				&apds9900_irq_handler,
				IRQF_TRIGGER_FALLING,
				"apds9900_ps_irq",
				0 );
		if (!ret)
			dbg("apds9900_set_irq: Start proximity Success\n");
		else
		{
			printk("%s : FAILED: request_irq ret=[  %d  ]\n", __func__, ret);
			return ret;
		}
		
		// 3. register irq wake
		ret = set_irq_wake(APDS9900_PS_IRQ, 1);
		if (ret) {
			printk("%s : FAILED: set_irq_wake ret=%d\n", __func__, ret);
			return ret;
		}
		
		/*********************************apds9900 setting irq enable********************/
		//[DS2 TEAM shs] : proximity irq high threshold value setting (low field)

#ifdef APDS9900_THRESHOLD_TUNNING
		ret = apds9900_set_reg(APDS9900_PILTL_REG, (apds9900data.threshold_value&0xFF));
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PIHTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}

		//[DS2 TEAM shs] : proximity irq high  threshold value setting (high field)
		ret = apds9900_set_reg(APDS9900_PILTH_REG, ((apds9900data.threshold_value>>8)&0xFF));
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PIHTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}
#else	   
		ret = apds9900_set_reg(APDS9900_PILTL_REG, (APDS9900_PS_INT_THRESHOLD&0xFF));
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PIHTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}

		//[DS2 TEAM shs] : proximity irq high  threshold value setting (high field)
		ret = apds9900_set_reg(APDS9900_PILTH_REG, ((APDS9900_PS_INT_THRESHOLD>>8)&0xFF));
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PIHTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}
#endif


		//[DS2 TEAM shs] : proximity irq low threshold value setting (low field)
		ret = apds9900_set_reg(APDS9900_PIHTL_REG, 0xFF);
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PILTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}		
		//[DS2 TEAM shs] : proximity irq low threshold value setting  (high field)
		ret = apds9900_set_reg(APDS9900_PIHTH_REG, 0xFF);
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PILTL_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}		

              //[DS2 TEAM shs] : proximity irq set persistence (4-4)		
		ret = apds9900_set_reg(APDS9900_PERS_REG, 0x33);
		if (ret<0) {
			printk("%s : FAILED: APDS9900_PERS_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}              

		//[DS2 TEAM shs] : clear interrupt register
		ret = apds9900_set_cmd(CMD_CLR_PS_INT);
		if (ret<0) {
			printk("%s : FAILED: CMD_CLR_PS_INT set_reg ret=%d\n", __func__, ret);
			return ret;
		}

		//[DS2 TEAM shs] :  enable proximity interrupt.
		ret = apds9900_set_reg(APDS9900_ENABLE_REG, 0x2F);
 		if (ret<0) {
			printk("%s : FAILED: APDS9900_ENABLE_REG set_reg ret=%d\n", __func__, ret);
			return ret;
		}
		dbg_data("IRQ ENABLE \n");

	}
	else {

		// 1. clear apds9900's interrupt enable bit

		// 2. free irq
		free_irq( APDS9900_PS_IRQ, NULL );

		// 3. allow suspend
		apds9900_allow_suspend();

		dbg_data("IRQ DISABLE \n");		
	}

	apds9900data.enirq = enable;

	dbg_func_out();

	return 0;
}
#endif



static int apds9900_irq_handler_called_counter=0;
static irqreturn_t apds9900_irq_handler(int irq, void *dev_id)
{
	dbg_func_in();
	dbg_data("apds9900_irq_handler calling apds9900_irq_handler_called_counter=%d\n",apds9900_irq_handler_called_counter++);

	schedule_work(&apds9900_work);

	dbg_func_out();

	return IRQ_HANDLED;
}

static int apds9900_work_f_called_counter=0;
#ifdef SENSOR_APDS9900_ISR_VER4
extern void restart_ps_sensor_polling(void);
#endif

static void apds9900_work_f(struct work_struct *work)
{
	u8 psint;
	int ret;

	dbg_func_in();

	dbg_data("%s : apds9900data.active = %d\n", __func__, apds9900data.active);

	if ( ! ( apds9900data.active & E_ACTIVE_PROXIMITY ) ) {
		printk("%s : apds9900data.active is not E_ACTIVE_PROXIMITY\n", __func__);
		goto apds9900_work_f_exit;
	}

	ret = apds9900_get_reg(APDS9900_STATUS_REG, &psint);
	if (ret) psint = 0;
#ifdef SENSOR_APDS9900_ISR_VER4
		apds9900_prevent_suspend_by_isr();
//		apds9900_prevent_suspend();
#else
	if (psint & (1<<6)) {
		apds9900_prevent_suspend();
	}
#endif
	//clear interrupt
	ret = apds9900_set_cmd(CMD_CLR_PS_INT);
	if (ret) {
		printk("apds9900_set_irq FAILED: apds9900_set_cmd ret=%d\n", ret);
	}

#ifdef SENSOR_APDS9900_ISR_VER4
	restart_ps_sensor_polling();
#endif

//	printk("[@@@@@]apds9900_work_f calling\n");
apds9900_work_f_exit:
	dbg_func_out();
}
/* -------------------------------------------------------------------- */


/* ------------- Power and Register ---------------*/
static int apds9900_set_power(status_e pwr)
{
	struct vreg *vr = vreg_get(NULL, APDS9900_VREG_STR);
	int rc = 0;	
	dbg_func_in();

	dbg_data("apds9900data.power = %d / pwr = %d\n", apds9900data.power, pwr);
	
	if (apds9900data.power == pwr ) return 0;

	if(pwr == E_POWER_ON) { //DS2 TEAM shs : power on sequence
		rc = vreg_set_level(vr, APDS9900_VREG_VOLT);
		if(!rc) rc = vreg_enable(vr);		
		apds9900data.power = E_POWER_ON;
		mdelay(10);
		dbg_data("POWER_ON\n");
	} else { //DS2 Team shs : power off sequence
		mdelay(10);
		rc = vreg_disable(vr);
		apds9900data.power = E_POWER_OFF;
		dbg_data("POWER_OFF\n");
	}

	dbg_func_out();
	return 0;
}

static status_e apds9900_get_available(void)
{
	u8 id=0;
	status_e available = E_UNAVAILABLE;
	int ret = 0;
	
	dbg_func_in();
	
	dbg_data("%s : apds9900data.power = %d\n", __func__, apds9900data.power);

	if ( apds9900data.power == E_POWER_OFF ) { //DS2 TEAM , SHS DS. power off state
		// 1. set power-on
		if ( apds9900_set_power(E_POWER_ON) ) return E_UNAVAILABLE;

		// 2. get chip-id and compare (== 0x01)
		ret = apds9900_i2c_read(APDS9900_ID_REG, &id, 1);
		printk("%s : ret = %d / id = 0x%02X\n", __func__, ret, id);
		//[DS 2team shs] : id value 0 or 4 (APDS 9900, APDS 99XX)
		/* shpark. The Id is variable. We can't define what is apds9900's ID. */
		//if( (ret == 0) &&( (id == 0) || (id==4))) available = E_AVAILABLE;
		if (!ret) available = E_AVAILABLE;
		else apds9900_set_power(E_POWER_OFF);

		// 3. set power-off
		if ( apds9900_set_power(E_POWER_OFF) ) return E_UNAVAILABLE;
	}
	else { //DS2 TEAM , SHS DS. power on state
		//[DS 2team shs] : id value 0 or 4 (APDS 9900, APDS 99XX)
		ret = apds9900_i2c_read(APDS9900_ID_REG, &id ,1 );
		if( (ret == 0) && ( (id == 0) || (id==4))) available = E_AVAILABLE;
	}

	dbg_func_out();

	return available;
}

static int apds9900_init_reg(void)
{
	int ret;


	dbg_func_in();


	// 1. config disable
	ret = apds9900_set_reg(APDS9900_ENABLE_REG, E_DISABLE);
	if(ret<0) return ret;
	// 2. config atime
	ret = apds9900_set_reg(APDS9900_ATIME_REG, 0xDB); // 100.64ms
	if(ret<0) return ret;
	// 3. config ptime
	ret = apds9900_set_reg(APDS9900_PTIME_REG, 0xFF); // 2.72ms
	if(ret<0) return ret;
	
	
	// 4. config wtime
	ret = apds9900_set_reg(APDS9900_WTIME_REG, 0xF6); // 27.2ms 
	if(ret<0) return ret;

	// 5. config ppcount
	ret = apds9900_set_reg(APDS9900_PPCOUNT_REG, 0x04); // 4pulse
	if(ret<0) return ret;

	
	// 6. config control
	ret = apds9900_set_reg(APDS9900_CONTROL_REG, 0x20); // 100mA, IR-diode, 1X PGain, 1X AGain
	if(ret<0) return ret;
	
	
	// 7. config enable

	ret = apds9900_set_reg(APDS9900_ENABLE_REG, 0x0F);
	if(ret<0) return ret;
	

	dbg_func_out();
	
	return 0;
}

static int apds9900_set_reg(u8  reg, u8 data)
{
	int ret = apds9900_i2c_write(reg, &data , 1);

	dbg_func_in();	
	if (!ret) {
		switch (reg) {
			case APDS9900_ENABLE_REG:
				apds9900data.reg.enable= data;
				break;
			case APDS9900_ATIME_REG:
				apds9900data.reg.atime= data;
				break;
			case APDS9900_PTIME_REG:
				apds9900data.reg.ptime= data;
				break;
			case APDS9900_WTIME_REG:
				apds9900data.reg.wtime= data;
				break;
			case APDS9900_PPCOUNT_REG:
				apds9900data.reg.ppcount= data;
				break;
			case APDS9900_CONTROL_REG:
				apds9900data.reg.control= data;
				break;
			case CMD_CLR_PS_INT:
				apds9900data.reg.interrupt= data;
				break;
			case APDS9900_PILTL_REG:
				apds9900data.reg.piltl=data;
				break;
			case APDS9900_PILTH_REG:
				break;
				
			case APDS9900_PIHTL_REG:
				apds9900data.reg.pihtl=data;
				break;
			case APDS9900_PIHTH_REG:
				break;
			case APDS9900_PERS_REG:
				apds9900data.reg.pers=data;
				break;
			default:
				ret = -1;
				break;
		}
	}

	dbg_func_out();

	return  ret;
}

static int apds9900_get_reg(u8 reg, u8 *data)
{
	int ret=0;
	
//	dbg_func_in();
	
	ret = apds9900_i2c_read(reg,data,1);

	

//	dbg_func_out();

	return  ret;
}

static int apds9900_set_cmd(u8 cmd)
{
	int ret;
	
	dbg_func_in();

	ret = apds9900_i2c_command(cmd);

	dbg_func_out();

	return  ret;
}

static int apds9900_init_data(struct i2c_client *client)
{
	int err = 0;
	int retrycnt = 3;
		
	dbg_func_in();

	// 1. assign i2c client
	apds9900data.client = client;
	i2c_set_clientdata(client, &apds9900data);

	// 2. initialize mutex
	mutex_init(&apds9900data.i2clock);
	mutex_init(&apds9900data.incalllock);

	// 3. initialize data
	apds9900data.power		= E_POWER_OFF; //DS2 TEAM , SHS DS. power off
	apds9900data.enirq		= E_DISABLE; //DS2 TEAM , SHS DS. irq off
	apds9900data.incall		= E_INCALL_OFF; //DS2 TEAM , SHS DS. calling state off
	apds9900data.active		= E_ACTIVE_NONE; //DS2 TEAM , SHS DS. active state off	
#ifdef APDS9900_ACTIVE_NEW
	apds9900data.active_app  = E_ACTIVE_NONE; //DS2 TEAM , SHS DS. active state off	
#endif

#ifdef APDS9900_THRESHOLD_TUNNING
	apds9900data.threshold_value  = APDS9900_PS_INT_THRESHOLD; 
       APDS9900_TABLE_PROXIMITY[0].z  = (apds9900data.threshold_value>>APDS9900_PS_INT_THR_SHIFT);
	apds9900data.threshold_value_lower = APDS9900_PS_INT_THRESHOLD_LOWER; 	   
       APDS9900_TABLE_PROXIMITY[1].z  = (apds9900data.threshold_value_lower>>APDS9900_PS_INT_THR_SHIFT);	
#endif
	
	// 4. check device available
	do {
	apds9900data.available = apds9900_get_available();
	if ( apds9900data.available == E_UNAVAILABLE ) {
		err = -ENODEV;
			retrycnt--;
		}
		else {
			retrycnt = 0;
	}
	} while(retrycnt);

	dbg_data("%s : apds9900data.available = %d\n", __func__, apds9900data.available);

	// 5. init wakelock
	apds9900_init_suspend();

	dbg_func_out();
	
	return err;
}
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/* HYSTERESIS Functions */
/* -------------------------------------------------------------------- */
static void apds9900_clear_hysteresis(active_e type)
{
	int i=0;

	//dbg_func_in();

	dbg_data("%s : type = %d\n", __func__, type);

	if (type & E_ACTIVE_PROXIMITY) {
		if (apds9900data.hysps.buf == NULL) {
			apds9900data.hysps.buf = kzalloc(sizeof(hysteresis_t)*HYSTERESIS_PS_BUF_SIZE, GFP_KERNEL);
		}
		for(i=0; i<HYSTERESIS_PS_BUF_SIZE; i++) {
			apds9900data.hysps.buf[i] = 0;
			apds9900data.hysps.index = 0;
		}
		apds9900data.hysps.lastaxes.x = 2;
		apds9900data.hysps.lastaxes.y = 0;
		apds9900data.hysps.lastaxes.z = 0;
	}
	if (type & E_ACTIVE_LIGHT) {
		if (apds9900data.hysls.buf == NULL) {
			apds9900data.hysls.buf = kzalloc(sizeof(hysteresis_t)*HYSTERESIS_LS_BUF_SIZE, GFP_KERNEL);
		}
		for(i=0; i<HYSTERESIS_LS_BUF_SIZE; i++) {
			apds9900data.hysls.buf[i] = 0;
			apds9900data.hysls.index = 0;
		}
		apds9900data.hysls.lastaxes.x = 0;
		apds9900data.hysls.lastaxes.y = 0;
		apds9900data.hysls.lastaxes.z = 0;
	}

	// dbg_func_out();
}

static void apds9900_push_hysteresis(active_e type, u8 adc)
{
	int i=0;


	if ( type == E_ACTIVE_PROXIMITY ) {
	dbg_data("type = %d / adc = %d\n", type, adc);		
		if ( (apds9900data.hysps.buf != NULL) && (apds9900data.hysps.index >= 0) ){
			if (apds9900data.hysps.index < HYSTERESIS_PS_BUF_SIZE - 1) {
				apds9900data.hysps.buf[apds9900data.hysps.index] = adc;
				apds9900data.hysps.index++;
			} else {
				for(i=1; i<HYSTERESIS_PS_BUF_SIZE; i++) {
					apds9900data.hysps.buf[i-1] = apds9900data.hysps.buf[i];
				}
				apds9900data.hysps.index = HYSTERESIS_PS_BUF_SIZE - 1;
				apds9900data.hysps.buf[apds9900data.hysps.index] = adc;
			}
			apds9900data.ps_adc=adc; //[DS2 TEAM] SHS : save ps adc last value			
	dbg_data("hysps.buf[%d] = %d\n", apds9900data.hysps.index, apds9900data.hysps.buf[apds9900data.hysps.index]);
		}
	} else if ( type == E_ACTIVE_LIGHT ) {
		if ( (apds9900data.hysls.buf != NULL) && (apds9900data.hysls.index >= 0) ){
			if (apds9900data.hysls.index < HYSTERESIS_LS_BUF_SIZE - 1) {
				apds9900data.hysls.buf[apds9900data.hysls.index] = adc;
				apds9900data.hysls.index++;
			} else {
				for(i=1; i<HYSTERESIS_LS_BUF_SIZE; i++) {
					apds9900data.hysls.buf[i-1] = apds9900data.hysls.buf[i];
				}
				apds9900data.hysls.index = HYSTERESIS_LS_BUF_SIZE - 1;
				apds9900data.hysls.buf[apds9900data.hysls.index] = adc;
			}
			apds9900data.ls_adc=adc; //[ES2 TEAM] SHS : save ls adc last value
//			dbg("hysls.buf[%d] = %d\n", apds9900data.hysls.index, apds9900data.hysls.buf[apds9900data.hysls.index]);
		}
	}
}

static status_e apds9900_get_hysresult(active_e type, axes_t* axes)
{
	int i;
	int rc = 0;
	int tblindex = 0;
	u8 compy=0;

	if ( (type == E_ACTIVE_PROXIMITY) && (apds9900data.hysps.buf != NULL) ) {
		for(i=0; i<=apds9900data.hysps.index; i++) {
			tblindex = apds9900_get_tbl_index(type, apds9900data.hysps.buf[i]);
			if(i==0) compy = APDS9900_TABLE_PROXIMITY[tblindex].y;
			else {
				if(APDS9900_TABLE_PROXIMITY[tblindex].y != compy) rc++;
			}
		dbg_data("tblindex : [%d] x = %d / y = %d / z = %d\n", tblindex, APDS9900_TABLE_PROXIMITY[tblindex].x, APDS9900_TABLE_PROXIMITY[tblindex].y, APDS9900_TABLE_PROXIMITY[tblindex].z);
		}
		if(!rc) {
			apds9900data.hysps.lastaxes.x = APDS9900_TABLE_PROXIMITY[tblindex].x;
			apds9900data.hysps.lastaxes.y = APDS9900_TABLE_PROXIMITY[tblindex].y;
			apds9900data.hysps.lastaxes.z = apds9900data.ps_adc;//[DS 2TEAM] SHS : return ps adc last value (z parameter)
		}
		memcpy(axes, &apds9900data.hysps.lastaxes, sizeof(axes_t));
		dbg_data("LAST_PS x = %d / y = %d / z = %d\n", axes->x, axes->y, axes->z); 
	} else if ( (type == E_ACTIVE_LIGHT) && (apds9900data.hysls.buf != NULL) ){
		for(i=0; i<=apds9900data.hysls.index; i++) {
			tblindex = apds9900_get_tbl_index(type, apds9900data.hysls.buf[i]);
			if(i==0) compy = APDS9900_TABLE_LIGHT[tblindex].y;
			else {
				if(APDS9900_TABLE_LIGHT[tblindex].y != compy) rc++;
			}
//			dbg("[%d] x = %d / y = %d / z = %d\n", tblindex, APDS9900_TABLE_LIGHT[tblindex].x, APDS9900_TABLE_LIGHT[tblindex].y, APDS9900_TABLE_LIGHT[tblindex].z);
		}
		if(!rc) {
			apds9900data.hysls.lastaxes.x = APDS9900_TABLE_LIGHT[tblindex].x;
			apds9900data.hysls.lastaxes.y = APDS9900_TABLE_LIGHT[tblindex].y;
			apds9900data.hysls.lastaxes.z = apds9900data.ls_adc;//[DS2 TEAM] SHS :return ls adc last value(z parameter)
		}
		memcpy(axes, &apds9900data.hysls.lastaxes, sizeof(axes_t));
//		dbg("LAST_LS x = %d / y = %d / z = %d\n", axes->x, axes->y, axes->z); 
	}

	return rc ? E_HYSTERESIS_ON : E_HYSTERESIS_NOT;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
/* WAKELOCK Functions */
/* -------------------------------------------------------------------- */
static inline void apds9900_init_suspend(void)
{
	wake_lock_init(&wakelock, WAKE_LOCK_SUSPEND, "apds9900_wakelock");
}
static inline void apds9900_deinit_suspend(void)
{
	wake_lock_destroy(&wakelock);
}
static inline void apds9900_prevent_suspend(void)
{
	dbg_data("-------------- apds9900_prevent_suspend wakestatus=%d\n",wakestatus);
	if(!wakestatus)
	{
//		dbg("-------------- apds9900_prevent_suspend \n");
		wake_lock(&wakelock);
		wakestatus = 1;
	}
}
#ifdef SENSOR_APDS9900_ISR_VER4
static inline void apds9900_prevent_suspend_by_isr(void)
{
	dbg_data("-------------- apds9900_prevent_suspend_by_isr wakestatus=%d\n",wakestatus);
	if(!wakestatus)
	{
//		dbg("-------------- apds9900_prevent_suspend \n");
		wake_lock_timeout(&wakelock, HZ/2.5);
		wakestatus = 1;
	}
}


/* static inline*/ void apds9900_allow_suspend(void)
{
	dbg_data("-------------- apds9900_allow_suspend  wakestatus=%d\n",wakestatus);
	if(wakestatus)
	{
//		dbg("-------------- apds9900_allow_suspend \n");
		wake_unlock(&wakelock);
		wakestatus = 0;
	}
}
#else
static inline void apds9900_allow_suspend(void)
{
	dbg_data("-------------- apds9900_allow_suspend  wakestatus=%d\n",wakestatus);
	if(wakestatus)
	{
//		dbg("-------------- apds9900_allow_suspend \n");
		wake_unlock(&wakelock);
		wakestatus = 0;
	}
}
#endif	
/* -------------------------------------------------------------------- */

/*
 * I2C init/probing/exit functions
 */

static struct i2c_driver apds9900_driver;
static int __devinit apds9900_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	int err = 0;
	
	dbg_func_in();

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE)) {
		err = -EIO;
		return err;
	}

	/* Initialize the APDS9900 chip */
	err = apds9900_init_data(client);

	/* register sysfs */
	if (!err)
	err = sysfs_create_group(&client->dev.kobj, &apds9900_attr_group);

#ifdef CONFIG_HAS_EARLYSUSPEND
	apds9900data.early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	apds9900data.early_suspend.suspend = apds9900_early_suspend;
	apds9900data.early_suspend.resume = apds9900_late_resume;
	if (!err)
     		register_early_suspend(&apds9900data.early_suspend);
#endif

#if defined SENSOR_APDS9900_ISR_VER3
	if (!err)
		err = apds9900_irq_init();
#endif

	dbg_func_out();
	
	return err;
}

static int __devexit apds9900_remove(struct i2c_client *client)
{
	int ret;
		
	dbg_func_in();

	apds9900_deinit_suspend();

	/* Power down the device */
	ret = apds9900_control_enable(E_ACTIVE_ALL, 0);
	
	kfree(i2c_get_clientdata(client));
	if(apds9900data.hysps.buf != NULL) kfree(apds9900data.hysps.buf);
	if(apds9900data.hysls.buf != NULL) kfree(apds9900data.hysls.buf);

	dbg_func_out();
	
	return 0;
}

#ifdef CONFIG_PM

static int apds9900_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret = 0;

	dbg_func_in();

	dbg_data("apds9900_suspend apds9900data.incall=%d", apds9900data.incall);


	if (( apds9900data.incall )&&(apds9900data.active_app) && (apds9900data.power == E_POWER_ON)) {
		printk(KERN_INFO "##### apds9900_suspend but  incall =%d active_app=%d,power = %d \n",
			apds9900data.incall,apds9900data.active_app,apds9900data.power);
		ret = apds9900_set_irq(E_ENABLE); //[DS 2TEAM SHS]apds9900 device driver interrupt enable
	}
	else if ((apds9900data.active_app) && (apds9900data.power == E_POWER_ON)){
		printk(KERN_INFO "##### apds9900_suspend but  incall =%d active_app=%d,power = %d \n",
			apds9900data.incall,apds9900data.active_app,apds9900data.power);
		ret = apds9900_set_irq(E_ENABLE); 
		return ;
	}
	else {
		printk(KERN_INFO "##### apds9900_suspend normal incall =%d active_app=%d,power = %d \n",
			apds9900data.incall,apds9900data.active_app,apds9900data.power);
#ifdef APDS9900_ACTIVE_NEW
	ret = apds9900_control_enable_real(E_ACTIVE_ALL, 0);
#else
		ret = apds9900_control_enable(E_ACTIVE_ALL, 0); //[DS 2TEAM SHS] device driver power off
#endif		
	}

	dbg_func_out();

	return ret; 
}

static int apds9900_resume(struct i2c_client *client)
{

	int ret = 0;

	dbg_func_in();

//	ret = apds9900_set_irq(E_DISABLE);
	apds9900_clear_hysteresis(E_ACTIVE_ALL);
#ifdef APDS9900_ACTIVE_NEW
       if(apds9900data.active_app){
		apds9900_control_enable_real(apds9900data.active_app, 1);
	       ret = apds9900_set_irq(E_DISABLE);
       }
#endif
	dbg_func_out();

	return ret;

}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void apds9900_early_suspend(struct early_suspend *handler)
{
	struct i2c_client *client = apds9900data.client;
	printk(KERN_INFO "##### apds9900_early_suspend ---- \n");
	apds9900_suspend(client,PMSG_SUSPEND);
}

static void apds9900_late_resume(struct early_suspend *handler)
{
	struct i2c_client *client = apds9900data.client;
	printk(KERN_INFO "##### apds9900_late_resume ---- \n");
	apds9900_resume(client);
}
#endif  /* CONFIG_HAS_EARLYSUSPEND */
#else

#define apds9900_suspend	NULL
#define apds9900_resume		NULL

#endif /* CONFIG_PM */

static const struct i2c_device_id apds9900_id[] = {
	{ "apds9900", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, apds9900_id);


#ifdef APDS9900_FROYO_UPDATE  //2011.01.03 Forever AS Froyo Update		
const static struct dev_pm_ops i2c_device_apds9900_pm_ops = {
             .suspend = apds9900_suspend,
             .resume = apds9900_resume,
};
#endif

static struct i2c_driver apds9900_driver = {
	.driver = {
		.name	= APDS9900_DRV_NAME,
		.owner	= THIS_MODULE,
#ifdef APDS9900_FROYO_UPDATE//2011.01.03 Forever AS Froyo Update		
              .pm = &i2c_device_apds9900_pm_ops,
#endif

	},
#ifndef APDS9900_FROYO_UPDATE//2011.01.03 Forever AS Froyo Update			
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = apds9900_suspend,
	.resume	= apds9900_resume,
#endif	
#endif	
	.probe	= apds9900_probe,
	.remove	= __devexit_p(apds9900_remove),
	.id_table = apds9900_id,
};

static int __init apds9900_init(void)
{
	return i2c_add_driver(&apds9900_driver);
}

static void __exit apds9900_exit(void)
{
	i2c_del_driver(&apds9900_driver);
}

MODULE_AUTHOR("Lee Kai Koon <kai-koon.lee@avagotech.com>");
MODULE_DESCRIPTION("APDS9900 ambient light + proximity sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(apds9900_init);
module_exit(apds9900_exit);
