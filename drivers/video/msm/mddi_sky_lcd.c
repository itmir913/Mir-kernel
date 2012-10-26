

/* Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
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
 
/**********************************************************************/
/*					*/
/* Include	*/
/*					*/
/***********************************************************************/
//#include "../../../../pantech/include/CUST_SKY.h"
#include "msm_fb.h"
#include "mddihost.h"
#include "mddihosti.h"
#include <mach/vreg.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <mach/gpio.h>

/**********************************************************************/
/*				*/
/* Define */
/*				*/
/***********************************************************************/
#ifdef SKY_MDDI_DEBUG
#define ENTER_FUNC2()	 	printk(KERN_ERR "[MDDI_SKY_LCD] Enter %s\n", __FUNCTION__);
#define EXIT_FUNC2() 	 	printk(KERN_ERR "[MDDI_SKY_LCD] Exit %s\n", __FUNCTION__);
#define PRINT_DEBUG(s,x) 	printk(KERN_ERR "[MDDI_SKY_LCD] %s:%s(%d)\n", __FUNCTION__,s,x);
#define PRINT_DEBUG_STR(s)	printk(KERN_ERR "[MDDI_SKY_LCD] %s:%s\n", __FUNCTION__,s);
#else
#define ENTER_FUNC2()	 
#define EXIT_FUNC2() 	 
#define PRINT_STR(X)  	 
#define PRINT_DEBUG(S,X)    
#define PRINT_DEBUG_STR(s)
#endif

/*LCD HW Feature*/
#define MDDI_RESET_PIN			23			//Low Active
#define LCD_RESET_PIN			101			//Low Active
#define LCD_DIM_CON_PIN			82			
#define LCD_VSync_PIN			97
#define LCD_GPIO_ID_PIN 		57

#define GPIO_HIGH_VALUE			1
#define GPIO_LOW_VALUE			0

#define SKY_LCD_WIDTH       	320
#define SKY_LCD_HEIGHT      	480

#define SKY_LCD_HW_RESET(){ \
	gpio_set_value(MDDI_RESET_PIN, GPIO_HIGH_VALUE); \
	mddi_wait(5); \
    gpio_set_value(MDDI_RESET_PIN ,GPIO_LOW_VALUE); \
    mddi_wait(20); \
    gpio_set_value(MDDI_RESET_PIN, GPIO_HIGH_VALUE); \
    mddi_wait(10); \
}

/*Backlight Driver Timing : MAX8848Y*/
#define T_INT		300	// min 120us
#define T_HI		3	// min 1us
#define T_LO		2	// min 1us
#define T_SHDN 	15	// typ 8ms

#define MAX8848Y_BLU_MAX       28
#define MAX8848Y_BLU_OFF  	   MAX8848Y_BLU_MAX
#define MAX8848Y_REFRESH_CYCLE 31

/*Samsung LCD Resister*/
//#if (BOARD_VER == EF31S_PT10) || (BOARD_VER == EF32K_PT10)				//S6D05A1
//Level1 Command
#define LCD_REG_TEARING_EFFECT_LINE_ON 		0x35
#define LCD_REG_TEARING_SCAN_LINE 			0x44 
//Level2 Command
#define LCD_REG_PASSWD1						0xF0
#define LCD_REG_DISCTL						0xF2
#define LCD_REG_PWRCTL						0xF4
#define LCD_REG_VCMCTL						0xF5
#define LCD_REG_SRCCTL						0xF6
#define LCD_REG_GAMMASEL 					0xF9
#define LCD_REG_PGAMMACTL 					0xFA
#define LCD_REG_NGAMMACTL 					0xFB

//#elif (BOARD_VER == EF31S_WS10)	|| (BOARD_VER == EF32K_WS10)			//S6D16A0
//Level1 Command
#define LCD_REG_NORON						0x13
#define LCD_REG_TEON						0x35
#define LCD_REG_CASET						0x2A
#define LCD_REG_PXLOFF						0x22
#define LCD_REG_PXLON						0x23
#define LCD_REG_PASET						0x2B
#define LCD_REG_WRDISBV						0x51
#define LCD_REG_WRCTRLD						0x53
#define LCD_REG_WRCABC						0x55
//Level2 Command
#define LCD_REG_PASSWD2						0xF1
#define LCD_REG_CLKCTL3						0xB7
#define LCD_REG_HOSTCTL1					0xB8
#define LCD_REG_HOSTCTL2					0xB9
#define LCD_REG_MDDI1						0xE7
#define LCD_REG_MDDI2						0xE8
//#else
//#error "Board version is not defined"
//#endif

//Common Register
#define LCD_REG_SLEEP_IN			 		0x10
#define LCD_REG_SLEEP_OUT			 		0x11
#define LCD_REG_DISPLAY_OFF			 		0x28
#define LCD_REG_DISPLAY_ON			 		0x29
#define LCD_REG_MEMORY_DATA_ACCESS_CONTROL  0x36
#define LCD_REG_INTERFACE_PIXEL_FORMAT		0x3A
#define LCD_REG_ID1							0xDA
#define LCD_REG_ID2							0xDB
#define LCD_REG_ID3							0xDC
#define LCD_REG_MWC							0x3C		// [BIH] New LCD... S6D05A1-X12
#define LCD_REG_RDDPM						0x0A		// [BIH] New LCD... S6D05A1-X12
#if 0
static int __init mddi_sky_probe(struct platform_device *pdev);	
#else
static int __devinit mddi_sky_probe(struct platform_device *pdev);	
#endif
static void mddi_sky_enter_sleep(void);
static int mddi_sky_exit_sleep(void);
static int mddi_sky_lcd_on(struct platform_device *pdev);
static int mddi_sky_lcd_off(struct platform_device *pdev);
static void mddi_sky_set_backlight(struct msm_fb_data_type *mfd);
#ifdef SKYMENU_ADD_INITLCD	// novapex_hsj 2011-01-20
static int mddi_sky_prim_initLCD(void);
#endif

/**********************************************************************/
/*							*/
/* Struct - ENUM */
/*								*/
/***********************************************************************/
struct sky_mddi_lcd_driver_config{
	boolean Mddi_Initialize;
	boolean Mddi_Sleep;
	boolean Mddi_NewLCD;
};

static struct platform_driver this_driver = {
	.probe  = mddi_sky_probe,
	.driver = {
		.name   = "mddi_sky_lcd",
	},
};

static struct msm_fb_panel_data mddi_sky_panel_data = {
	.on = mddi_sky_lcd_on,
	.off = mddi_sky_lcd_off,
	.set_backlight = mddi_sky_set_backlight,
#ifdef SKYMENU_ADD_INITLCD	// novapex_hsj 2011-01-20
	.initLCD = mddi_sky_prim_initLCD,
#endif
	//.set_vsync_notifier = mddi_sky_vsync_set_handler,   //TODO
};

static struct platform_device this_device = {
	.name   = "mddi_sky_lcd",
	.id		= 1,
	.dev	= {
		.platform_data = &mddi_sky_panel_data,
	}
};

/**********************************************************************/
/*							*/
/* Global value	*/
/*							*/
/***********************************************************************/
static struct msm_panel_common_pdata *mddi_sky_pdata;

static struct sky_mddi_lcd_driver_config MddiLcdConfig = {
	.Mddi_Initialize = FALSE,
	.Mddi_Sleep      = TRUE,
	.Mddi_NewLCD = FALSE
};

static uint32 sky_lcd_gpio_init_table[] = {
	GPIO_CFG(LCD_RESET_PIN,    0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(LCD_DIM_CON_PIN,  0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(LCD_GPIO_ID_PIN,  0, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(MDDI_RESET_PIN,   0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};

#if 1//(BOARD_VER == EF31S_PT10) || (BOARD_VER == EF32K_PT10)		
__attribute__((__aligned__(32)))
static uint8 sky_mddi_lcd_reg_val[][32] = {// [BIH] New LCD... S6D05A1-X12
	//LCD_REG_DISCTL - 19
	{0x3B, 0x33, 0x03, 0x08, 0x08,  0x08, 0x08, 0x00, 0x08, 0x28,  0x00, 0x00, 0x00, 0x00, 0x53,  0x08, 0x08, 0x08, 0x08, },   
	//LCD_REG_PWRCTL - 14
	{0x07, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x04, 0x70,  0x04, 0x04, 0x70, 0x04, },	
	//LCD_REG_VCMCTL - 12
	{0x00, 0x60, 0x70, 0x00, 0x00,  0x02, 0x00, 0x00, 0x04, 0x00,  0x60, 0x70, },	
	//LCD_REG_SRCCTL - 8	
	{0x02, 0x00, 0x08, 0x03, 0x01,  0x00, 0x01, 0x00,},
	//LCD_REG_PGAMMACTL - 16
	{0x13, 0x0D, 0x02, 0x1F, 0x24,  0x25, 0x24, 0x22, 0x2F, 0x33,  0x3B, 0x3C, 0x16, 0x00, 0x00,  0x00, },
	//LCD_REG_NGAMMACTL - 16
	{0x00, 0x13, 0x12, 0x3C, 0x3B,  0x37, 0x2F, 0x22, 0x25, 0x21,  0x1A, 0x18, 0x03, 0x00, 0x00,  0x00 },
	//LCD_REG_PGAMMACTL - 16
	{0x17, 0x0F, 0x05, 0x22, 0x22,  0x21, 0x24, 0x22, 0x2D, 0x30,  0x37, 0x39, 0x17, 0x00, 0x00,  0x00, },
	//LCD_REG_NGAMMACTL - 16
	{0x00, 0x17, 0x13, 0x3F, 0x3E,  0x36, 0x32, 0x25, 0x20, 0x22,  0x21, 0x21, 0x05, 0x00, 0x00,  0x00, },
	//LCD_REG_PGAMMACTL - 16
	{0x00, 0x06, 0x28, 0x3F, 0x3F,  0x3E, 0x38, 0x0F, 0x21, 0x2B,  0x39, 0x3A, 0x16, 0x00, 0x00,  0x00, },
	//LCD_REG_NGAMMACTL - 16
	{0x00, 0x05, 0x11, 0x2D, 0x2B,  0x27, 0x21, 0x11, 0x38, 0x3C,  0x3F, 0x3F, 0x2B, 0x00, 0x00,  0x00, },
};
#endif

static uint16 mddi_sky_vsync_attempts;
static uint32 sky_mddi_rows_per_second		  = 29760;	
static uint32 sky_mddi_rows_per_refresh		  = 496;	
static uint32 sky_mddi_usecs_per_refresh 	  = 16666;	
static boolean mddi_sky_monitor_refresh_value = TRUE;
static boolean mddi_sky_report_refresh_measurements = FALSE;

static msm_fb_vsync_handler_type mddi_sky_vsync_handler;
static void *mddi_sky_vsync_handler_arg;
static void mddi_sky_vsync_set_handler(msm_fb_vsync_handler_type handler, void *arg);

static struct vreg *vreg_gp2;
static struct vreg *vreg_gp5;

static mddi_client_capability_type  MddiCap;

extern boolean mddi_vsync_detect_enabled;

extern bool bWakeupEnabled;
extern int iWakeupCount;
extern int iSleepCount;

/**********************************************************************/
/*					*/
/* Function */
/*          */
/***********************************************************************/	
/**********************************************************************/
/*@brief */
/*@param */
/*@return */
/***********************************************************************/
static void sky_lcd_gpio_init(uint32 *table, int len, unsigned enable)
{
  int n, rc;
  ENTER_FUNC2(); 
  
  for (n = 0; n < len; n++) {
  	printk("Cnt:%d", n);
    rc = gpio_tlmm_config(table[n],
      enable ? GPIO_CFG_ENABLE : GPIO_CFG_DISABLE);
    if (rc) {
      printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
        __func__, table[n], rc);
      //break;
    }	
  }
  EXIT_FUNC2();
}

/**********************************************************************
/*@brief 
/*@param
/*@return
***********************************************************************/
static int mddi_sky_prim_lcd_initial(void)
{
	int ret = 0;
	
	ENTER_FUNC2();
	
	if (gpio_get_value(LCD_GPIO_ID_PIN) !=0x00) {
		MddiLcdConfig.Mddi_NewLCD = TRUE;
	}
	else {
		MddiLcdConfig.Mddi_NewLCD = FALSE;
	}
	
	printk("[MDDI_SKY_LCD] %s: MddiLcdConfig.Mddi_NewLCD: %d\n", __func__, MddiLcdConfig.Mddi_NewLCD);	// [BIH] New LCD... S6D05A1-X12
	
	if (MddiLcdConfig.Mddi_NewLCD == TRUE) {
		ret = mddi_queue_register_write(LCD_REG_PASSWD1, 0x00005A5A, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_PASSWD1, ret);
		}
		
		ret = mddi_queue_register_write_extn(LCD_REG_DISCTL, (uint32 *)sky_mddi_lcd_reg_val[0], MDDI_DATA_PACKET_24_BYTES / 4, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_DISCTL, ret);
		}
		
		ret = mddi_queue_register_write_extn(LCD_REG_PWRCTL, (uint32 *)sky_mddi_lcd_reg_val[1], MDDI_DATA_PACKET_16_BYTES / 4, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_PWRCTL, ret);
		}
		
		ret = mddi_queue_register_write_extn(LCD_REG_VCMCTL, (uint32 *)sky_mddi_lcd_reg_val[2], MDDI_DATA_PACKET_12_BYTES / 4, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_VCMCTL, ret);
		}
		
		ret = mddi_queue_register_write_extn(LCD_REG_SRCCTL, (uint32 *)sky_mddi_lcd_reg_val[3], MDDI_DATA_PACKET_8_BYTES / 4, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_SRCCTL, ret);
		}
		
		ret = mddi_queue_register_write(LCD_REG_GAMMASEL, 0x00000014, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_GAMMASEL, ret);
		}
		
		ret = mddi_queue_register_write_extn(LCD_REG_PGAMMACTL, (uint32 *)sky_mddi_lcd_reg_val[4], MDDI_DATA_PACKET_16_BYTES / 4, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_PGAMMACTL, ret);
		}
		
		ret = mddi_queue_register_write_extn(LCD_REG_NGAMMACTL, (uint32 *)sky_mddi_lcd_reg_val[5], MDDI_DATA_PACKET_16_BYTES / 4, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_NGAMMACTL, ret);
		}
		
		ret = mddi_queue_register_write(LCD_REG_GAMMASEL, 0x00000012, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_GAMMASEL, ret);
		}
		
		ret = mddi_queue_register_write_extn(LCD_REG_PGAMMACTL, (uint32 *)sky_mddi_lcd_reg_val[6], MDDI_DATA_PACKET_16_BYTES / 4, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_PGAMMACTL, ret);
		}
		
		ret = mddi_queue_register_write_extn(LCD_REG_NGAMMACTL, (uint32 *)sky_mddi_lcd_reg_val[7], MDDI_DATA_PACKET_16_BYTES / 4, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_NGAMMACTL, ret);
		}
		
		ret = mddi_queue_register_write(LCD_REG_GAMMASEL, 0x00000011, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_GAMMASEL, ret);
		}
		
		ret = mddi_queue_register_write_extn(LCD_REG_PGAMMACTL, (uint32 *)sky_mddi_lcd_reg_val[8], MDDI_DATA_PACKET_16_BYTES / 4, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_PGAMMACTL, ret);
		}
		
		ret = mddi_queue_register_write_extn(LCD_REG_NGAMMACTL, (uint32 *)sky_mddi_lcd_reg_val[9], MDDI_DATA_PACKET_16_BYTES / 4, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_NGAMMACTL, ret);
		}
		
		ret = mddi_queue_register_write(LCD_REG_TEARING_SCAN_LINE, 0x00000100, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_TEARING_SCAN_LINE, ret);
		}
		
		ret = mddi_queue_register_write(LCD_REG_TEARING_EFFECT_LINE_ON, 0x00000000, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_TEARING_EFFECT_LINE_ON, ret);
		}
		
		ret = mddi_queue_register_write(LCD_REG_MEMORY_DATA_ACCESS_CONTROL, 0x00000098, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_MEMORY_DATA_ACCESS_CONTROL, ret);
		}
		
		ret = mddi_queue_register_write(LCD_REG_CASET, 0x3F010000, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_CASET, ret);
		}
		
		ret = mddi_queue_register_write(LCD_REG_PASET, 0xDF010000, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_PASET, ret);
		}
		
		ret = mddi_queue_register_write(LCD_REG_INTERFACE_PIXEL_FORMAT, 0x00000077, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_INTERFACE_PIXEL_FORMAT, ret);
		}
		
		ret = mddi_queue_register_write(LCD_REG_MWC, 0x00000000, TRUE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_MWC, ret);
		}
	}
	else {
		ret = mddi_queue_register_write(LCD_REG_PASSWD2, 0x00005A5A, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_PASSWD2, ret);
		}
		
		ret = mddi_queue_register_write(LCD_REG_CLKCTL3, 0x00111100, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_CLKCTL3, ret);
		}		
		
		ret = mddi_queue_register_write(LCD_REG_HOSTCTL1, 0x00000231, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_HOSTCTL1, ret);
		}		
		
		ret = mddi_queue_register_write(LCD_REG_HOSTCTL2, 0x00000600, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_HOSTCTL2, ret);
		}	
		
		ret = mddi_queue_register_write(LCD_REG_TEON, 0x00000000, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_TEON, ret);
		}	
		
		ret = mddi_queue_register_write(LCD_REG_PASSWD2, 0x0000A5A5, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_PASSWD2, ret);
		}	
		
		ret = mddi_queue_register_write(LCD_REG_CASET, 0x3F010000, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_CASET, ret);
		}		
		
		ret = mddi_queue_register_write(LCD_REG_PASET, 0xDF010000, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_PASET, ret);
		}	
		
		ret = mddi_queue_register_write(LCD_REG_INTERFACE_PIXEL_FORMAT, 0x00000077, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_INTERFACE_PIXEL_FORMAT, ret);
		}
		
		ret = mddi_queue_register_write(LCD_REG_WRDISBV, 0x00000000, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_WRDISBV, ret);
		}
		
		ret = mddi_queue_register_write(LCD_REG_WRCTRLD, 0x00000000, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_WRCTRLD, ret);
		}
		
		ret = mddi_queue_register_write(LCD_REG_WRCABC, 0x00000000, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_WRCTRLD, ret);
		}
		
		ret = mddi_queue_register_write(LCD_REG_MEMORY_DATA_ACCESS_CONTROL, 0x00000040, FALSE, 0);
		if (ret) {
			printk(KERN_ERR "[MDDI_SKY_LCD] %s: mddi_queue_register_write failed(%d) : %d\n", __func__, LCD_REG_MEMORY_DATA_ACCESS_CONTROL, ret);
		}
	}
	
	EXIT_FUNC2();
	
	if (ret) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

/**********************************************************************
/*@brief 
/*@param
/*@return
***********************************************************************/
static int mddi_sky_prim_lcd_init(void)
{
	int ret = 0;
	uint32 RegData = 0x00;
	
   	mddi_get_capability(&MddiCap);
	
	printk(KERN_ERR "[MDDI_SKY_LCD] MddiCap: packet_length:0x%x, packet_type:0x%x, Mfr_Name:0x%x, Product_Code:0x%x, parameter_CRC:0x%x\n", 
		MddiCap.packet_length, 
		MddiCap.packet_type, 
		MddiCap.Mfr_Name, 
		MddiCap.Product_Code,  
		MddiCap.parameter_CRC);
	
	if (!MddiCap.packet_length || mddi_queue_register_read(LCD_REG_ID1, &RegData, true, 0)) {
		return FALSE;
	}
	
	ret = mddi_sky_prim_lcd_initial();
	mddi_sky_exit_sleep();
	
	return ret;
}

#ifdef SKYMENU_ADD_INITLCD	// novapex_hsj 2011-01-20
/**********************************************************************
/*@brief 
/*@param
/*@return
***********************************************************************/
static int mddi_sky_prim_initLCD(void)
{
	int ret = 0;
	
	MddiLcdConfig.Mddi_Initialize = TRUE;
	MddiLcdConfig.Mddi_Sleep = FALSE;
	
	ret = mddi_sky_prim_lcd_initial();
	
	return ret;
}
#endif

/**********************************************************************/
/*@brief */
/*@param	*/
/*@return	*/
/***********************************************************************/
static void mddi_sky_enter_sleep(void)
{

	ENTER_FUNC2();

	mddi_queue_register_write(LCD_REG_DISPLAY_OFF, 0x00, TRUE, 0);	
	if(MddiLcdConfig.Mddi_NewLCD==TRUE){
		mddi_wait(20);
	}
	mddi_queue_register_write(LCD_REG_SLEEP_IN, 0x00, TRUE, 0);	
	mddi_wait(150);
 
	EXIT_FUNC2();
}

/**********************************************************************/
/*@brief */
/*@param */
/*@return */
/***********************************************************************/
static int mddi_sky_exit_sleep(void)
{
	ENTER_FUNC2();

//	mddi_wait(30);

	if(MddiLcdConfig.Mddi_NewLCD==TRUE){
		mddi_queue_register_write(LCD_REG_CASET, 0x3F010000, FALSE, 0);
		mddi_queue_register_write(LCD_REG_PASET, 0xDF010000, FALSE, 0);
	}
	
	mddi_queue_register_write(LCD_REG_SLEEP_OUT, 0x00, FALSE, 0);
	mddi_wait(120);
	mddi_queue_register_write(LCD_REG_DISPLAY_ON, 0x00, FALSE, 0);

	mddi_wait(30);

	if(MddiLcdConfig.Mddi_NewLCD==TRUE){
		mddi_queue_register_write(LCD_REG_CASET, 0x3F010000, FALSE, 0);
		mddi_queue_register_write(LCD_REG_PASET, 0xDF010000, FALSE, 0);
	}

	mddi_queue_register_write(LCD_REG_SLEEP_OUT, 0x00, FALSE, 0);
	mddi_wait(120);
	mddi_queue_register_write(LCD_REG_DISPLAY_ON, 0x00, FALSE, 0);

	EXIT_FUNC2();
	return 0;
}

// novapex_hsj 20110309
static u8 backlightValue[] = {
	0, 25, 19, 16, 12, 8, 4, 1
};

/**********************************************************************/
/*@brief  	Limited current level : 20.8mA                */
/*@param	brightness = 7 : max / brightness = 0 : off	    */
/*@return																									*/
/***********************************************************************/
static void mddi_sky_change_brightness(u8 brightness)
{
	// novapex_hsj 20110309
	/*static u8 bk_lvl_old = 0;
	u8 bk_lvl, index;
	unsigned long flags;
	
	ENTER_FUNC2();
	
	//local_save_flags(flags);
	//local_irq_disable();
	
	bk_lvl = backlightValue[brightness];
	
	if (bk_lvl != bk_lvl_old) {
		bk_lvl_old = bk_lvl;
		
		if (bk_lvl == 0) {
			gpio_set_value(LCD_DIM_CON_PIN, GPIO_LOW_VALUE);
			mdelay(T_SHDN);
		}
		else {
			gpio_set_value(LCD_DIM_CON_PIN, GPIO_LOW_VALUE);
			mdelay(T_SHDN);
			gpio_set_value(LCD_DIM_CON_PIN, GPIO_HIGH_VALUE);
			udelay(T_INT);
			
			local_save_flags(flags);
			local_irq_disable();
			
			for (index = 1; index <= bk_lvl; index++) {
				gpio_set_value(LCD_DIM_CON_PIN, GPIO_LOW_VALUE);
				udelay(T_LO);
				gpio_set_value(LCD_DIM_CON_PIN, GPIO_HIGH_VALUE);
				udelay(T_HI);
			}
			
			local_irq_restore(flags);
		}
	}
	
	//local_irq_restore(flags);
	
	EXIT_FUNC2();*/

	// novapex_hsj_20110428__irq lock 구간이 길다는 의견이 있어 이전 코드수정
	static u8 previous_level = MAX8848Y_REFRESH_CYCLE;	// max_current_level = MAX8848Y_BLU_OFF;
	static u8 set_level = 0;
	static boolean first_start = FALSE;						// first_set_start = FALSE;
	int idx, count = 0;
	unsigned long frags;
	
	ENTER_FUNC2();
	
	if (brightness == 0) {								// Backlight Off
		previous_level = MAX8848Y_REFRESH_CYCLE;
		first_start = TRUE;
		
		gpio_set_value(LCD_DIM_CON_PIN, GPIO_LOW_VALUE);
		mdelay(T_SHDN);
		return;
	}
	
	if (first_start == TRUE) {
		first_start = FALSE;
		
		gpio_set_value(LCD_DIM_CON_PIN, GPIO_LOW_VALUE);
		mdelay(T_SHDN);
		gpio_set_value(LCD_DIM_CON_PIN, GPIO_HIGH_VALUE);
		udelay(T_INT);
	}
	
	set_level = backlightValue[brightness];
	
	if (previous_level == set_level) {
		return;
	}
	else if (set_level > previous_level) {					// 이전보다 어둡게
		count = set_level - previous_level;
	}
	else {										// 이전보다 밝게
		count = set_level + (MAX8848Y_REFRESH_CYCLE - previous_level);
	}
	
	local_save_flags(frags);
	local_irq_disable();
	
	for (idx = 0; idx < count; idx++) {
		gpio_set_value(LCD_DIM_CON_PIN, GPIO_LOW_VALUE);
		udelay(T_LO);
		gpio_set_value(LCD_DIM_CON_PIN, GPIO_HIGH_VALUE);
		udelay(T_HI);
	}
	
	local_irq_restore(frags);
	
	previous_level = set_level;
	
	EXIT_FUNC2();
}

/**********************************************************************/
/*@brief  */
/*@param  */
/*@return */   
/***********************************************************************/
static void mddi_sky_set_backlight(struct msm_fb_data_type *mfd)
{
#if 0//(BOARD_VER == EF31S_PT10) || (BOARD_VER == EF32K_PT10)
	return;   //TODO
#endif
	
	PRINT_DEBUG("Require BL Level", mfd->bl_level);
	
	if (MddiLcdConfig.Mddi_Initialize == FALSE) {
		PRINT_DEBUG_STR("MDDI don't initialize");
	}
	
	mddi_sky_change_brightness(mfd->bl_level);
}

/**********************************************************************/
/*@brief 	*/
/*@param 	*/
/*@return */
/***********************************************************************/
static int mddi_sky_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	ENTER_FUNC2();
	PRINT_DEBUG("Board Ver:",BOARD_VER);
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;

	if (mfd->key != MFD_KEY)
		return -EINVAL;

	if(MddiLcdConfig.Mddi_Initialize == FALSE){
		PRINT_DEBUG_STR("MDDI don't initialize");
		if(mddi_sky_prim_lcd_init()!=TRUE){
			mfd->MddiLcd_InitializeFail = TRUE;
			PRINT_DEBUG_STR("MDDI initialize Error")
		}
		else{
			mfd->MddiLcd_InitializeFail = FALSE;
			PRINT_DEBUG_STR("MDDI initialize")
		}
		MddiLcdConfig.Mddi_Initialize = TRUE;
		MddiLcdConfig.Mddi_Sleep = FALSE;
	}else if(MddiLcdConfig.Mddi_Sleep == TRUE){
#ifdef	CONFIG_EF31S_32K_BOARD		//pantch_lks@PS3_20110112 - ef31s_ef32k 
		if(mfd->MddiLcd_InitializeFail == TRUE)
			return 0;
#endif
		mddi_host_client_cnt_reset();		//[BIH] add RTD read cound set to zero. when this line obsolated, RTD read ISR operation executed before MDDI clock on, and makes some problem.
		if(mddi_sky_exit_sleep()!=0)	//[BIH] if mddi_sky_exit_sleep() failed by any reason,  take LCD h/w reset and init again.
		{
			SKY_LCD_HW_RESET();
			mddi_sky_prim_lcd_initial();
			mddi_sky_exit_sleep();
		}
		MddiLcdConfig.Mddi_Sleep = FALSE;
	}

	EXIT_FUNC2();
	return 0;
}

/**********************************************************************/
/*@brief 	*/
/*@param	*/
/*@return	*/
/***********************************************************************/
void sky_lcd_screen_off(void)
{
	//TODO - because of msm_fb.c error
}

/**********************************************************************/
/*@brief 	*/
/*@param	*/
/*@return	*/
/***********************************************************************/
static int mddi_sky_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	
	ENTER_FUNC2();
#ifdef	CONFIG_EF31S_32K_BOARD		//pantch_lks@PS3_20110112 - ef31s_ef32k 
	mfd = platform_get_drvdata(pdev);
	
	if (!mfd)
		return -ENODEV;	
	
	if(mfd->MddiLcd_InitializeFail == TRUE)
		return 0;
#endif	
#ifdef	CONFIG_EF31S_32K_BOARD		//pantch_lks@PS3_20110112 - ef31s_ef32k 
	if(mfd->MddiLcd_InitializeFail == TRUE)
		return 0;
#endif
	if(MddiLcdConfig.Mddi_Sleep == FALSE){		
		mddi_sky_change_brightness(0);
		mddi_sky_enter_sleep();		
		MddiLcdConfig.Mddi_Sleep = TRUE;
	}	
	EXIT_FUNC2();	
	return 0;
}

/**********************************************************************/
/*@brief */
/*@param */
/*@return */
/***********************************************************************/
void mddi_sky_lcd_vsync_detected(boolean detected)
{
	/* static timetick_type start_time = 0; */
	static struct timeval start_time;
	static boolean first_time = TRUE;
	/* uint32 mdp_cnt_val = 0; */
	/* timetick_type elapsed_us; */
	struct timeval now;
	uint32 elapsed_us;
	uint32 num_vsyncs;

	if ((detected) || (mddi_sky_vsync_attempts > 5)) {
		if ((detected) && (mddi_sky_monitor_refresh_value)) {
			/* if (start_time != 0) */
			if (!first_time) {
				jiffies_to_timeval(jiffies, &now);
				elapsed_us =
				    (now.tv_sec - start_time.tv_sec) * 1000000 +
				    now.tv_usec - start_time.tv_usec;
				/*
				* LCD is configured for a refresh every usecs,
				* so to determine the number of vsyncs that
				* have occurred since the last measurement add
				* half that to the time difference and divide
				* by the refresh rate.
				*/
				num_vsyncs = (elapsed_us +
					      (sky_mddi_usecs_per_refresh >>
					       1)) /
				    sky_mddi_usecs_per_refresh;
				/*
				 * LCD is configured for * hsyncs (rows) per
				 * refresh cycle. Calculate new rows_per_second
				 * value based upon these new measurements.
				 * MDP can update with this new value.
				 */
				sky_mddi_rows_per_second =
				    (sky_mddi_rows_per_refresh * 1000 *
				     num_vsyncs) / (elapsed_us / 1000);
			}
			/* start_time = timetick_get(); */
			first_time = FALSE;
			jiffies_to_timeval(jiffies, &start_time);
			if (mddi_sky_report_refresh_measurements) {
				/* mdp_cnt_val = MDP_LINE_COUNT; */
			}
		}
		/* if detected = TRUE, client initiated wakeup was detected */
		if (mddi_sky_vsync_handler != NULL) {
			(*mddi_sky_vsync_handler)
			    (mddi_sky_vsync_handler_arg);
			mddi_sky_vsync_handler_arg = NULL;
		}
		mddi_vsync_detect_enabled = FALSE;
		mddi_sky_vsync_attempts = 0;
		
#if 0 //- TODO		
		/* need to clear this vsync wakeup */ 
		if (!mddi_queue_register_write_int(REG_INTR, 0x0000)) {
			MDDI_MSG_ERR("Vsync interrupt clear failed!\n");
		}
#endif		

		if (!detected) {
			/* give up after 5 failed attempts but show error */
			MDDI_MSG_NOTICE("Vsync detection failed!\n");
		} else if ((mddi_sky_monitor_refresh_value) &&
			(mddi_sky_report_refresh_measurements)) {
			MDDI_MSG_NOTICE("  Lines Per Second=%d!\n",
				sky_mddi_rows_per_second);
		}
	} else
		/* if detected = FALSE, we woke up from hibernation, but did not
		 * detect client initiated wakeup.
		 */
		mddi_sky_vsync_attempts++;
}

/**********************************************************************/
/*@brief 		*/
/*@param		*/
/*@return		*/
/***********************************************************************/
/* ISR to be executed */
void mddi_sky_vsync_set_handler(msm_fb_vsync_handler_type handler, void *arg)
{
	boolean error = FALSE;
	unsigned long flags;

	/* Disable interrupts */
	spin_lock_irqsave(&mddi_host_spin_lock, flags);
	/* INTLOCK(); */

	if (mddi_sky_vsync_handler != NULL)
		error = TRUE;

	/* Register the handler for this particular GROUP interrupt source */
	mddi_sky_vsync_handler = handler;
	mddi_sky_vsync_handler_arg = arg;

	/* Restore interrupts */
	spin_unlock_irqrestore(&mddi_host_spin_lock, flags);
	/* INTFREE(); */

	if (error)
		MDDI_MSG_ERR("MDDI: Previous Vsync handler never called\n");
#if 0	//- TODO
	/* Enable the vsync wakeup */
	mddi_queue_register_write(REG_INTR, 0x8100, FALSE, 0);
#endif

	mddi_sky_vsync_attempts = 1;
	mddi_vsync_detect_enabled = TRUE;
}				
/**********************************************************************/
/*@brief 		*/
/*@param		*/
/*@return		*/
/***********************************************************************/
static int __devinit mddi_sky_probe(struct platform_device *pdev)
{
	ENTER_FUNC2(); 
	
	if (pdev->id == 0) {
		mddi_sky_pdata = pdev->dev.platform_data;
		return 0;
	}

	msm_fb_add_device(pdev);
	
	EXIT_FUNC2();
	return 0;
}

/*********************************************************************/
/*@brief 	*/
/*@param	*/
/*@return	*/
/***********************************************************************/
static int __init mddi_sky_init(void)
{
	int ret;
	struct msm_panel_info *pinfo;
	
	ENTER_FUNC2(); 
	PRINT_DEBUG_STR("KS LEE CHECK 3");
	
	/* Timing variables for tracking vsync*/
	/* Frame Frequency = Fosc/(8*NHW|PIHW*(Line+B) [Hz] = 60Hz
 	 * Fosc : 20Mhz, Line = 480, B(B+F proch) = 8+8 , NHW = 83
	 * horizontal count = 320+2+2
	 * vertical count   = 480+16 - Vertical B/F porch =8/8
	 * dot_clock >= 35Mhz
	*/
	sky_mddi_rows_per_second      = 44640;//29760;	// 60Hz * 496
	sky_mddi_rows_per_refresh     = 496; 	// vertical count
	sky_mddi_usecs_per_refresh    = 11111;//16666;	// 1/60Hz

	sky_lcd_gpio_init(sky_lcd_gpio_init_table, 
						ARRAY_SIZE(sky_lcd_gpio_init_table), 1);
	
	ret = platform_driver_register(&this_driver);
	if (!ret) {
		pinfo = &mddi_sky_panel_data.panel_info;
		pinfo->xres = SKY_LCD_WIDTH;
		pinfo->yres = SKY_LCD_HEIGHT;
		pinfo->type = MDDI_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->mddi.vdopkt = MDDI_DEFAULT_PRIM_PIX_ATTR;
		pinfo->wait_cycle = 0;
		pinfo->bpp = 24;
		pinfo->fb_num = 2;
		pinfo->clk_rate = 160000000;
		pinfo->clk_min = 160000000;
		pinfo->clk_max = 160000000;
		pinfo->lcd.vsync_enable = TRUE;
		pinfo->lcd.refx100 =
			(sky_mddi_rows_per_second * 100) /
			sky_mddi_rows_per_refresh;
		pinfo->lcd.v_back_porch = 8;
		pinfo->lcd.v_front_porch = 8;
		pinfo->lcd.v_pulse_width = 0;
		pinfo->lcd.hw_vsync_mode = TRUE;
		pinfo->lcd.vsync_notifier_period = (1 * HZ);
		pinfo->bl_max = 7;
		pinfo->bl_min = 1;

		ret = platform_device_register(&this_device);
		if (ret)
			platform_driver_unregister(&this_driver);
	
	}
/* - TODO
	if (!ret)
		mddi_lcd.vsync_detected = mddi_sky_lcd_vsync_detected;
*/
	EXIT_FUNC2();

	return ret;
}

module_init(mddi_sky_init);
