/*
 * pantech-bl.c
 * Copyright (c) Jinwoo Cha <jwcha@pantech.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 * H1940 leds driver
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include <linux/err.h>
//#include <asm/arch/regs-gpio.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <linux/err.h>


#include "smd_rpcrouter.h"
#include "proc_comm.h"

#define	HAVE_KEYBOARD_LED	0

#define	PM_LIBPROG	0x30000061
#define PM_LIBVERS	0x00010001
#define PM_SECURE_MPP_CONFIG_I_SINK_PROC	6
#define PM_SET_LED_INTENSITY_PROC		16	

#define PM_LCD_LED	0
#define	PM_KBD_LED	1

// TBD
// 0 ~ 15 (0mA ~ -150mA)
#define	KEYBOARD_LED_LEVEL	3
#define	FRONT_LED_LEVEL		3
#define	LED_OFF_LEVEL		0

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
  PM_MPP__I_SINK__LEVEL_5mA,
  PM_MPP__I_SINK__LEVEL_10mA,
  PM_MPP__I_SINK__LEVEL_15mA,
  PM_MPP__I_SINK__LEVEL_20mA,
  PM_MPP__I_SINK__LEVEL_25mA,
  PM_MPP__I_SINK__LEVEL_30mA,
  PM_MPP__I_SINK__LEVEL_35mA,
  PM_MPP__I_SINK__LEVEL_40mA,
  PM_MPP__I_SINK__LEVEL_INVALID
}pm_mpp_i_sink_level_type;

typedef enum
{
  PM_MPP__I_SINK__SWITCH_DIS,
  PM_MPP__I_SINK__SWITCH_ENA,
  PM_MPP__I_SINK__SWITCH_ENA_IF_MPP_HIGH,
  PM_MPP__I_SINK__SWITCH_ENA_IF_MPP_LOW,
  PM_MPP__I_SINK__SWITCH_INVALID
}pm_mpp_i_sink_switch_type;

static struct msm_rpc_endpoint *pm_lib_ep;
static int CompVer = PM_LIBVERS;

typedef struct {
	struct rpc_request_hdr	hdr;
	uint32_t	led;
	uint32_t	val;	// cp use 1byte, but we have to use 4bytes, WHY?
} msm_pm_set_led_intensity_req;

typedef struct {
	struct rpc_reply_hdr	hdr;
	uint32_t	err_flag;
} msm_pm_set_led_intensity_rep;

typedef struct {
	struct rpc_request_hdr	hdr;
	uint32_t	mpp;
	uint32_t	level;	
	uint32_t	onoff;	
} msm_pm_secure_mpp_config_i_sink_req;

typedef struct {
	struct rpc_reply_hdr	hdr;
	uint32_t	err_flag;
} msm_pm_secure_mpp_config_i_sink_rep;

/*
 * Front backlight (LCD_DRV_N)
 * front key
 * brightness (0~255)
 */
void pantech_button_bl_set(struct led_classdev *led_dev, enum led_brightness value)
{
#if	0
	msm_pm_set_led_intensity_req req;
	msm_pm_set_led_intensity_rep rep;
#else
	msm_pm_secure_mpp_config_i_sink_req	req;
	msm_pm_secure_mpp_config_i_sink_rep	rep;
#endif
	int	state;
	int	rc = 0;

	printk(KERN_ERR "%s: button led set %d\n", __func__, value);

	if (value) {
#if	0
		//pm_mpp_config_i_sink(PM_MPP_7, PM_MPP__I_SINK__LEVEL_5mA,
		//			PM_MPP__I_SINK__SWITCH_ENA);
		req.led = cpu_to_be32(PM_LCD_LED);	// 0
		req.val = cpu_to_be32(FRONT_LED_LEVEL);	// 3 
		rc = msm_rpc_call_reply(pm_lib_ep,
				PM_SET_LED_INTENSITY_PROC,
				&req, sizeof(req), 
				&rep, sizeof(rep), 
				5 * HZ);
#else
		//pm_mpp_config_i_sink(PM_MPP_7, PM_MPP__I_SINK__LEVEL_5mA,
		//			PM_MPP__I_SINK__SWITCH_ENA);
		req.mpp   = cpu_to_be32(PM_MPP_7);	
		req.level = cpu_to_be32(PM_MPP__I_SINK__LEVEL_5mA);	
		req.onoff = cpu_to_be32(PM_MPP__I_SINK__SWITCH_ENA);	
		rc = msm_rpc_call_reply(pm_lib_ep,
				PM_SECURE_MPP_CONFIG_I_SINK_PROC,
				&req, sizeof(req), 
				&rep, sizeof(rep), 
				5 * HZ);
#endif
	} else {
#if	0
		//pm_set_led_intensity(PM_KBD_LED, PM_LED_KBD_SETTING_LEVEL0);
		req.led = cpu_to_be32(PM_LCD_LED);	// 0
		req.val = cpu_to_be32(LED_OFF_LEVEL);	// 0
		rc = msm_rpc_call_reply(pm_lib_ep,
				PM_SET_LED_INTENSITY_PROC,
				&req, sizeof(req), 
				&rep, sizeof(rep), 
				5 * HZ);
#else
		//pm_mpp_config_i_sink(PM_MPP_7, PM_MPP__I_SINK__LEVEL_5mA,
		//			PM_MPP__I_SINK__SWITCH_DIS);
		req.mpp   = cpu_to_be32(PM_MPP_7);	
		req.level = cpu_to_be32(PM_MPP__I_SINK__LEVEL_5mA);	
		req.onoff = cpu_to_be32(PM_MPP__I_SINK__SWITCH_DIS);	
		rc = msm_rpc_call_reply(pm_lib_ep,
				PM_SECURE_MPP_CONFIG_I_SINK_PROC,
				&req, sizeof(req), 
				&rep, sizeof(rep), 
				5 * HZ);
#endif
	}

	if (rc < 0)
		printk(KERN_ERR "%s: msm_rpc_call failed! rc = %d\n", 
				__func__, rc);

}
/* for native */


static struct led_classdev pantech_button_backlight = {
	.name			= "button-backlight",
	.brightness_set		= pantech_button_bl_set,
	//.default_trigger	= "h1940-charger",
};

#if	HAVE_KEYBOARD_LED
/*
 * Keyboard backlight (KPD_DRV_N)
 * qwerty key
 * brightness (0~255)
 */
void pantech_keyboard_bl_set(struct led_classdev *led_dev, enum led_brightness value)
{
	msm_pm_set_led_intensity_req req;
	msm_pm_set_led_intensity_rep rep;
	int	state;
	int	rc = 0;

	state = gpio_get_value(92);	// SLIDESENSE

	printk(KERN_ERR "%s: keyboard led set %d (slidesense: %d)\n", __func__, value, state);

	//if ((state == 1)||(value == 0)) {	// in slide open status only

		if (value) {
			//pm_set_led_intensity(PM_KBD_LED, PM_LED_KBD_SETTING_LEVEL3);
			req.led = cpu_to_be32(PM_KBD_LED);	// 1
			req.val = cpu_to_be32(KEYBOARD_LED_LEVEL);	// 3
			rc = msm_rpc_call_reply(pm_lib_ep,
					PM_SET_LED_INTENSITY_PROC,
					&req, sizeof(req), 
					&rep, sizeof(rep), 
					5 * HZ);
		} else {
			//pm_set_led_intensity(PM_KBD_LED, PM_LED_KBD_SETTING_LEVEL0);
			req.led = cpu_to_be32(PM_KBD_LED);	// 1
			req.val = cpu_to_be32(LED_OFF_LEVEL);	// 0
			rc = msm_rpc_call_reply(pm_lib_ep,
					PM_SET_LED_INTENSITY_PROC,
					&req, sizeof(req), 
					&rep, sizeof(rep), 
					5 * HZ);
		}

		if (rc < 0)
			printk(KERN_ERR "%s: msm_rpc_call failed! rc = %d\n", 
					__func__, rc);
	
	//}
}
/* for native */


static struct led_classdev pantech_keyboard_backlight = {
	.name			= "keyboard-backlight",
	.brightness_set		= pantech_keyboard_bl_set,
	//.default_trigger	= "h1940-bluetooth",
};
#endif	// HAVE_KEYBOARD_LED

static int __init pantechbl_probe(struct platform_device *pdev)
{
	int ret;

	printk(KERN_ERR "%s is called\n", __func__);

	ret = led_classdev_register(&pdev->dev, &pantech_button_backlight);
#if	1
	// PM RPC connection setup
	if (ret == 0) {
		pm_lib_ep = msm_rpc_connect(PM_LIBPROG, CompVer, 0);
		if (IS_ERR(pm_lib_ep)) {
			printk(KERN_ERR "%s: init rpc failed! rc = %ld\n",
				__func__, PTR_ERR(pm_lib_ep));
			ret = PTR_ERR(pm_lib_ep);
		}	
	}
#endif

	if (ret)
		goto err_buttonbl;

#if	HAVE_KEYBOARD_LED
	ret = led_classdev_register(&pdev->dev, &pantech_keyboard_backlight);
	if (ret)
		goto err_keybdbl;
#endif	// HAVE_KEYBOARD_LED

	return 0;

err_keybdbl:
	led_classdev_unregister(&pantech_button_backlight);
err_buttonbl:
err_lcdbl:
	return ret;
}

static int pantechbl_remove(struct platform_device *pdev)
{
	led_classdev_unregister(&pantech_button_backlight); // front
#if	HAVE_KEYBOARD_LED
	led_classdev_unregister(&pantech_keyboard_backlight);	// qwerty
#endif	// HAVE_KEYBOARD_LED
	return 0;
}


static struct platform_driver pantechbl_driver = {
	.driver		= {
		.name	= "pantech-bl",
		.owner	= THIS_MODULE,
	},
	.probe		= pantechbl_probe,
	.remove		= pantechbl_remove,
};


static int __init pantech_backlight_init(void)
{
	int rc;
	uint32_t ver;

	printk(KERN_ERR "%s is called\n", __func__);

/// test wjmin
#if	0
	// find compatible server with major version matching
	rc = msm_rpc_get_compatible_server(PM_LIBPROG, PM_LIBVERS, &ver);
	if (rc < 0) {
		printk(KERN_ERR
			"Unable to find compatible version for API "
			"%#010x with ver %#010x\n",
			PM_LIBPROG, PM_LIBVERS);
		return rc;
	}
	printk(KERN_ERR "pantech-bl Registering with rs%08x:%08x\n",
		PM_LIBPROG, ver);
	CompVer = ver;
#endif

	rc =  platform_driver_register(&pantechbl_driver);
	printk(KERN_ERR "%s: platform_driver_register rc = %d\n",
			__func__, rc);

	return rc;
}

static void __exit pantech_backlight_exit(void)
{
	platform_driver_unregister(&pantechbl_driver);
}

// c.f. init.h
//postcore_initcall(pantech_backlight_init);	// 2nd stage
//device_initcall(pantech_backlight_init);	// 6th stage
late_initcall(pantech_backlight_init);	// 7th stage

//module_init(pantech_backlight_init);
//module_exit(pantech_backlight_exit);

MODULE_AUTHOR("Jinwoo Cha <jwcha@pantech.com>");
MODULE_DESCRIPTION("pantech backlight driver");
MODULE_LICENSE("GPL");
