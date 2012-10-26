/*
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2008-2011, Code Aurora Forum. All rights reserved.
 * Author: Brian Swetland <swetland@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/bootmem.h>
#include <linux/power_supply.h>


#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>
#include <asm/setup.h>
#ifdef CONFIG_CACHE_L2X0
#include <asm/hardware/cache-l2x0.h>
#endif

#include <asm/mach/mmc.h>
#include <mach/vreg.h>
#include <mach/mpp.h>
#include <mach/board.h>
#include <mach/pmic.h>
#include <mach/msm_iomap.h>
#include <mach/msm_rpcrouter.h>
#include <mach/msm_hsusb.h>
#include <mach/rpc_hsusb.h>
#include <mach/rpc_pmapp.h>
#include <mach/msm_serial_hs.h>
#include <mach/memory.h>
#include <mach/msm_battery.h>
#include <mach/rpc_server_handset.h>
#include <mach/msm_tsif.h>
#include <mach/socinfo.h>

#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/i2c.h>
#include <linux/android_pmem.h>
#include <mach/camera.h>

#include "devices.h"
#include "clock.h"
#include "msm-keypad-devices.h"
#include "pm.h"
#ifdef CONFIG_ARCH_MSM7X27
#include <linux/msm_kgsl.h>
#endif

#ifdef CONFIG_USB_ANDROID
#include <linux/usb/android_composite.h>
#endif

#include <linux/skbuff.h>

#ifdef CONFIG_WIFI_CONTROL_FUNC //kychoi 110113
#include <linux/wlan_plat.h>
#endif

//#if defined(F_SKYCAM_I2C_GPIO)
#include <linux/i2c-gpio.h>
//#endif

//#if CONFIG_TOUCHSCREEN_CY8C_TS
#ifdef CONFIG_TOUCHSCREEN_CYTTSP_I2C
#include <linux/cyttsp.h>

static struct cyttsp_platform_data cyttsp_data = {
        .maxx = 320, //EF31S/EF32K 480,
        .maxy = 480, //EF31S/EF32K 800,   
        .flags = 0,
//      .gen = CYTTSP_SPI10,    /* either */
//      .gen = CYTTSP_GEN2,     /* or */
        .gen = CYTTSP_GEN3,     /* or */
        .use_st = CYTTSP_USE_ST,
        .use_mt = CYTTSP_USE_MT,
        .use_trk_id = CYTTSP_USE_TRACKING_ID,
        .use_sleep = CYTTSP_USE_SLEEP,
        .use_gestures = CYTTSP_USE_GESTURES,
       /* activate up to 4 groups
         * and set active distance
         */
        .gest_set = CYTTSP_GEST_GRP1 | CYTTSP_GEST_GRP2 |
                                CYTTSP_GEST_GRP3 | CYTTSP_GEST_GRP4 |
                                CYTTSP_ACT_DIST,
        /* change act_intrvl to customize the Active power state 
         * scanning/processing refresh interval for Operating mode
         */
        .act_intrvl = CYTTSP_ACT_INTRVL_DFLT,
        /* change tch_tmout to customize the touch timeout for the
         * Active power state for Operating mode
         */
        .tch_tmout = CYTTSP_TCH_TMOUT_DFLT,
        /* change lp_intrvl to customize the Low Power power state 
         * scanning/processing refresh interval for Operating mode
         */
        .lp_intrvl = CYTTSP_LP_INTRVL_DFLT,
};
#endif	// CONFIG_TOUCHSCREEN_CYTTSP_I2C

#if ((BOARD_VER >=EF31S_PT10) ||(BOARD_VER >=EF32K_PT10)) 
void msm_init_sky_vibrator(void);
#endif

#ifdef F_SKYCAM_I2C_GPIO
#define GPIO_PIN_CAMERA_SCL	1 
#define GPIO_PIN_CAMERA_SDA	0 

static struct i2c_gpio_platform_data i2c_gpio_camera_data = {
	.scl_pin = GPIO_PIN_CAMERA_SCL,
	.sda_pin = GPIO_PIN_CAMERA_SDA,
	.udelay = 1,//5	/* 100 KHz */
};

struct platform_device msm_device_i2c_gpio_camera = {
	.name = "i2c-gpio",
	.id = 4,
	.dev = {
    .platform_data = &i2c_gpio_camera_data,
	}
};
#endif	// F_SKYCAM_I2C_GPIO

#define CONFIG_SKY_SND_MAX8981	//CONFIG_SKY_SND_MAX97001 97001 -> 8981 amp test 101214
#define CONFIG_SKY_SND_EXTAMP
//#undef CONFIG_SKY_SND_EXTAMP
#ifndef CONFIG_SKY_SND_MAX8981	//97001 -> 8981 amp test 101214
#ifdef CONFIG_SKY_SND_MAX97001
#ifdef CONFIG_SKY_SND_EXTAMP
//#error1234
#include "./qdsp5/sky_snd_ext_amp_max97001.h"
#endif	// CONFIG_SKY_SND_EXTAMP
#if ((BOARD_VER >=EF31S_PT10) ||(BOARD_VER >=EF32K_PT10)) 
#ifdef CONFIG_SKY_SND_EXTAMP
#define GPIO_PIN_EXTAMP_SCL	123
#define GPIO_PIN_EXTAMP_SDA	122
static struct i2c_gpio_platform_data i2c_gpio_extamp_data = {
	.scl_pin = GPIO_PIN_EXTAMP_SCL,
	.sda_pin = GPIO_PIN_EXTAMP_SDA,
	.udelay = 3,	/* 150 KHz */

};
struct platform_device msm_device_i2c_gpio_subsystem = {
	.name = "i2c-gpio",
	.id = 5,
	.dev = {
    .platform_data = &i2c_gpio_extamp_data,
	}
};
#endif	// CONFIG_SKY_SND_EXTAMP
#endif	// BOARD_VER
#else	// CONFIG_SKY_SND_MAX97001
#ifdef CONFIG_SKY_SND_EXTAMP
#include "./qdsp5/sky_snd_ext_amp_tpa2051d3.h"
#endif
/*by elecjang 20100320 <==*/
//by elecjang 20100320
#if ((BOARD_VER >=EF31S_PT10) ||(BOARD_VER >=EF32K_PT10)) 
#ifdef CONFIG_SKY_SND_EXTAMP
#define GPIO_PIN_SUBSYSTEM_SCL	123
#define GPIO_PIN_SUBSYSTEM_SDA	122

static struct i2c_gpio_platform_data i2c_gpio_audio_data = {
	.scl_pin = GPIO_PIN_SUBSYSTEM_SCL,
	.sda_pin = GPIO_PIN_SUBSYSTEM_SDA,
	.udelay = 5,	/* 100 KHz */
};

struct platform_device msm_device_i2c_gpio_subsystem = {
	.name = "i2c-gpio",
	.id = 5,
	.dev = {
    .platform_data = &i2c_gpio_audio_data,
	}
};
#endif	// CONFIG_SKY_SND_EXTAMP
#endif	// BOARD_VER
#endif	// CONFIG_SKY_SND_MAX97001
#else
struct platform_device msm_device_subsystem = {
	.name = "max8981-amp",
	.id = -1,
};
#endif//CONFIG_SKY_SND_MAX8981
#ifdef CONFIG_ARCH_MSM7X25
#define MSM_PMEM_MDP_SIZE	0xb21000
#define MSM_PMEM_ADSP_SIZE	0x97b000
#define MSM_PMEM_AUDIO_SIZE	0x121000
#define MSM_FB_SIZE		0x200000
#define PMEM_KERNEL_EBI1_SIZE	0x64000
#endif

#if defined(CONFIG_SKY_TDMB_I2C_GPIO) || defined(CONFIG_SENSOR_I2C_GPIO) || defined(CONFIG_TOUCH_I2C_GPIO) || defined(F_SKYCAM_I2C_GPIO)
#include <linux/i2c-gpio.h>
#endif // CONFIG_SKY_TDMB_I2C_GPIO || CONFIG_SENSOR_I2C_GPIO || CONFIG_TOUCH_I2C_GPIO || F_SKYCAM_I2C_GPIO
#ifdef CONFIG_SKY_TDMB_I2C_GPIO
#if defined(CONFIG_EF31S_32K_BOARD)
#define GPIO_PIN_TDMB_SCL	29
#define GPIO_PIN_TDMB_SDA	28
#endif
static struct i2c_gpio_platform_data i2c_gpio_tdmb_data = {
	.scl_pin = GPIO_PIN_TDMB_SCL,
	.sda_pin = GPIO_PIN_TDMB_SDA,
	.udelay = 2,	/* 100 KHz */
};
struct platform_device msm_device_i2c_gpio_tdmb = {
	.name = "i2c-gpio",
	.id = 3,
	.dev = {
		.platform_data = &i2c_gpio_tdmb_data,
	}
};
#endif /* CONFIG_SKY_TDMB_I2C_GPIO */
#ifdef CONFIG_TOUCH_I2C_GPIO
#if ((BOARD_VER >=EF31S_PT10) ||(BOARD_VER >=EF32K_PT10)) 
#define GPIO_PIN_TOUCH_SCL		60
#define GPIO_PIN_TOUCH_SDA	61
#else
#define GPIO_PIN_TOUCH_SCL		92
#define GPIO_PIN_TOUCH_SDA	93
#endif // BOARD_VER



static struct i2c_gpio_platform_data i2c_gpio_touch_data = {
	.scl_pin = GPIO_PIN_TOUCH_SCL,
	.sda_pin = GPIO_PIN_TOUCH_SDA,
	.udelay = 3,	/* 150 KHz */
};

struct platform_device msm_device_i2c_gpio_touch = {
	.name = "i2c-gpio",
	.id = 1,
	.dev = {
		.platform_data = &i2c_gpio_touch_data,
	}
};
#endif /* CONFIG_TOUCH_I2C_GPIO */



#ifdef CONFIG_SENSOR_I2C_GPIO // shpark 100107 add
#define GPIO_PIN_SENSOR_SCL	90
#define GPIO_PIN_SENSOR_SDA	91

static struct i2c_gpio_platform_data i2c_gpio_sensor_data = {
	.scl_pin = GPIO_PIN_SENSOR_SCL,
	.sda_pin = GPIO_PIN_SENSOR_SDA,
	.udelay = 5,	/* 100 KHz */
};
struct platform_device msm_device_i2c_gpio_sensor = {
	.name = "i2c-gpio",
	.id = 2,
	.dev = {
		.platform_data = &i2c_gpio_sensor_data,
	}
};
#endif /* CONFIG_SENSOR_I2C_GPIO */
#ifdef CONFIG_ARCH_MSM7X27
#define MSM_PMEM_MDP_SIZE	0x1B76000
#define MSM_PMEM_ADSP_SIZE	0xB71000
#define MSM_PMEM_AUDIO_SIZE	0x5B000
#define MSM_FB_SIZE		0x177000
#define MSM_GPU_PHYS_SIZE	SZ_2M
#define PMEM_KERNEL_EBI1_SIZE	0x1C000
/* Using lower 1MB of OEMSBL memory for GPU_PHYS */
#define MSM_GPU_PHYS_START_ADDR	 0xD600000ul
#endif

#ifdef CONFIG_EF31S_32K_BOARD
/* Using upper 1/2MB of Apps Bootloader memory*/
#define MSM_PMEM_AUDIO_START_ADDR	0x80000ul
#define MSM_PMEM_TDMB_SIZE 0x100000  // 20091126 KKS : PV_TDMB_SURFACE
#endif

static struct resource smc91x_resources[] = {
	[0] = {
		.start	= 0x9C004300,
		.end	= 0x9C0043ff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= MSM_GPIO_TO_INT(132),
		.end	= MSM_GPIO_TO_INT(132),
		.flags	= IORESOURCE_IRQ,
	},
};

#ifdef CONFIG_USB_FUNCTION
static struct usb_mass_storage_platform_data usb_mass_storage_pdata = {
#if ((MODEL_ID == MODEL_EF31S) ||(MODEL_ID == MODEL_EF32K))
	.nluns          = 0x01,
#else
	.nluns          = 0x02,
#endif
	.buf_size       = 16384,
#ifdef FEATURE_ANDROID_PANTECH_USB_SKY
	.vendor         = "PANTECH",
#else
	.vendor         = "GOOGLE",
#endif
	.product        = "Mass storage",
	.release        = 0xffff,
};

static struct platform_device mass_storage_device = {
	.name           = "usb_mass_storage",
	.id             = -1,
	.dev            = {
		.platform_data          = &usb_mass_storage_pdata,
	},
};
#endif
#ifdef CONFIG_USB_ANDROID
#ifdef FEATURE_ANDROID_PANTECH_USB_SKY
static char *usb_functions_default[] = {
	"modem",
	"diag",
	"obex",
	"usb_mass_storage",
};

static char *usb_functions_default_adb[] = {
	"modem",
	"diag",
	"obex",
	"adb",
	"usb_mass_storage",
};

static char *usb_functions_rndis[] = {
	"rndis",
    	"diag",
};

static char *usb_functions_rndis_adb[] = {
	"rndis",
    	"diag",
	"adb",
};

static char *usb_functions_all[] = {
#ifdef CONFIG_USB_ANDROID_RNDIS
	"rndis",
#endif
#ifdef CONFIG_USB_F_SERIAL
	"modem",
#endif
#ifdef CONFIG_USB_ANDROID_DIAG
	"diag",
#endif
#ifdef CONFIG_USB_ANDROID_RMNET
	"rmnet",
#endif
    	"obex",
	"adb",
	"usb_mass_storage",
#ifdef CONFIG_USB_ANDROID_ACM
	"acm",
#endif
};

static struct android_usb_product usb_products[] = {
	{
		.product_id	= 0x1104,
		.num_functions	= ARRAY_SIZE(usb_functions_default),
		.functions	= usb_functions_default,
	},
	{
		.product_id	= 0x1104,
		.num_functions	= ARRAY_SIZE(usb_functions_default_adb),
		.functions	= usb_functions_default_adb,
	},
	{
		.product_id	= 0x1104,
		.num_functions	= ARRAY_SIZE(usb_functions_rndis),
		.functions	= usb_functions_rndis,
	},
	{
		.product_id	= 0x1104,
		.num_functions	= ARRAY_SIZE(usb_functions_rndis_adb),
		.functions	= usb_functions_rndis_adb,
	},
};

static struct usb_mass_storage_platform_data mass_storage_pdata = {
	.nluns		= 1,
	.vendor		= "Pantech Incorporated",
	.product        = "Mass storage",
	.release	= 0x0100,
};

static struct platform_device usb_mass_storage_device = {
	.name	= "usb_mass_storage",
	.id	= -1,
	.dev	= {
		.platform_data = &mass_storage_pdata,
	},
};

static struct usb_ether_platform_data rndis_pdata = {
	/* ethaddr is filled by board_serialno_setup */
	.vendorID	= 0x10A9,
	.vendorDescr	= "Pantech Incorporated",
};

static struct platform_device rndis_device = {
	.name	= "rndis",
	.id	= -1,
	.dev	= {
		.platform_data = &rndis_pdata,
	},
};

static struct android_usb_platform_data android_usb_pdata = {
	.vendor_id	= 0x10A9,
	.product_id	= 0x1104,
	.version	= 0x0100,
	.product_name		= "SKY(OBEX) USB MODEM",
	.manufacturer_name	= "Pantech",
	.num_products = ARRAY_SIZE(usb_products),
	.products = usb_products,
	.num_functions = ARRAY_SIZE(usb_functions_all),
	.functions = usb_functions_all,
	.serial_number = "1234567890ABCDEF",
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id		= -1,
	.dev		= {
		.platform_data = &android_usb_pdata,
	},
};

static int __init board_serialno_setup(char *serialno)
{
	int i;
	char *src = serialno;

	/* create a fake MAC address from our serial number.
	 * first byte is 0x02 to signify locally administered.
	 */
	rndis_pdata.ethaddr[0] = 0x02;
	for (i = 0; *src; i++) {
		/* XOR the USB serial across the remaining bytes */
		rndis_pdata.ethaddr[i % (ETH_ALEN - 1) + 1] ^= *src++;
	}

	android_usb_pdata.serial_number = serialno;
	return 1;
}
__setup("androidboot.serialno=", board_serialno_setup);

#else/*FEATURE_ANDROID_PANTECH_USB_SKY*/

static char *usb_functions_default[] = {
	"diag",
	"modem",
	"nmea",
	"rmnet",
	"usb_mass_storage",
};

static char *usb_functions_default_adb[] = {
	"diag",
	"adb",
	"modem",
	"nmea",
	"rmnet",
	"usb_mass_storage",
};

static char *usb_functions_rndis[] = {
	"rndis",
};

static char *usb_functions_rndis_adb[] = {
	"rndis",
	"adb",
};

static char *usb_functions_all[] = {
#ifdef CONFIG_USB_ANDROID_RNDIS
	"rndis",
#endif
#ifdef CONFIG_USB_ANDROID_DIAG
	"diag",
#endif
	"adb",
#ifdef CONFIG_USB_F_SERIAL
	"modem",
	"nmea",
#endif
#ifdef CONFIG_USB_ANDROID_RMNET
	"rmnet",
#endif
	"usb_mass_storage",
#ifdef CONFIG_USB_ANDROID_ACM
	"acm",
#endif
};

static struct android_usb_product usb_products[] = {
	{
		.product_id	= 0x9026,
		.num_functions	= ARRAY_SIZE(usb_functions_default),
		.functions	= usb_functions_default,
	},
	{
		.product_id	= 0x9025,
		.num_functions	= ARRAY_SIZE(usb_functions_default_adb),
		.functions	= usb_functions_default_adb,
	},
	{
		.product_id	= 0xf00e,
		.num_functions	= ARRAY_SIZE(usb_functions_rndis),
		.functions	= usb_functions_rndis,
	},
	{
		.product_id	= 0x9024,
		.num_functions	= ARRAY_SIZE(usb_functions_rndis_adb),
		.functions	= usb_functions_rndis_adb,
	},
};

static struct usb_mass_storage_platform_data mass_storage_pdata = {
	.nluns		= 1,
	.vendor		= "Qualcomm Incorporated",
	.product        = "Mass storage",
	.release	= 0x0100,
	.can_stall	= 1,
};

static struct platform_device usb_mass_storage_device = {
	.name	= "usb_mass_storage",
	.id	= -1,
	.dev	= {
		.platform_data = &mass_storage_pdata,
	},
};

static struct usb_ether_platform_data rndis_pdata = {
	/* ethaddr is filled by board_serialno_setup */
	.vendorID	= 0x05C6,
	.vendorDescr	= "Qualcomm Incorporated",
};

static struct platform_device rndis_device = {
	.name	= "rndis",
	.id	= -1,
	.dev	= {
		.platform_data = &rndis_pdata,
	},
};

static struct android_usb_platform_data android_usb_pdata = {
	.vendor_id	= 0x05C6,
	.product_id	= 0x9026,
	.version	= 0x0100,
	.product_name		= "Qualcomm HSUSB Device",
	.manufacturer_name	= "Qualcomm Incorporated",
	.num_products = ARRAY_SIZE(usb_products),
	.products = usb_products,
	.num_functions = ARRAY_SIZE(usb_functions_all),
	.functions = usb_functions_all,
	.serial_number = "1234567890ABCDEF",
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id		= -1,
	.dev		= {
		.platform_data = &android_usb_pdata,
	},
};

static int __init board_serialno_setup(char *serialno)
{
	int i;
	char *src = serialno;

	/* create a fake MAC address from our serial number.
	 * first byte is 0x02 to signify locally administered.
	 */
	rndis_pdata.ethaddr[0] = 0x02;
	for (i = 0; *src; i++) {
		/* XOR the USB serial across the remaining bytes */
		rndis_pdata.ethaddr[i % (ETH_ALEN - 1) + 1] ^= *src++;
	}

	android_usb_pdata.serial_number = serialno;
	return 1;
}
__setup("androidboot.serialno=", board_serialno_setup);
#endif/*FEATURE_ANDROID_PANTECH_USB_SKY*/
#endif

static struct platform_device smc91x_device = {
	.name		= "smc91x",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(smc91x_resources),
	.resource	= smc91x_resources,
};

#ifdef CONFIG_USB_FUNCTION
static struct usb_function_map usb_functions_map[] = {
	{"diag", 0},
	{"adb", 1},
	{"modem", 2},
	{"nmea", 3},
	{"mass_storage", 4},
	{"ethernet", 5},
	{"rmnet", 6},
};

/* dynamic composition */
static struct usb_composition usb_func_composition[] = {
	{
		.product_id         = 0x9012,
		.functions	    = 0x5, /* 0101 */
	},

	{
		.product_id         = 0x9013,
		.functions	    = 0x15, /* 10101 */
	},

	{
		.product_id         = 0x9014,
		.functions	    = 0x30, /* 110000 */
	},

	{
		.product_id         = 0x9016,
		.functions	    = 0xD, /* 01101 */
	},

	{
		.product_id         = 0x9017,
		.functions	    = 0x1D, /* 11101 */
	},

	{
		.product_id         = 0xF000,
		.functions	    = 0x10, /* 10000 */
	},

	{
		.product_id         = 0xF009,
		.functions	    = 0x20, /* 100000 */
	},

	{
		.product_id         = 0x9018,
		.functions	    = 0x1F, /* 011111 */
	},
#ifdef CONFIG_USB_FUNCTION_RMNET
	{
		.product_id         = 0x9021,
		/* DIAG + RMNET */
		.functions	    = 0x41,
	},
	{
		.product_id         = 0x9022,
		/* DIAG + ADB + RMNET */
		.functions	    = 0x43,
	},
#endif

};

static struct msm_hsusb_platform_data msm_hsusb_pdata = {
	.version	= 0x0100,
	.phy_info	= (USB_PHY_INTEGRATED | USB_PHY_MODEL_65NM),
	.vendor_id          = 0x5c6,
	.product_name       = "Qualcomm HSUSB Device",
	.serial_number      = "1234567890ABCDEF",
	.manufacturer_name  = "Qualcomm Incorporated",
	.compositions	= usb_func_composition,
	.num_compositions = ARRAY_SIZE(usb_func_composition),
	.function_map   = usb_functions_map,
	.num_functions	= ARRAY_SIZE(usb_functions_map),
	.config_gpio    = NULL,
};
#endif	// CONFIG_USB_FUNCTION

#ifdef CONFIG_USB_EHCI_MSM
static void msm_hsusb_vbus_power(unsigned phy_info, int on)
{
	if (on)
		msm_hsusb_vbus_powerup();
	else
		msm_hsusb_vbus_shutdown();
}

static struct msm_usb_host_platform_data msm_usb_host_pdata = {
	.phy_info       = (USB_PHY_INTEGRATED | USB_PHY_MODEL_65NM),
};

static void __init msm7x2x_init_host(void)
{
	if (machine_is_msm7x25_ffa() || machine_is_msm7x27_ffa())
		return;

	msm_add_host(0, &msm_usb_host_pdata);
}
#endif	// CONFIG_USB_EHCI_MSM

#ifdef CONFIG_USB_MSM_OTG_72K
static int hsusb_rpc_connect(int connect)
{
	if (connect)
		return msm_hsusb_rpc_connect();
	else
		return msm_hsusb_rpc_close();
}
#endif

#ifdef CONFIG_USB_MSM_OTG_72K
struct vreg *vreg_3p3;
static int msm_hsusb_ldo_init(int init)
{
	if (init) {
		/*
		 * PHY 3.3V analog domain(VDDA33) is powered up by
		 * an always enabled power supply (LP5900TL-3.3).
		 * USB VREG default source is VBUS line. Turning
		 * on USB VREG has a side effect on the USB suspend
		 * current. Hence USB VREG is explicitly turned
		 * off here.
		 */
		vreg_3p3 = vreg_get(NULL, "usb");
		if (IS_ERR(vreg_3p3))
			return PTR_ERR(vreg_3p3);
		vreg_enable(vreg_3p3);
		vreg_disable(vreg_3p3);
		vreg_put(vreg_3p3);
	}

	return 0;
}

static int msm_hsusb_pmic_notif_init(void (*callback)(int online), int init)
{
	int ret;

	if (init) {
		ret = msm_pm_app_rpc_init(callback);
	} else {
		msm_pm_app_rpc_deinit(callback);
		ret = 0;
	}
	return ret;
}

static int msm_otg_rpc_phy_reset(void __iomem *regs)
{
	return msm_hsusb_phy_reset();
}

static struct msm_otg_platform_data msm_otg_pdata = {
	.rpc_connect	= hsusb_rpc_connect,
	.pmic_vbus_notif_init         = msm_hsusb_pmic_notif_init,
	.chg_vbus_draw		 = hsusb_chg_vbus_draw,
	.chg_connected		 = hsusb_chg_connected,
	.chg_init		 = hsusb_chg_init,
#ifdef CONFIG_USB_EHCI_MSM
	.vbus_power = msm_hsusb_vbus_power,
#endif
	.ldo_init		= msm_hsusb_ldo_init,
	.pclk_required_during_lpm = 1,
	.pclk_src_name		= "ebi1_usb_clk",
};

#ifdef CONFIG_USB_GADGET
static struct msm_hsusb_gadget_platform_data msm_gadget_pdata;
#endif
#endif

#define SND(desc, num) { .name = #desc, .id = num }
static struct snd_endpoint snd_endpoints_list[] = {
	SND(HANDSET, 0),
	SND(MONO_HEADSET, 2),
	SND(HEADSET, 3),
	SND(SPEAKER, 6),
	SND(TTY_HEADSET, 8),
	SND(TTY_VCO, 9),
	SND(TTY_HCO, 10),
	SND(BT, 12),
	SND(IN_S_SADC_OUT_HANDSET, 16),
	SND(IN_S_SADC_OUT_SPEAKER_PHONE, 25),
	SND(CURRENT, 27),
};
#undef SND

static struct msm_snd_endpoints msm_device_snd_endpoints = {
	.endpoints = snd_endpoints_list,
	.num = sizeof(snd_endpoints_list) / sizeof(struct snd_endpoint)
};

static struct platform_device msm_device_snd = {
	.name = "msm_snd",
	.id = -1,
	.dev    = {
		.platform_data = &msm_device_snd_endpoints
	},
};

#define DEC0_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#ifdef CONFIG_ARCH_MSM7X25
#define DEC1_FORMAT ((1<<MSM_ADSP_CODEC_WAV)|(1<<MSM_ADSP_CODEC_ADPCM)| \
	(1<<MSM_ADSP_CODEC_YADPCM)|(1<<MSM_ADSP_CODEC_QCELP)| \
	(1<<MSM_ADSP_CODEC_MP3))
#define DEC2_FORMAT ((1<<MSM_ADSP_CODEC_WAV)|(1<<MSM_ADSP_CODEC_ADPCM)| \
	(1<<MSM_ADSP_CODEC_YADPCM)|(1<<MSM_ADSP_CODEC_QCELP)| \
	(1<<MSM_ADSP_CODEC_MP3))
#define DEC3_FORMAT 0
#define DEC4_FORMAT 0
#else
#define DEC1_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC2_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC3_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC4_FORMAT (1<<MSM_ADSP_CODEC_MIDI)
#endif

static unsigned int dec_concurrency_table[] = {
	/* Audio LP */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DMA)), 0,
	0, 0, 0,

	/* Concurrency 1 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	 /* Concurrency 2 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 3 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 4 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 5 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 6 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	0, 0, 0, 0,

	/* Concurrency 7 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),
};

#define DEC_INFO(name, queueid, decid, nr_codec) { .module_name = name, \
	.module_queueid = queueid, .module_decid = decid, \
	.nr_codec_support = nr_codec}

static struct msm_adspdec_info dec_info_list[] = {
	DEC_INFO("AUDPLAY0TASK", 13, 0, 11), /* AudPlay0BitStreamCtrlQueue */
#ifdef CONFIG_ARCH_MSM7X25
	DEC_INFO("AUDPLAY1TASK", 14, 1, 5),  /* AudPlay1BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY2TASK", 15, 2, 5),  /* AudPlay2BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY3TASK", 16, 3, 0),  /* AudPlay3BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY4TASK", 17, 4, 0),  /* AudPlay4BitStreamCtrlQueue */
#else
	DEC_INFO("AUDPLAY1TASK", 14, 1, 11),  /* AudPlay1BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY2TASK", 15, 2, 11),  /* AudPlay2BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY3TASK", 16, 3, 11),  /* AudPlay3BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY4TASK", 17, 4, 1),  /* AudPlay4BitStreamCtrlQueue */
#endif
};

static struct msm_adspdec_database msm_device_adspdec_database = {
	.num_dec = ARRAY_SIZE(dec_info_list),
	.num_concurrency_support = (ARRAY_SIZE(dec_concurrency_table) / \
					ARRAY_SIZE(dec_info_list)),
	.dec_concurrency_table = dec_concurrency_table,
	.dec_info_list = dec_info_list,
};

static struct platform_device msm_device_adspdec = {
	.name = "msm_adspdec",
	.id = -1,
	.dev    = {
		.platform_data = &msm_device_adspdec_database
	},
};

static struct android_pmem_platform_data android_pmem_kernel_ebi1_pdata = {
	.name = PMEM_KERNEL_EBI1_DATA_NAME,
	/* if no allocator_type, defaults to PMEM_ALLOCATORTYPE_BITMAP,
	 * the only valid choice at this time. The board structure is
	 * set to all zeros by the C runtime initialization and that is now
	 * the enum value of PMEM_ALLOCATORTYPE_BITMAP, now forced to 0 in
	 * include/linux/android_pmem.h.
	 */
	.cached = 0,
};

static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 1,
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
};

static struct android_pmem_platform_data android_pmem_audio_pdata = {
	.name = "pmem_audio",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
};
#if 1// 20091126 KKS : PV_TDMB_SURFACE
static struct android_pmem_platform_data android_pmem_tdmb_pdata = {
	.name = "pmem_tdmb",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
};
#endif

static struct platform_device android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &android_pmem_pdata },
};

static struct platform_device android_pmem_adsp_device = {
	.name = "android_pmem",
	.id = 1,
	.dev = { .platform_data = &android_pmem_adsp_pdata },
};

static struct platform_device android_pmem_audio_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &android_pmem_audio_pdata },
};

static struct platform_device android_pmem_kernel_ebi1_device = {
	.name = "android_pmem",
	.id = 4,
	.dev = { .platform_data = &android_pmem_kernel_ebi1_pdata },
};

static struct msm_handset_platform_data hs_platform_data = {
	.hs_name = "7k_handset",
	.pwr_key_delay_ms = 500, /* 0 will disable end key */
};
#if 1// 20091126 KKS : PV_TDMB_SURFACE
static struct platform_device android_pmem_tdmb_device = {
	.name = "android_pmem",
	.id = 5,
	.dev = { .platform_data = &android_pmem_tdmb_pdata },
};
#endif 

static struct platform_device hs_device = {
	.name   = "msm-handset",
	.id     = -1,
	.dev    = {
		.platform_data = &hs_platform_data,
	},
};

/* TSIF begin */
#if defined(CONFIG_TSIF) || defined(CONFIG_TSIF_MODULE)

#define TSIF_B_SYNC      GPIO_CFG(87, 5, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA)
#define TSIF_B_DATA      GPIO_CFG(86, 3, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA)
#define TSIF_B_EN        GPIO_CFG(85, 3, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA)
#define TSIF_B_CLK       GPIO_CFG(84, 4, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA)

static const struct msm_gpio tsif_gpios[] = {
	{ .gpio_cfg = TSIF_B_CLK,  .label =  "tsif_clk", },
	{ .gpio_cfg = TSIF_B_EN,   .label =  "tsif_en", },
	{ .gpio_cfg = TSIF_B_DATA, .label =  "tsif_data", },
	{ .gpio_cfg = TSIF_B_SYNC, .label =  "tsif_sync", },
};

static struct msm_tsif_platform_data tsif_platform_data = {
	.num_gpios = ARRAY_SIZE(tsif_gpios),
	.gpios = tsif_gpios,
	.tsif_clk = "tsif_clk",
	.tsif_pclk = "tsif_pclk",
	.tsif_ref_clk = "tsif_ref_clk",
};
#endif /* defined(CONFIG_TSIF) || defined(CONFIG_TSIF_MODULE) */
/* TSIF end   */

#define LCDC_CONFIG_PROC          21
#define LCDC_UN_CONFIG_PROC       22
#define LCDC_API_PROG             0x30000066
#define LCDC_API_VERS             0x00010001

#define GPIO_OUT_132    132
#define GPIO_OUT_131    131
#define GPIO_OUT_103    103
#define GPIO_OUT_102    102
#define GPIO_IN_102		102

//pantch_lks@PS3_20111206
#ifdef CONFIG_EF31S_32K_BOARD
#define GPIO_OUT_101    101
#define GPIO_OUT_82      82
#define GPIO_OUT_23      23
#else
#define GPIO_OUT_88      88
#endif // !CONFIG_EF31S_32K_BOARD

static struct msm_rpc_endpoint *lcdc_ep;

static int msm_fb_lcdc_config(int on)
{
	int rc = 0;
	struct rpc_request_hdr hdr;

	if (on)
		pr_info("lcdc config\n");
	else
		pr_info("lcdc un-config\n");

	lcdc_ep = msm_rpc_connect_compatible(LCDC_API_PROG, LCDC_API_VERS, 0);
	if (IS_ERR(lcdc_ep)) {
		printk(KERN_ERR "%s: msm_rpc_connect failed! rc = %ld\n",
			__func__, PTR_ERR(lcdc_ep));
		return -EINVAL;
	}

	rc = msm_rpc_call(lcdc_ep,
				(on) ? LCDC_CONFIG_PROC : LCDC_UN_CONFIG_PROC,
				&hdr, sizeof(hdr),
				5 * HZ);
	if (rc)
		printk(KERN_ERR
			"%s: msm_rpc_call failed! rc = %d\n", __func__, rc);

	msm_rpc_close(lcdc_ep);
	return rc;
}

static int gpio_array_num[] = {
				GPIO_OUT_132, /* spi_clk */
				GPIO_OUT_131, /* spi_cs  */
				GPIO_OUT_103, /* spi_sdi */
				GPIO_OUT_102, /* spi_sdoi */

#ifdef CONFIG_EF31S_32K_BOARD
				GPIO_OUT_101, /* lcdc_reset */
#else
				GPIO_OUT_88
#endif // !CONFIG_EF31S_32K_BOARD
				};

static void lcdc_gordon_gpio_init(void)
{
	if (gpio_request(GPIO_OUT_132, "spi_clk"))
		pr_err("failed to request gpio spi_clk\n");
	if (gpio_request(GPIO_OUT_131, "spi_cs"))
		pr_err("failed to request gpio spi_cs\n");
	if (gpio_request(GPIO_OUT_103, "spi_sdi"))
		pr_err("failed to request gpio spi_sdi\n");
	if (gpio_request(GPIO_OUT_102, "spi_sdoi"))
		pr_err("failed to request gpio spi_sdoi\n");
#ifdef CONFIG_EF31S_32K_BOARD
	if (gpio_request(GPIO_OUT_101, "lcdc_reset"))
		pr_err("failed to request gpio lcdc_reset\n");
	if (gpio_request(GPIO_OUT_82, "lcdc_backlight"))
		pr_err("failed to request gpio lcdc_backlight\n");    
#else
	if (gpio_request(GPIO_OUT_88, "gpio_dac"))
		pr_err("failed to request gpio_dac\n");
#endif // !CONFIG_EF31S_32K_BOARD
}

//pantch_lks@PS3_20101201 - prepare ef31s_ef32k
static void mddi_sky_gpio_init(void)
{
#ifdef CONFIG_EF31S_32K_BOARD
	printk(KERN_ERR "[MDDI_SKY_LCD] Enter %s\n", __FUNCTION__);

	if (gpio_request(GPIO_OUT_23, "mddi_reset"))
		pr_err("failed to request gpio mddi_reset\n");
	if (gpio_request(GPIO_OUT_82, "lcdc_backlight"))
		pr_err("failed to request gpio lcdc_backlight\n");    
	if (gpio_request(GPIO_OUT_101, "lcdc_reset"))
		pr_err("failed to request gpio lcdc_reset\n");	
#endif // !CONFIG_EF31S_32K_BOARD
}

static uint32_t lcdc_gpio_table[] = {
	GPIO_CFG(GPIO_OUT_132, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(GPIO_OUT_131, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(GPIO_OUT_103, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(GPIO_IN_102, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	// lived
#ifdef CONFIG_EF31S_32K_BOARD
	GPIO_CFG(GPIO_OUT_101, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	// lived - //pantch_lks@PS3_20101201 - prepare ef31s_ef32k
	GPIO_CFG(GPIO_OUT_82, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	// lived	
	GPIO_CFG(GPIO_OUT_23, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	// lived - //pantch_lks@PS3_20101201 - prepare ef31s_ef32k	
#else
	GPIO_CFG(GPIO_OUT_102, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(GPIO_OUT_88,  0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
#endif // !CONFIG_EF31S_32K_BOARD
};

static void config_lcdc_gpio_table(uint32_t *table, int len, unsigned enable)
{
	int n, rc;
	for (n = 0; n < len; n++) {
		rc = gpio_tlmm_config(table[n],
			enable ? GPIO_CFG_ENABLE : GPIO_CFG_DISABLE);
		if (rc) {
			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, table[n], rc);
			break;
		}
	}
}

static void lcdc_gordon_config_gpios(int enable)
{
#ifdef CONFIG_EF31S_32K_BOARD
	static int enabled = 0;

	if (enabled == enable)
		return;
	enabled = enable;
#endif
	config_lcdc_gpio_table(lcdc_gpio_table,
		ARRAY_SIZE(lcdc_gpio_table), enable);
}

static char *msm_fb_lcdc_vreg[] = {
#ifdef CONFIG_EF31S_32K_BOARD
	"gp2"
#else // !CONFIG_EF31S_32K_BOARD
	"gp5"
#endif // CONFIG_EF31S_32K_BOARD
};

static int msm_fb_lcdc_power_save(int on)
{
	struct vreg *vreg[ARRAY_SIZE(msm_fb_lcdc_vreg)];
	int i, rc = 0;

	for (i = 0; i < ARRAY_SIZE(msm_fb_lcdc_vreg); i++) {
		if (on) {
			vreg[i] = vreg_get(0, msm_fb_lcdc_vreg[i]);
			rc = vreg_enable(vreg[i]);
			if (rc) {
				printk(KERN_ERR "vreg_enable: %s vreg"
						"operation failed \n",
						msm_fb_lcdc_vreg[i]);
				goto bail;
			}
		} else {
			int tmp;
			vreg[i] = vreg_get(0, msm_fb_lcdc_vreg[i]);
			tmp = vreg_disable(vreg[i]);
			if (tmp) {
				printk(KERN_ERR "vreg_disable: %s vreg "
						"operation failed \n",
						msm_fb_lcdc_vreg[i]);
				if (!rc)
					rc = tmp;
			}
#ifndef CONFIG_EF31S_32K_BOARD			
			tmp = gpio_tlmm_config(GPIO_CFG(GPIO_OUT_88, 0,
						GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,
						GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			if (tmp) {
				printk(KERN_ERR "gpio_tlmm_config failed\n");
				if (!rc)
					rc = tmp;
			}
			gpio_set_value(88, 0);
			mdelay(15);
			gpio_set_value(88, 1);
			mdelay(15);
#endif // !CONFIG_EF31S_32K_BOARD	
		}
	}

	return rc;

bail:
	if (on) {
		for (; i > 0; i--)
			vreg_disable(vreg[i - 1]);
	}

	return rc;
}
static struct lcdc_platform_data lcdc_pdata = {
	.lcdc_gpio_config = msm_fb_lcdc_config,
	.lcdc_power_save   = msm_fb_lcdc_power_save,
};

static struct msm_panel_common_pdata lcdc_gordon_panel_data = {
	.panel_config_gpio = lcdc_gordon_config_gpios,
	.gpio_num          = gpio_array_num,
};

static struct platform_device lcdc_gordon_panel_device = {
	.name   = "lcdc_gordon_vga",
	.id     = 0,
	.dev    = {
		.platform_data = &lcdc_gordon_panel_data,
	}
};

static struct resource msm_fb_resources[] = {
	{
		.flags  = IORESOURCE_DMA,
	}
};

static int msm_fb_detect_panel(const char *name)
{
	int ret = -EPERM;

	if (machine_is_msm7x25_ffa() || machine_is_msm7x27_ffa()) {
		if (!strcmp(name, "lcdc_gordon_vga"))
			ret = 0;
		else
			ret = -ENODEV;
	}

	return ret;
}

static struct msm_fb_platform_data msm_fb_pdata = {
	.detect_client = msm_fb_detect_panel,
	.mddi_prescan = 1,
};

static struct platform_device msm_fb_device = {
	.name   = "msm_fb",
	.id     = 0,
	.num_resources  = ARRAY_SIZE(msm_fb_resources),
	.resource       = msm_fb_resources,
	.dev    = {
		.platform_data = &msm_fb_pdata,
	}
};

#ifdef CONFIG_SKY_TDMB_EBI_IF
static void __init msm_tdmb_add_devices(void)
{
  msm_tdmb_register_device(0);
}
#endif /* CONFIG_SKY_TDMB_EBI_IF */
#ifdef CONFIG_BT
#if (defined(CONFIG_SKY_BCM_BT)) || (defined(CONFIG_BAHAMA_CORE)) && \
	(defined(CONFIG_MSM_BT_POWER) || defined(CONFIG_MSM_BT_POWER_MODULE))

static int bluetooth_power(int);
static struct platform_device msm_bt_power_device = {
	.name = "bt_power",
#ifndef CONFIG_SKY_BCM_BT
	.id	 = -1,
	.dev	 = {
		.platform_data = &bluetooth_power,
	},
#endif //CONFIG_SKY_BCM_BT
};
#endif


enum {
	BT_WAKE,
	BT_RFR,
	BT_CTS,
	BT_RX,
	BT_TX,
	BT_PCM_DOUT,
	BT_PCM_DIN,
	BT_PCM_SYNC,
	BT_PCM_CLK,
	BT_HOST_WAKE,
#ifdef CONFIG_SKY_BLUETOOTH
   BT_SYSRST,
#endif

};

static unsigned bt_config_power_on[] = {
	GPIO_CFG(42, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* WAKE */
	GPIO_CFG(43, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* RFR */
	GPIO_CFG(44, 2, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* CTS */
	GPIO_CFG(45, 2, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* Rx */
	GPIO_CFG(46, 3, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* Tx */
	GPIO_CFG(68, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* PCM_DOUT */
	GPIO_CFG(69, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* PCM_DIN */
	GPIO_CFG(70, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* PCM_SYNC */
	GPIO_CFG(71, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* PCM_CLK */
	GPIO_CFG(83, 0, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* HOST_WAKE */
#ifdef CONFIG_SKY_BLUETOOTH
	GPIO_CFG(110,0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* BT_SYSRST *///kychoi 101209 EF31S
#endif
};
static unsigned bt_config_power_off[] = {
	GPIO_CFG(42, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* WAKE */
	GPIO_CFG(43, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* RFR */
	GPIO_CFG(44, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* CTS */
	GPIO_CFG(45, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* Rx */
	GPIO_CFG(46, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* Tx */
	GPIO_CFG(68, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* PCM_DOUT */
	GPIO_CFG(69, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* PCM_DIN */
	GPIO_CFG(70, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* PCM_SYNC */
	GPIO_CFG(71, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* PCM_CLK */
	GPIO_CFG(83, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* HOST_WAKE */
#ifdef CONFIG_SKY_BLUETOOTH
	GPIO_CFG(110, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* BT_SYSRST *///kychoi 101209 EF31S
#endif
};

static int bluetooth_power(int on)
{
    int rc = 0;
    int pin = 0;

    printk("%s: %d\n",__func__, on);

	if (on) {
		for (pin = 0; pin < ARRAY_SIZE(bt_config_power_on); pin++) {
        rc = gpio_tlmm_config(bt_config_power_on[pin], GPIO_CFG_ENABLE);
			if (rc) {
          printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, bt_config_power_on[pin], rc);
				return -EIO;
			}
		}
      gpio_set_value(110, on); /* BT_SYSRST */
       mdelay(10);
      gpio_set_value(110, 0); /* BT_SYSRST */
       mdelay(10);
      gpio_set_value(110, on); /* BT_SYSRST */
	} else {
      gpio_set_value(110, on); /* BT_SYSRST */

		for (pin = 0; pin < ARRAY_SIZE(bt_config_power_off); pin++) {
        rc = gpio_tlmm_config(bt_config_power_off[pin], GPIO_CFG_ENABLE);
			if (rc) {
          printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, bt_config_power_off[pin], rc);
				return -EIO;
			}
		}
    
      /* check for initial RFKILL block (power off) */
      /* some RFKILL versions/configurations rfkill_register */
      /* calls here for an initial set_block */
      /* avoid calling i2c and regulator before unblock (on) */
      if (platform_get_drvdata(&msm_bt_power_device) == NULL) {
        dev_info(&msm_bt_power_device.dev,
          "%s: initialized OFF/blocked\n", __func__);
        goto out;
      }
	}

  out:
    if (rc < 0)
      on = 0;
    dev_info(&msm_bt_power_device.dev,
      "Bluetooth power switch: state %d result %d\n", on, rc);

    return rc;
  }


static void __init bt_power_init(void)
  {
    int rc = 0;
    int pin = 0;
  
		for (pin = 0; pin < ARRAY_SIZE(bt_config_power_off); pin++) {
      rc = gpio_tlmm_config(bt_config_power_off[pin], GPIO_CFG_ENABLE);
			if (rc) {
        printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, bt_config_power_off[pin], rc);
				return -EIO;
			}
		}

	msm_bt_power_device.dev.platform_data = &bluetooth_power;
}
#else
#define bt_power_init(x) do {} while (0)
#endif
#ifdef CONFIG_EF13_BOARD
static int nIsLedOn;

void alsps_power_ctrl(int on_off)
{
	struct vreg* vreg_alsps = vreg_get(NULL, "gp3");
	int rc=0;

	if (IS_ERR(vreg_alsps))
	{
		printk(KERN_ERR "[als-ps] Failed to vreg_get\n");
		return;
	}

	if(on_off)
	{
	    if(!nIsLedOn)
	    {
			if (!vreg_set_level(vreg_alsps, 3000))
			{
				rc = vreg_enable(vreg_alsps);
				printk(KERN_ERR "[als-ps] Set gp3 as 3.0v %d\n",rc);
	            nIsLedOn = 1;
			}
		}
	}
	else
	{
	    rc = vreg_disable(vreg_alsps);
	    printk(KERN_ERR "[als-ps] gp3 off %d\n",rc);
		nIsLedOn = 0;
	}
}

/* 2010.01.25 JW Jeon */
static void __init alsps_power_init(void)
{
    alsps_power_ctrl(1);
}
#endif

#ifdef CONFIG_ARCH_MSM7X27
static struct resource kgsl_resources[] = {
	{
		.name = "kgsl_reg_memory",
		.start = 0xA0000000,
		.end = 0xA001ffff,
		.flags = IORESOURCE_MEM,
	},
	{
		.name = "kgsl_yamato_irq",
		.start = INT_GRAPHICS,
		.end = INT_GRAPHICS,
		.flags = IORESOURCE_IRQ,
	},
};

static struct kgsl_platform_data kgsl_pdata;

static struct platform_device msm_device_kgsl = {
	.name = "kgsl",
	.id = -1,
	.num_resources = ARRAY_SIZE(kgsl_resources),
	.resource = kgsl_resources,
	.dev = {
		.platform_data = &kgsl_pdata,
	},
};
#endif

static struct platform_device msm_device_pmic_leds = {
	.name   = "pmic-leds",
	.id = -1,
};

static struct resource bluesleep_resources[] = {
	{
		.name	= "gpio_host_wake",
		.start	= 83,
		.end	= 83,
		.flags	= IORESOURCE_IO,
	},
	{
		.name	= "gpio_ext_wake",
		.start	= 42,
		.end	= 42,
		.flags	= IORESOURCE_IO,
	},
	{
		.name	= "host_wake",
		.start	= MSM_GPIO_TO_INT(83),
		.end	= MSM_GPIO_TO_INT(83),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device msm_bluesleep_device = {
	.name = "bluesleep",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(bluesleep_resources),
	.resource	= bluesleep_resources,
};



#ifdef CONFIG_TOUCH_I2C_GPIO
static struct i2c_board_info i2c_touch_devices[] __initdata = {
	{
		I2C_BOARD_INFO("cy8c20234", 0x11)
	},
	{
        	I2C_BOARD_INFO("cy8ctma300", 0x21), 
    	},
};
#endif /*CONFIG_TOUCH_I2C_GPIO*/

static struct i2c_board_info i2c_devices[] = {
#ifndef F_SKYCAM_I2C_GPIO
#ifdef CONFIG_MT9D112
	{
		I2C_BOARD_INFO("mt9d112", 0x78 >> 1),
	},
#endif
#ifdef CONFIG_S5K3E2FX
	{
		I2C_BOARD_INFO("s5k3e2fx", 0x20 >> 1),
	},
#endif
#ifdef CONFIG_MT9P012
	{
		I2C_BOARD_INFO("mt9p012", 0x6C >> 1),
	},
#endif
#ifdef CONFIG_MT9P012_KM
	{
		I2C_BOARD_INFO("mt9p012_km", 0x6C >> 2),
	},
#endif
#if defined(CONFIG_MT9T013) || defined(CONFIG_SENSORS_MT9T013)
	{
		I2C_BOARD_INFO("mt9t013", 0x6C),
	},
#endif
#ifdef CONFIG_VB6801
	{
		I2C_BOARD_INFO("vb6801", 0x20),
	},
#endif
#endif
#ifdef CONFIG_TOUCHSCREEN_QT602240
	{
		I2C_BOARD_INFO("qt602240-i2c", 0x4A),
	},
#endif
//#ifdef CONFIG_TOUCHSCREEN_CY8C_TS
#ifdef CONFIG_TOUCHSCREEN_CYTTSP_I2C
	{
		I2C_BOARD_INFO(CYTTSP_I2C_NAME, 0x20),
		.platform_data = &cyttsp_data,
#if 1//ndef CYTTSP_USE_TIMER
//		.irq = TROUT_GPIO_TO_INT(TROUT_GPIO_TP_ATT_N)
		.irq = MSM_GPIO_TO_INT(81)
#endif /* CYTTSP_USE_TIMER */
	},
#endif	


#ifdef CONFIG_SKY_TDMB_I2C_HW
  {
#if defined(CONFIG_SKY_TDMB_INC_BB)
		I2C_BOARD_INFO("tdmb_i2c", 0x80 >> 1),
#elif defined(CONFIG_SKY_TDMB_FCI_BB)
		I2C_BOARD_INFO("tdmb_i2c", 0xB0 >> 1),
#elif defined(CONFIG_SKY_TDMB_RTV_BB)
		I2C_BOARD_INFO("tdmb_i2c", 0x86 >> 1),
#else
		#error
#endif
  },
#endif /* CONFIG_SKY_TDMB_I2C_HW */
};

#ifdef CONFIG_SKY_TDMB_I2C_GPIO
static struct i2c_board_info i2c_tdmb_devices[] __initdata = {
	{
#if defined(CONFIG_SKY_TDMB_INC_BB)
		I2C_BOARD_INFO("tdmb_i2c", 0x80 >> 1),
#elif defined(CONFIG_SKY_TDMB_FCI_BB)
		I2C_BOARD_INFO("tdmb_i2c", 0xB0 >> 1),
#elif defined(CONFIG_SKY_TDMB_RTV_BB)
		I2C_BOARD_INFO("tdmb_i2c", 0x86 >> 1),
#else
		#error
#endif
	},
};
#endif /* CONFIG_SKY_TDMB_I2C_GPIO */

#ifdef F_SKYCAM_I2C_GPIO
static struct i2c_board_info msm_i2c_gpio_board_info[] __initdata = {
#ifdef F_SKYCAM_SENSOR_MT9P111
	{
		I2C_BOARD_INFO("mt9p111", 0x7a >> 1),
	},
#endif
#ifdef F_SKYCAM_SENSOR_MT9T111
	{
		I2C_BOARD_INFO("mt9t111", 0x7a >> 1),
	},
#endif
#ifdef F_SKYCAM_SENSOR_YACBAC1SDDAS	
	{
		I2C_BOARD_INFO("yacbac1sddas", 0x60 >> 1),
	},
#endif	
};
#endif
#if ((BOARD_VER >=EF31S_PT10) ||(BOARD_VER >=EF32K_PT10)) 
#ifdef CONFIG_SKY_SND_EXTAMP
#ifndef CONFIG_SKY_SND_MAX8981	//97001 -> 8981 amp test 101214
#ifdef CONFIG_SKY_SND_MAX97001
static struct i2c_board_info i2c_subsystem_devices[] __initdata = {
	{
		I2C_BOARD_INFO("max97001-amp", MAX97001_SLAVE_ADDR),
	},
};
#else
#err
static struct i2c_board_info i2c_subsystem_devices[] __initdata = {
	{
		I2C_BOARD_INFO("tpa2051d3-amp", TPA2051D3_SLAVE_ADDR),
	},
};
#endif	// CONFIG_SKY_SND_MAX97001
#endif	// CONFIG_SKY_SND_MAX8981
#endif	// CONFIG_SKY_SND_EXTAMP
#endif	/*	#if ((BOARD_VER >=EF31S_PT10) ||(BOARD_VER >=EF32K_PT10)) 	*/
#ifdef CONFIG_SENSOR_I2C_GPIO
static struct i2c_board_info i2c_sensor_device[] __initdata = {
#ifdef CONFIG_INPUT_BMA150 //CONFIG_BOSCH_SMB380
	{
		I2C_BOARD_INFO("bma150", 0x38),		// accelerometer sensor
	},
#endif

#if defined (CONFIG_SENSORS_AK8975)
	{
		I2C_BOARD_INFO("akm8975", 0x0c),		// magneticfield sensor
	},
#elif defined  CONFIG_INPUT_YAS529
	{
		I2C_BOARD_INFO("yas529", 0x2e),		// magneticfield sensor
	},
#endif	
#ifdef CONFIG_INPUT_APDS9900
	{
		I2C_BOARD_INFO("apds9900", 0x39),
	},
#elif defined CONFIG_INPUT_GP2A
	{
		I2C_BOARD_INFO("gp2ap002s00f", 0x44),	// proximity sensor
	},	
#endif
};
#endif
#ifdef CONFIG_MSM_CAMERA
#if 1 //SKYCAM 
static uint32_t camera_off_gpio_table[] = {
	/* parallel CAMERA interfaces */
	GPIO_CFG(4,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT4 */
	GPIO_CFG(5,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT5 */
	GPIO_CFG(6,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT6 */
	GPIO_CFG(7,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT7 */
	GPIO_CFG(8,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT8 */
	GPIO_CFG(9,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT9 */
	GPIO_CFG(10, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT10 */
	GPIO_CFG(11, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT11 */
	GPIO_CFG(12, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* PCLK */
	GPIO_CFG(13, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* HSYNC_IN */
	GPIO_CFG(14, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* VSYNC_IN */
	GPIO_CFG(15, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), /* MCLK */
};

static uint32_t camera_on_gpio_table[] = {
   /* parallel CAMERA interfaces */
	GPIO_CFG(4,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT4 */
	GPIO_CFG(5,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT5 */
	GPIO_CFG(6,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT6 */
	GPIO_CFG(7,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT7 */
	GPIO_CFG(8,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT8 */
	GPIO_CFG(9,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT9 */
	GPIO_CFG(10, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT10 */
	GPIO_CFG(11, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT11 */
   GPIO_CFG(12, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* CAM_PCLK */
   GPIO_CFG(13, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* CAM_HSYNC */
   GPIO_CFG(14, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* CAM_VSYNC */
   GPIO_CFG(15, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), /* CAM_MCLK */
};
#else
	GPIO_CFG(0,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT0 */
	GPIO_CFG(1,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT1 */
	GPIO_CFG(2,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT2 */
	GPIO_CFG(3,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT3 */
	GPIO_CFG(4,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT4 */
	GPIO_CFG(5,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT5 */
	GPIO_CFG(6,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT6 */
	GPIO_CFG(7,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT7 */
	GPIO_CFG(8,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT8 */
	GPIO_CFG(9,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT9 */
	GPIO_CFG(10, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT10 */
	GPIO_CFG(11, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT11 */
	GPIO_CFG(12, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* PCLK */
	GPIO_CFG(13, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* HSYNC_IN */
	GPIO_CFG(14, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* VSYNC_IN */
	GPIO_CFG(15, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), /* MCLK */
};

static uint32_t camera_on_gpio_table[] = {
	/* parallel CAMERA interfaces */
	GPIO_CFG(0,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT0 */
	GPIO_CFG(1,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT1 */
	GPIO_CFG(2,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT2 */
	GPIO_CFG(3,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT3 */
	GPIO_CFG(4,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT4 */
	GPIO_CFG(5,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT5 */
	GPIO_CFG(6,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT6 */
	GPIO_CFG(7,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT7 */
	GPIO_CFG(8,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT8 */
	GPIO_CFG(9,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT9 */
	GPIO_CFG(10, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT10 */
	GPIO_CFG(11, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT11 */
	GPIO_CFG(12, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_16MA), /* PCLK */
	GPIO_CFG(13, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* HSYNC_IN */
	GPIO_CFG(14, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* VSYNC_IN */
	GPIO_CFG(15, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_16MA), /* MCLK */
	};
#endif	//SKYCAM

static void config_gpio_table(uint32_t *table, int len)
{
	int n, rc;
	for (n = 0; n < len; n++) {
		rc = gpio_tlmm_config(table[n], GPIO_CFG_ENABLE);
		if (rc) {
			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, table[n], rc);
			break;
		}
	}
}

static struct vreg *vreg_gp2;
static struct vreg *vreg_gp3;

static void msm_camera_vreg_config(int vreg_en)
{
	int rc;

	if (vreg_gp2 == NULL) {
		vreg_gp2 = vreg_get(NULL, "gp2");
		if (IS_ERR(vreg_gp2)) {
			printk(KERN_ERR "%s: vreg_get(%s) failed (%ld)\n",
				__func__, "gp2", PTR_ERR(vreg_gp2));
			return;
		}

		rc = vreg_set_level(vreg_gp2, 1800);
		if (rc) {
			printk(KERN_ERR "%s: GP2 set_level failed (%d)\n",
				__func__, rc);
		}
	}

	if (vreg_gp3 == NULL) {
		vreg_gp3 = vreg_get(NULL, "gp3");
		if (IS_ERR(vreg_gp3)) {
			printk(KERN_ERR "%s: vreg_get(%s) failed (%ld)\n",
				__func__, "gp3", PTR_ERR(vreg_gp3));
			return;
		}

		rc = vreg_set_level(vreg_gp3, 2850);
		if (rc) {
			printk(KERN_ERR "%s: GP3 set level failed (%d)\n",
				__func__, rc);
		}
	}

	if (vreg_en) {
		rc = vreg_enable(vreg_gp2);
		if (rc) {
			printk(KERN_ERR "%s: GP2 enable failed (%d)\n",
				 __func__, rc);
		}

		rc = vreg_enable(vreg_gp3);
		if (rc) {
			printk(KERN_ERR "%s: GP3 enable failed (%d)\n",
				__func__, rc);
		}
	} else {
		rc = vreg_disable(vreg_gp2);
		if (rc) {
			printk(KERN_ERR "%s: GP2 disable failed (%d)\n",
				 __func__, rc);
		}

		rc = vreg_disable(vreg_gp3);
		if (rc) {
			printk(KERN_ERR "%s: GP3 disable failed (%d)\n",
				__func__, rc);
		}
	}
}

static int config_camera_on_gpios(void)
{
	int vreg_en = 1;

	if (machine_is_msm7x25_ffa() ||
	    machine_is_msm7x27_ffa())
		msm_camera_vreg_config(vreg_en);

	config_gpio_table(camera_on_gpio_table,
		ARRAY_SIZE(camera_on_gpio_table));
	return 0;
}

static void config_camera_off_gpios(void)
{
	int vreg_en = 0;

	if (machine_is_msm7x25_ffa() ||
	    machine_is_msm7x27_ffa())
		msm_camera_vreg_config(vreg_en);

	config_gpio_table(camera_off_gpio_table,
		ARRAY_SIZE(camera_off_gpio_table));
}

static struct msm_camera_device_platform_data msm_camera_device_data = {
	.camera_gpio_on  = config_camera_on_gpios,
	.camera_gpio_off = config_camera_off_gpios,
	.ioext.mdcphy = MSM_MDC_PHYS,
	.ioext.mdcsz  = MSM_MDC_SIZE,
	.ioext.appphy = MSM_CLK_CTL_PHYS,
	.ioext.appsz  = MSM_CLK_CTL_SIZE,
};

int pmic_set_flash_led_current(enum pmic8058_leds id, unsigned mA)
{
	int rc;
	rc = pmic_flash_led_set_current(mA);
	return rc;
}

static struct msm_camera_sensor_flash_src msm_flash_src = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_PMIC,
	._fsrc.pmic_src.num_of_src = 1,
	._fsrc.pmic_src.low_current  = 30,
	._fsrc.pmic_src.high_current = 100,
	._fsrc.pmic_src.led_src_1 = 0,
	._fsrc.pmic_src.led_src_2 = 0,
	._fsrc.pmic_src.pmic_set_current = pmic_set_flash_led_current,
};

#ifdef CONFIG_MT9D112
static struct msm_camera_sensor_flash_data flash_mt9d112 = {
	.flash_type = MSM_CAMERA_FLASH_LED,
	.flash_src  = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9d112_data = {
	.sensor_name    = "mt9d112",
	.sensor_reset   = 89,
	.sensor_pwd     = 85,
	.vcm_pwd        = 0,
	.vcm_enable     = 0,
	.pdata          = &msm_camera_device_data,
	.flash_data     = &flash_mt9d112
};

static struct platform_device msm_camera_sensor_mt9d112 = {
	.name      = "msm_camera_mt9d112",
	.dev       = {
		.platform_data = &msm_camera_sensor_mt9d112_data,
	},
};
#endif

#ifdef CONFIG_S5K3E2FX
static struct msm_camera_sensor_flash_data flash_s5k3e2fx = {
	.flash_type = MSM_CAMERA_FLASH_LED,
	.flash_src  = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_s5k3e2fx_data = {
	.sensor_name    = "s5k3e2fx",
	.sensor_reset   = 89,
	.sensor_pwd     = 85,
	.vcm_pwd        = 0,
	.vcm_enable     = 0,
	.pdata          = &msm_camera_device_data,
	.flash_data     = &flash_s5k3e2fx
};

static struct platform_device msm_camera_sensor_s5k3e2fx = {
	.name      = "msm_camera_s5k3e2fx",
	.dev       = {
		.platform_data = &msm_camera_sensor_s5k3e2fx_data,
	},
};
#endif

#ifdef CONFIG_MT9P012
static struct msm_camera_sensor_flash_data flash_mt9p012 = {
	.flash_type = MSM_CAMERA_FLASH_LED,
	.flash_src  = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9p012_data = {
	.sensor_name    = "mt9p012",
	.sensor_reset   = 89,
	.sensor_pwd     = 85,
	.vcm_pwd        = 88,
	.vcm_enable     = 0,
	.pdata          = &msm_camera_device_data,
	.flash_data     = &flash_mt9p012
};

static struct platform_device msm_camera_sensor_mt9p012 = {
	.name      = "msm_camera_mt9p012",
	.dev       = {
		.platform_data = &msm_camera_sensor_mt9p012_data,
	},
};
#endif

#ifdef CONFIG_MT9P012_KM
static struct msm_camera_sensor_flash_data flash_mt9p012_km = {
	.flash_type = MSM_CAMERA_FLASH_LED,
	.flash_src  = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9p012_km_data = {
	.sensor_name    = "mt9p012_km",
	.sensor_reset   = 89,
	.sensor_pwd     = 85,
	.vcm_pwd        = 88,
	.vcm_enable     = 0,
	.pdata          = &msm_camera_device_data,
	.flash_data     = &flash_mt9p012_km
};

static struct platform_device msm_camera_sensor_mt9p012_km = {
	.name      = "msm_camera_mt9p012_km",
	.dev       = {
		.platform_data = &msm_camera_sensor_mt9p012_km_data,
	},
};
#endif

#ifdef CONFIG_MT9T013
static struct msm_camera_sensor_flash_data flash_mt9t013 = {
	.flash_type = MSM_CAMERA_FLASH_LED,
	.flash_src  = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9t013_data = {
	.sensor_name    = "mt9t013",
	.sensor_reset   = 89,
	.sensor_pwd     = 85,
	.vcm_pwd        = 0,
	.vcm_enable     = 0,
	.pdata          = &msm_camera_device_data,
	.flash_data     = &flash_mt9t013
};

static struct platform_device msm_camera_sensor_mt9t013 = {
	.name      = "msm_camera_mt9t013",
	.dev       = {
		.platform_data = &msm_camera_sensor_mt9t013_data,
	},
};
#endif

#ifdef CONFIG_VB6801
static struct msm_camera_sensor_flash_data flash_vb6801 = {
	.flash_type = MSM_CAMERA_FLASH_LED,
	.flash_src  = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_vb6801_data = {
	.sensor_name    = "vb6801",
	.sensor_reset   = 89,
	.sensor_pwd     = 88,
	.vcm_pwd        = 0,
	.vcm_enable     = 0,
	.pdata          = &msm_camera_device_data,
	.flash_data     = &flash_vb6801
};

static struct platform_device msm_camera_sensor_vb6801 = {
	.name      = "msm_camera_vb6801",
	.dev       = {
		.platform_data = &msm_camera_sensor_vb6801_data,
	},
};
#endif

#ifdef F_SKYCAM_SENSOR_MV9335
static struct msm_camera_sensor_flash_data flash_mv9335 = {
	.flash_type = MSM_CAMERA_FLASH_NONE,
	.flash_src  = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_mv9335_data = {
	.sensor_name    = "mv9335",
	.sensor_reset   = 86,//116,//89,
	.sensor_pwd     = 0,//85,
	.vcm_pwd        = 0,
	.pdata          = &msm_camera_device_data,
	.flash_data     = &flash_mv9335
};

static struct platform_device msm_camera_sensor_mv9335 = {
	.name      = "msm_camera_mv9335",
	.dev       = {
		.platform_data = &msm_camera_sensor_mv9335_data,
	},
};
#endif	// F_SKYCAM_SENSOR_MV9335

#ifdef F_SKYCAM_SENSOR_MT9P111
static struct msm_camera_sensor_flash_data flash_mt9p111 = {
	.flash_type = MSM_CAMERA_FLASH_LED,
	.flash_src  = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9p111_data = {
	.sensor_name    = "mt9p111",
	.sensor_reset   = 2,
	.sensor_pwd     = 0,//23, //pantch_lks@PS3_20111206- prepare ef31s_ef32k 	
	.vcm_pwd        = 0,//27,
       .vcm_enable     = 0,
	.pdata          = &msm_camera_device_data,
	.flash_data     = &flash_mt9p111
};

static struct platform_device msm_camera_sensor_mt9p111 = {
	.name      = "msm_camera_mt9p111",
	.dev       = {
		.platform_data = &msm_camera_sensor_mt9p111_data,
	},
};
#endif	// F_SKYCAM_SENSOR_MT9P111
#ifdef F_SKYCAM_SENSOR_MT9T111
static struct msm_camera_sensor_flash_data flash_mt9t111 = {
//	.flash_type = MSM_CAMERA_FLASH_LED,
	.flash_type = MSM_CAMERA_FLASH_NONE,
	.flash_src  = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9t111_data = {
	.sensor_name    = "mt9t111",
	.sensor_reset   = 2,
	.sensor_pwd     = 3,
	.vcm_pwd        = 0,
       .vcm_enable     = 0,
	.pdata          = &msm_camera_device_data,
	.flash_data     = &flash_mt9t111
};

static struct platform_device msm_camera_sensor_mt9t111 = {
	.name      = "msm_camera_mt9t111",
	.dev       = {
		.platform_data = &msm_camera_sensor_mt9t111_data,
	},
};
#endif	// F_SKYCAM_SENSOR_MT9T111
#ifdef F_SKYCAM_SENSOR_YACBAC1SDDAS
static struct msm_camera_sensor_flash_data flash_yacbac1sddas = {
	.flash_type = MSM_CAMERA_FLASH_NONE,
	.flash_src  = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_yacbac1sddas_data = {
	.sensor_name    = "yacbac1sddas",
#ifdef F_SKYCAM_TARGET_EF31
	.sensor_reset   = 17,
	.sensor_pwd     = 96,
#else  // F_SKYCAM_TARGET_EF13
	.sensor_reset   = 2,
	.sensor_pwd     = 27,
#endif
	.vcm_pwd        = 0,//23,
	.pdata          = &msm_camera_device_data,
	.flash_data     = &flash_yacbac1sddas
};

static struct platform_device msm_camera_sensor_yacbac1sddas = {
	.name      = "msm_camera_yacbac1sddas",
	.dev       = {
		.platform_data = &msm_camera_sensor_yacbac1sddas_data,
	},
};
#endif	// F_SKYCAM_SENSOR_YACBAC1SDDAS
#endif	// CONFIG_MSM_CAMERA

static u32 msm_calculate_batt_capacity(u32 current_voltage);

static struct msm_psy_batt_pdata msm_psy_batt_data = {
#ifdef CONFIG_EF31S_32K_BOARD
	.voltage_min_design 	= 3400,
	.voltage_max_design	= 4200,
#else
	.voltage_min_design 	= 2800,
	.voltage_max_design	= 4300,
#endif	
	.avail_chg_sources   	= AC_CHG | USB_CHG ,
	.batt_technology        = POWER_SUPPLY_TECHNOLOGY_LION,
	.calculate_capacity	= &msm_calculate_batt_capacity,
};

static u32 msm_calculate_batt_capacity(u32 current_voltage)
{
#ifndef CONFIG_EF31S_32K_BOARD
/*	Original Code 	*/
	u32 low_voltage   = msm_psy_batt_data.voltage_min_design;
	u32 high_voltage  = msm_psy_batt_data.voltage_max_design;

	return (current_voltage - low_voltage) * 100
		/ (high_voltage - low_voltage);
#else
/*	For FuelGauge . by dscheon 	*/

	// If the level is more than 100%, the battery status bar is not displayed.
	if( current_voltage > 1000 )
		return 100;

/// Copy from 13L
#if 0
	// If the level is more than 0.4%, Not turn off power.
	if( (current_voltage <10) && (current_voltage > 4) )
		return 1;

	// Compensate the level, if more than 90%, because the level is 97% at full charged by charger.
	if(current_voltage > 900 )
	{
		u32 result;
		// y = 10/7 * x - 38.571
		result = (current_voltage/7) - 38;
		return (result > 100) ? 100 : result;
	}
#endif /// 0
	return (current_voltage /10);
#endif
}

static struct platform_device msm_batt_device = {
	.name 		    = "msm-battery",
	.id		    = -1,
	.dev.platform_data  = &msm_psy_batt_data,
};

#ifdef  CONFIG_EF31S_32K_BOARD
// added by jwcha100113
static struct platform_device pantech_backlight_device = {
    .name   = "pantech-bl",
    .id     = -1,
};

// added by jwcha091105
static struct platform_device pantech_debug_device = {
	.name = "pantech-debug",
	.id   = -1,
};

#endif	// CONFIG_EF31S_32K_BOARD

//	'10.10.04 added by hkkwon@DS2
#ifdef CONFIG_WIFI_MEM_PREALLOC //kychoi 110113  feature º¯°æ
#define PREALLOC_WLAN_NUMBER_OF_SECTIONS	4
#define PREALLOC_WLAN_NUMBER_OF_BUFFERS		160
#define PREALLOC_WLAN_SECTION_HEADER		24

#define WLAN_SECTION_SIZE_0	(PREALLOC_WLAN_NUMBER_OF_BUFFERS * 128)
#define WLAN_SECTION_SIZE_1	(PREALLOC_WLAN_NUMBER_OF_BUFFERS * 128)
#define WLAN_SECTION_SIZE_2	(PREALLOC_WLAN_NUMBER_OF_BUFFERS * 512)
#define WLAN_SECTION_SIZE_3	(PREALLOC_WLAN_NUMBER_OF_BUFFERS * 1024)

#define WLAN_SKB_BUF_NUM	16

static struct sk_buff *wlan_static_skb[WLAN_SKB_BUF_NUM];

typedef struct wifi_mem_prealloc_struct {
	void *mem_ptr;
	unsigned long size;
} wifi_mem_prealloc_t;

static wifi_mem_prealloc_t wifi_mem_array[PREALLOC_WLAN_NUMBER_OF_SECTIONS] = {
	{ NULL, (WLAN_SECTION_SIZE_0 + PREALLOC_WLAN_SECTION_HEADER) },
	{ NULL, (WLAN_SECTION_SIZE_1 + PREALLOC_WLAN_SECTION_HEADER) },
	{ NULL, (WLAN_SECTION_SIZE_2 + PREALLOC_WLAN_SECTION_HEADER) },
	{ NULL, (WLAN_SECTION_SIZE_3 + PREALLOC_WLAN_SECTION_HEADER) }
};

void *wifi_mem_prealloc(int section, unsigned long size)
{
	if (section == PREALLOC_WLAN_NUMBER_OF_SECTIONS)
		return wlan_static_skb;
	if ((section < 0) || (section > PREALLOC_WLAN_NUMBER_OF_SECTIONS))
		return NULL;
	if (wifi_mem_array[section].size < size)
		return NULL;
	return wifi_mem_array[section].mem_ptr;
}
EXPORT_SYMBOL(wifi_mem_prealloc);

static int init_wifi_mem(void)
{
	int i;

	for(i=0;( i < WLAN_SKB_BUF_NUM );i++) {
		if (i < (WLAN_SKB_BUF_NUM/2))
			wlan_static_skb[i] = dev_alloc_skb(4096);
		else
			wlan_static_skb[i] = dev_alloc_skb(8192);
	}
	for(i=0;( i < PREALLOC_WLAN_NUMBER_OF_SECTIONS );i++) {
		wifi_mem_array[i].mem_ptr = kmalloc(wifi_mem_array[i].size,
							GFP_KERNEL);
		if (wifi_mem_array[i].mem_ptr == NULL)
			return -ENOMEM;
	}
	return 0;
}
#endif //CONFIG_WIFI_MEM_PREALLOC


#if (0)	// by dscheon
static struct platform_device *early_devices[] __initdata = {
#ifdef CONFIG_GPIOLIB
	&msm_gpio_devices[0],
	&msm_gpio_devices[1],
	&msm_gpio_devices[2],
	&msm_gpio_devices[3],
	&msm_gpio_devices[4],
	&msm_gpio_devices[5],
#endif
};
#endif


#ifdef CONFIG_WIFI_CONTROL_FUNC //kychoi 110113

/* BCM4329 returns wrong sdio_vsn(1) when we read cccr,
 * we use predefined value (sdio_vsn=2) here to initial sdio driver well
 */
static struct embedded_sdio_data wlan_emb_data = {
	.cccr	= {
		.sdio_vsn	= 2,
		.multi_block	= 1,
		.low_speed	= 0,
		.wide_bus	= 0,
		.high_power	= 1,
		.high_speed	= 1,
	},
};

#endif

#ifndef CONFIG_WIFI_CONTROL_FUNC //kychoi 110113
//kychoi 101209 (+) EF31S_WIFI
static uint32_t wlan_on_gpio_cfgs[] = {
    	/* WLAN_SHDN */
    	GPIO_CFG(20, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),
    	/* WLAN_HOST_WAKE */
    	GPIO_CFG(19, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
};

static uint32_t wlan_off_gpio_cfgs[] = {
    	/* WLAN_SHDN */
    GPIO_CFG(20, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),
    	/* WLAN_HOST_WAKE */
    GPIO_CFG(19, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
}; //kychoi 110103 sleep current
#endif

#ifdef CONFIG_WIFI_CONTROL_FUNC
static int wlan_power_state;//kychoi 110112
#endif

static void wlan_power(int on)
{
	int rc = 0;
	int pin = 0;

	printk(KERN_ERR "%s: %d\n",__func__, on);

#ifdef CONFIG_WIFI_CONTROL_FUNC //kychoi 110113

       mdelay(20);
   		gpio_set_value(20, on);
		mdelay(40);//kychoi 110302 35

      wlan_power_state = on;//kychoi 110112
      
#else 

   	for (pin = 0; pin < ARRAY_SIZE(wlan_on_gpio_cfgs); pin++) //kychoi 110103 sleep current
   {
			rc = gpio_tlmm_config(wlan_on_gpio_cfgs[pin], GPIO_ENABLE);
			if (rc) {
				printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, wlan_on_gpio_cfgs[pin], rc);
			}
		}

	if (on) {
		gpio_set_value(20, on);
		mdelay(10);
		gpio_set_value(20, 0);
		mdelay(10);
		gpio_set_value(20, on); 
	} else {
		gpio_set_value(20, 0);
#if 0 //kychoi 110103 sleep current
		for (pin = 0; pin < ARRAY_SIZE(wlan_off_gpio_cfgs); pin++) {
			rc = gpio_tlmm_config(wlan_off_gpio_cfgs[pin], GPIO_ENABLE);
			if (rc) {
				printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, wlan_off_gpio_cfgs[pin], rc);
			}
		}	
#endif    
	}
  #endif

}

#ifdef CONFIG_WIFI_CONTROL_FUNC //kychoi 110113
static int wlan_reset_state;

int wlan_reset(int on)
{
	printk("%s: do nothing\n", __func__);
	wlan_reset_state = on;
	return 0;
}
#endif

//kychoi 101209 (-)
static void msm7x27_wlan_init(void)
{
	int rc = 0;
	wlan_power(0); //kychoi 110113 sleep current
//kychoi 101209 (-) EF31S_WIFI
#ifdef	CONFIG_WIFI_MEM_PREALLOC  //kychoi 110113
	rc = init_wifi_mem();
	if (rc != 0){
		printk(KERN_ERR "%s: init_wifi_mem_return val: %d \n", __func__, rc);
	}
#endif
}

#ifdef CONFIG_WIFI_CONTROL_FUNC //kychoi 110113
static int wlan_cd = 0; /* WIFI virtual 'card detect' status */
static void (*wlan_status_cb)(int card_present, void *dev_id);
static void *wlan_status_cb_devid;

static int wlan_status_register(
			void (*callback)(int card_present, void *dev_id),
			void *dev_id)
{
	if (wlan_status_cb)
		return -EINVAL;
	wlan_status_cb = callback;
	wlan_status_cb_devid = dev_id;
	return 0;
}

static unsigned int wlan_status(struct device *dev)
{
	return wlan_cd;
}


int wlan_set_carddetect(int val)
{
		printk(KERN_ERR"%s: %d\n", __func__, val);
    wlan_cd = val;
	if (wlan_status_cb) {
		wlan_status_cb(val, wlan_status_cb_devid);
	} else
			printk(KERN_ERR"%s: Nobody to notify\n", __func__);
	return 0;
}


static struct resource wlan_resources[] = {
	[0] = {
		.name		= "bcm4329_wlan_irq",
		.start		= MSM_GPIO_TO_INT(19),
		.end		= MSM_GPIO_TO_INT(19),
       .flags          = IORESOURCE_IRQ | IORESOURCE_IRQ_LOWLEVEL, //kychoi 110119
	},
};

static struct wifi_platform_data  wlan_control = {
	.set_power      = wlan_power,
	.set_reset      =  wlan_reset,
	.set_carddetect =  wlan_set_carddetect,
	.mem_prealloc	= wifi_mem_prealloc,
};

static struct platform_device wlan_device = {
        .name           = "bcm4329_wlan",
        .id             = 1,
        .num_resources  = ARRAY_SIZE(wlan_resources),
        .resource       = wlan_resources,
        .dev            = {
                .platform_data = &wlan_control,
        },
};
#endif //CONFIG_WIFI_CONTROL_FUNC //kychoi 110113

#ifdef CONFIG_ANDROID_RAM_CONSOLE
static struct platform_device ram_console_device = {
	.name   = "ram_console",
	.id = -1,
};
#endif /* CONFIG_ANDROID_RAM_CONSOLE */
static struct platform_device *devices[] __initdata = {
#if (0)	// by dscheon
#if !defined(CONFIG_MSM_SERIAL_DEBUGGER)
	&msm_device_uart3,
#endif
#endif
	&msm_device_smd,
	&msm_device_dmov,
	&msm_device_nand,

#ifdef CONFIG_USB_MSM_OTG_72K
	&msm_device_otg,
#ifdef CONFIG_USB_GADGET
	&msm_device_gadget_peripheral,
#endif
#endif

#ifdef CONFIG_USB_FUNCTION
	&msm_device_hsusb_peripheral,
	&mass_storage_device,
#endif

#ifdef CONFIG_USB_ANDROID
	&usb_mass_storage_device,
	&rndis_device,
#ifdef CONFIG_USB_ANDROID_DIAG
	&usb_diag_device,
#endif
	&android_usb_device,
#endif
	&msm_device_i2c,
#ifndef CONFIG_EF31S_32K_BOARD
	&smc91x_device,
#endif	
	&msm_device_tssc,
	&android_pmem_kernel_ebi1_device,
	&android_pmem_device,
	&android_pmem_adsp_device,
	&android_pmem_audio_device,
  &android_pmem_tdmb_device,         // 20091126 KKS : PV_TDMB_SURFACE
	&msm_fb_device,
	&lcdc_gordon_panel_device,
	&msm_device_uart_dm1,
#if defined(CONFIG_UART2DM_CONSOLE) || defined(CONFIG_SERIAL_MSM_HSL_CONSOLE)
	&msm_device_uart_dm2,
#endif
#ifdef CONFIG_BT
	&msm_bt_power_device,
#endif
	&msm_device_pmic_leds,
	&msm_device_snd,
	&msm_device_adspdec,
#ifdef CONFIG_SKY_TDMB_I2C_GPIO
	&msm_device_i2c_gpio_tdmb,
#endif /* CONFIG_SKY_TDMB_I2C_GPIO */
#ifdef CONFIG_TOUCH_I2C_GPIO
	&msm_device_i2c_gpio_touch,
#endif /*CONFIG_TOUCH_I2C_GPIO */
#ifdef CONFIG_SENSOR_I2C_GPIO // shpark 100107 add
	&msm_device_i2c_gpio_sensor,
#endif // CONFIG_SENSOR_I2C_GPIO
#ifdef CONFIG_MT9T013
	&msm_camera_sensor_mt9t013,
#endif
#ifdef CONFIG_MT9D112
	&msm_camera_sensor_mt9d112,
#endif
#ifdef CONFIG_S5K3E2FX
	&msm_camera_sensor_s5k3e2fx,
#endif
#ifdef CONFIG_MT9P012
	&msm_camera_sensor_mt9p012,
#endif
#ifdef CONFIG_MT9P012_KM
	&msm_camera_sensor_mt9p012_km,
#endif
#ifdef CONFIG_VB6801
	&msm_camera_sensor_vb6801,
#endif
	&msm_bluesleep_device,
#ifdef CONFIG_ARCH_MSM7X27
	&msm_device_kgsl,
#endif
#if defined(CONFIG_TSIF) || defined(CONFIG_TSIF_MODULE)
	&msm_device_tsif,
#endif
	&hs_device,
	&msm_batt_device,
#ifdef F_SKYCAM_I2C_GPIO
	&msm_device_i2c_gpio_camera,
#endif
#ifdef F_SKYCAM_SENSOR_MT9P111
	&msm_camera_sensor_mt9p111,
#endif
#ifdef F_SKYCAM_SENSOR_MT9T111
	&msm_camera_sensor_mt9t111,
#endif
#ifdef  F_SKYCAM_SENSOR_YACBAC1SDDAS
	&msm_camera_sensor_yacbac1sddas,
#endif
//by elecjang 20100320
#ifndef CONFIG_SKY_SND_MAX8981
#if ((BOARD_VER >=EF31S_PT10) ||(BOARD_VER >=EF32K_PT10)) 
#ifdef CONFIG_SKY_SND_EXTAMP
	&msm_device_i2c_gpio_subsystem,
#endif
#endif
#endif
#ifdef CONFIG_WIFI_CONTROL_FUNC //kychoi 110113
&wlan_device
#endif

};
// EF31 - ginger break
#ifdef CONFIG_FB_MSM_MDDI //pantch_\lks@PS3_20111122 - prepare ef31s_ef32k 	
#if 0
static int msm_fb_mddi_sel_clk(u32 *clk_rate)
{
	*clk_rate *= 2;
	return 0;
}

static struct mddi_platform_data mddi_pdata = {
	//.mddi_power_save = display_common_power,
	.mddi_sel_clk = msm_fb_mddi_sel_clk,
};
#endif

//HW VSync 
static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = 97,
};
#endif // TODO_APP_DISPLAY_TYPE_MDDI

static void __init msm_fb_add_devices(void)
{
#if TODO_APP_DISPLAY_TYPE_MDDI			//pantch_lks@PS3_20111122 - prepare ef31s_ef32k 	
	msm_fb_register_device("mdp", &mdp_pdata);
	msm_fb_register_device("pmdh", 0/*&mddi_pdata*/);	
	msm_fb_register_device("lcdc", 0);
#else
#ifdef CONFIG_EF31S_32K_BOARD
	msm_fb_register_device("mdp", 0);
#else // !CONFIG_EF31S_32K_BOARD
	msm_fb_register_device("mdp", &mdp_pdata);
#endif // CONFIG_EF31S_32K_BOARD
#ifdef CONFIG_FB_MSM_MDDI // 20090927
	msm_fb_register_device("pmdh", 0);
#endif // CONFIG_FB_MSM_MDDI
//20100603_c@m_???_chg [ LCD ON time reduce
#ifdef CONFIG_EF31S_32K_BOARD
	msm_fb_register_device("lcdc", 0);
#else
	msm_fb_register_device("lcdc", &lcdc_pdata);
#endif
// ]20100603_c@m_???_chg_end 

#endif

}

extern struct sys_timer msm_timer;

static void __init msm7x2x_init_irq(void)
{
	msm_init_irq();
}

static struct msm_acpu_clock_platform_data msm7x2x_clock_data = {
	.acpu_switch_time_us = 50,
	.max_speed_delta_khz = 400000,
	.vdd_switch_time_us = 62,
	.max_axi_khz = 160000,
};

void msm_serial_debug_init(unsigned int base, int irq,
			   struct device *clk_device, int signal_irq);

#if (defined(CONFIG_MMC_MSM_SDC1_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC2_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC3_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC4_SUPPORT))

#ifdef CONFIG_EF31S_32K_BOARD
static unsigned long vreg_sts,vreg_sts_wlan, gpio_sts;
#else
static unsigned long vreg_sts, gpio_sts;
#endif
static struct vreg *vreg_mmc;
static unsigned mpp_mmc = 2;

#ifdef CONFIG_EF31S_32K_BOARD
static void sdcc_gpio_init(void)
{
#ifdef CONFIG_EF31S_32K_BOARD
      vreg_mmc = vreg_get(NULL, "mmc");

    if (IS_ERR(vreg_mmc)) {
      printk(KERN_ERR "%s: vreg get failed (%ld)\n",
             __func__, PTR_ERR(vreg_mmc));
      return;
    }
#endif
#if 1//#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	int rc = 0;
	if (gpio_request(37, "sdc1_status_irq"))
		pr_err("failed to request gpio sdc1_status_irq\n");
	rc = gpio_tlmm_config(GPIO_CFG(37, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc)
		printk(KERN_ERR "%s: Failed to configure GPIO %d\n",
				__func__, rc);
#endif
	if (gpio_request(20, "sdc2_wlan_reset"))
		pr_err("failed to request gpio sdc2_wlan_reset\n");
	rc = gpio_tlmm_config(GPIO_CFG(20, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_8MA), GPIO_CFG_ENABLE);
	if (rc)
		printk(KERN_ERR "%s: Failed to configuration GPIO_20 %d\n",
				__func__, rc);

#if defined(CONFIG_BCM4329_HOSTWAKEUP) || defined(CONFIG_BCM4329_OOB_ONLY) || defined(CONFIG_BCM4329_OOB_GPIO)
	if (gpio_request(19, "wifi_host_wake"))
		pr_err("failed to request wifi_host_wake\n");
	rc = gpio_tlmm_config(GPIO_CFG(19, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_8MA), GPIO_CFG_ENABLE);
	if (rc)
		printk(KERN_ERR "%s: Failed to configuration GPIO_19 %d\n",
				__func__, rc);
#endif

}
#endif	//	CONFIG_EF31S_32K_BOARD

#ifndef FEATURE_SDC2_PULL_UP //kychoi 110105 sleepcurrent
//#define FEATURE_SDC2_PULL_UP
#endif


struct sdcc_gpio {
	struct msm_gpio *cfg_data;
	uint32_t size;
	struct msm_gpio *sleep_cfg_data;
};

static struct msm_gpio sdc1_cfg_data[] = {
	{GPIO_CFG(51, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc1_dat_3"},
	{GPIO_CFG(52, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc1_dat_2"},
	{GPIO_CFG(53, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc1_dat_1"},
	{GPIO_CFG(54, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc1_dat_0"},
	{GPIO_CFG(55, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc1_cmd"},
	{GPIO_CFG(56, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA), "sdc1_clk"},
};

static struct msm_gpio sdc2_cfg_data[] = {
#ifdef FEATURE_SDC2_PULL_UP	
	{GPIO_CFG(62, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA), "sdc2_clk"},
	{GPIO_CFG(63, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_cmd"},
	{GPIO_CFG(64, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_3"},
	{GPIO_CFG(65, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_2"},
	{GPIO_CFG(66, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_1"},
	{GPIO_CFG(67, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_0"},
#else
	{GPIO_CFG(62, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA), "sdc2_clk"},
	{GPIO_CFG(63, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA), "sdc2_cmd"},
	{GPIO_CFG(64, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA), "sdc2_dat_3"},
	{GPIO_CFG(65, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA), "sdc2_dat_2"},
	{GPIO_CFG(66, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA), "sdc2_dat_1"},
	{GPIO_CFG(67, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA), "sdc2_dat_0"},
#endif
};

static struct msm_gpio sdc2_sleep_cfg_data[] = {
#ifdef FEATURE_SDC2_PULL_UP	 //kychoi 110106
	{GPIO_CFG(62, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA), "sdc2_clk"},
	{GPIO_CFG(63, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_cmd"},
	{GPIO_CFG(64, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_3"},
	{GPIO_CFG(65, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_2"},
	{GPIO_CFG(66, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_1"},
	{GPIO_CFG(67, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc2_dat_0"},
#else
	{GPIO_CFG(62, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "sdc2_clk"},
	{GPIO_CFG(63, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "sdc2_cmd"},
	{GPIO_CFG(64, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "sdc2_dat_3"},
	{GPIO_CFG(65, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "sdc2_dat_2"},
	{GPIO_CFG(66, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "sdc2_dat_1"},
	{GPIO_CFG(67, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), "sdc2_dat_0"},
#endif
};
static struct msm_gpio sdc3_cfg_data[] = {
#ifndef CONFIG_SENSOR_I2C_GPIO // shpark 100107	
	{GPIO_CFG(88, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA), "sdc3_clk"},
	{GPIO_CFG(89, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc3_cmd"},
	{GPIO_CFG(90, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc3_dat_3"},
	{GPIO_CFG(91, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc3_dat_2"},
#endif // !CONFIG_SENSOR_I2C_GPIO	
	{GPIO_CFG(92, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc3_dat_1"},
	{GPIO_CFG(93, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc3_dat_0"},
};

static struct msm_gpio sdc4_cfg_data[] = {
	{GPIO_CFG(19, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc4_dat_3"},
	{GPIO_CFG(20, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc4_dat_2"},
	{GPIO_CFG(21, 4, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc4_dat_1"},
#ifdef CONFIG_EF31S_32K_BOARD
	{GPIO_CFG(108, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc4_dat_0"},
#else	
	{GPIO_CFG(107, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc4_cmd"},
	{GPIO_CFG(108, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), "sdc4_dat_0"},
	{GPIO_CFG(109, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA), "sdc4_clk"},
#endif
};

static struct sdcc_gpio sdcc_cfg_data[] = {
	{
		.cfg_data = sdc1_cfg_data,
		.size = ARRAY_SIZE(sdc1_cfg_data),
		.sleep_cfg_data = NULL,
	},
	{
		.cfg_data = sdc2_cfg_data,
		.size = ARRAY_SIZE(sdc2_cfg_data),
		.sleep_cfg_data = sdc2_sleep_cfg_data,
	},
	{
		.cfg_data = sdc3_cfg_data,
		.size = ARRAY_SIZE(sdc3_cfg_data),
		.sleep_cfg_data = NULL,
	},
	{
		.cfg_data = sdc4_cfg_data,
		.size = ARRAY_SIZE(sdc4_cfg_data),
		.sleep_cfg_data = NULL,
	},
};
#ifndef CONFIG_EF31S_32K_BOARD
### moved location by pantech
static unsigned long vreg_sts, gpio_sts;
static unsigned mpp_mmc = 2;
static struct vreg *vreg_mmc;
#endif

static void msm_sdcc_setup_gpio(int dev_id, unsigned int enable)
{
	int rc = 0;
	struct sdcc_gpio *curr;

	curr = &sdcc_cfg_data[dev_id - 1];
	if (!(test_bit(dev_id, &gpio_sts)^enable))
		return;

	if (enable) {
		set_bit(dev_id, &gpio_sts);
		rc = msm_gpios_request_enable(curr->cfg_data, curr->size);
		if (rc)
			printk(KERN_ERR "%s: Failed to turn on GPIOs for slot %d\n",
				__func__,  dev_id);
	} else {
		clear_bit(dev_id, &gpio_sts);
		if (curr->sleep_cfg_data) {
			msm_gpios_enable(curr->sleep_cfg_data, curr->size);
			msm_gpios_free(curr->sleep_cfg_data, curr->size);
			return;
		}
		msm_gpios_disable_free(curr->cfg_data, curr->size);
	}
}

static uint32_t msm_sdcc_setup_power(struct device *dv, unsigned int vdd)
{
	int rc = 0;
	struct platform_device *pdev;

	pdev = container_of(dv, struct platform_device, dev);
	msm_sdcc_setup_gpio(pdev->id, !!vdd);

#ifdef FAETURE_SKY_SDCARD_DETECT
	if (vdd == 0) 
		return 0;
#else
	if (vdd == 0) {
		if (!vreg_sts)
			return 0;

		clear_bit(pdev->id, &vreg_sts);

		if (!vreg_sts) {
			if (machine_is_msm7x25_ffa() ||
					machine_is_msm7x27_ffa()) {
				rc = mpp_config_digital_out(mpp_mmc,
				     MPP_CFG(MPP_DLOGIC_LVL_MSMP,
				     MPP_DLOGIC_OUT_CTRL_LOW));
			} else
				rc = vreg_disable(vreg_mmc);
			if (rc)
				printk(KERN_ERR "%s: return val: %d \n",
					__func__, rc);
		}
		return 0;
	}
#endif

	if (!vreg_sts) {
		if (machine_is_msm7x25_ffa() || machine_is_msm7x27_ffa()) {
			rc = mpp_config_digital_out(mpp_mmc,
			     MPP_CFG(MPP_DLOGIC_LVL_MSMP,
			     MPP_DLOGIC_OUT_CTRL_HIGH));
		} else {
#if 1//#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
          rc = vreg_set_level(vreg_mmc, 3000);
#else
			rc = vreg_set_level(vreg_mmc, 2850);
#endif
			if (!rc)
				rc = vreg_enable(vreg_mmc);
#ifdef FAETURE_SKY_SDCARD_DETECT
			msleep(100);
#endif
		}
		if (rc)
			printk(KERN_ERR "%s: return val: %d \n",
					__func__, rc);
	}
	set_bit(pdev->id, &vreg_sts);
	return 0;
}
#ifdef CONFIG_EF31S_32K_BOARD
static uint32_t msm_sdio_setup_power(struct device *dv, unsigned int vdd)
{
	int rc = 0;
	struct platform_device *pdev;

	pdev = container_of(dv, struct platform_device, dev);
	msm_sdcc_setup_gpio(pdev->id, !!vdd);

	if (vdd == 0) 
	{
		if (!vreg_sts_wlan)
			return 0;

		clear_bit(pdev->id, &vreg_sts_wlan);

		return 0;
	}
	
	set_bit(pdev->id, &vreg_sts_wlan);
	return 0;
}
#endif
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
static unsigned int msm7x27_sdcc_slot_status(struct device *dev)
{
    unsigned int sdcc_slot_status = 0;
	sdcc_slot_status = (gpio_get_value(37) ? 0 : 1);
	printk(KERN_ERR "%s: return val: %d \n", __func__, sdcc_slot_status);
	return sdcc_slot_status;
}
#endif
#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
static struct mmc_platform_data msm7x2x_sdc1_data = {
	.ocr_mask	= MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.nonremovable	= 0,
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	.status_irq	= MSM_GPIO_TO_INT(37),
	.status = msm7x27_sdcc_slot_status,
	//--- 10.08.11 edited by ds2@hkkwon
	//.irq_flags      = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	.irq_flags      =  IRQF_TRIGGER_FALLING,
#endif
};
#endif

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
static struct mmc_platform_data msm7x2x_sdc2_data = {
	.ocr_mask	= MMC_VDD_28_29,
#if (1)
	.translate_vdd	= msm_sdio_setup_power,	// DS2@ dsceon for Sleep Current
#else
	.translate_vdd	= msm_sdcc_setup_power,
#endif
#ifdef CONFIG_WIFI_CONTROL_FUNC //kychoi 110113
	.status			= wlan_status,
	.register_status_notify	= wlan_status_register,
	.embedded_sdio		= &wlan_emb_data, //kychoi 110302
#endif	
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#ifdef CONFIG_MMC_MSM_SDIO_SUPPORT
#ifdef CONFIG_BCM4329_HOSTWAKEUP
	.sdiowakeup_irq = MSM_GPIO_TO_INT(19),
#else
	//.sdiowakeup_irq = MSM_GPIO_TO_INT(66),
#endif
#endif
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.nonremovable	= 1,
};
#endif

#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
static struct mmc_platform_data msm7x2x_sdc3_data = {
	.ocr_mask	= MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.nonremovable	= 0,
};
#endif

#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
static struct mmc_platform_data msm7x2x_sdc4_data = {
	.ocr_mask	= MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.nonremovable	= 0,
};
#endif

static void __init msm7x2x_init_mmc(void)
{
	if (!machine_is_msm7x25_ffa() && !machine_is_msm7x27_ffa()) {
		vreg_mmc = vreg_get(NULL, "mmc");
		if (IS_ERR(vreg_mmc)) {
			printk(KERN_ERR "%s: vreg get failed (%ld)\n",
			       __func__, PTR_ERR(vreg_mmc));
			return;
		}
	}

	sdcc_gpio_init();
#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
	msm_add_sdcc(1, &msm7x2x_sdc1_data);
#endif
#ifndef CONFIG_EF31S_32K_BOARD
	if (machine_is_msm7x25_surf() || machine_is_msm7x27_surf() ||
		machine_is_msm7x27_ffa()) {
#endif		
#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
		msm_sdcc_setup_gpio(2, 1);
		msm_add_sdcc(2, &msm7x2x_sdc2_data);
#endif
#ifndef CONFIG_EF31S_32K_BOARD
	}
#endif
	if (machine_is_msm7x25_surf() || machine_is_msm7x27_surf()) {
#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
		msm_add_sdcc(3, &msm7x2x_sdc3_data);
#endif
#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
		msm_add_sdcc(4, &msm7x2x_sdc4_data);
#endif
	}
}
#else
#define msm7x2x_init_mmc() do {} while (0)
#endif


static struct msm_pm_platform_data msm7x25_pm_data[MSM_PM_SLEEP_MODE_NR] = {
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].latency = 16000,

	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].latency = 12000,

	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency = 2000,
};

static struct msm_pm_platform_data msm7x27_pm_data[MSM_PM_SLEEP_MODE_NR] = {
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].supported = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].suspend_enabled = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].idle_enabled = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].latency = 16000,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].residency = 20000,

	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].supported = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].suspend_enabled = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].idle_enabled = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].latency = 12000,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].residency = 20000,

	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].supported = 1,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].suspend_enabled
		= 1,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].idle_enabled = 1,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency = 2000,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].residency = 0,
};

static void
msm_i2c_gpio_config(int iface, int config_type)
{
	int gpio_scl;
	int gpio_sda;
	if (iface) {
#if 1 //temp
		gpio_scl = 60;
		gpio_sda = 61;
#else
		gpio_scl = 95;
		gpio_sda = 96;
#endif		
	} else {
		gpio_scl = 60;
		gpio_sda = 61;
	}
	if (config_type) {
		gpio_tlmm_config(GPIO_CFG(gpio_scl, 1, GPIO_CFG_INPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_16MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(GPIO_CFG(gpio_sda, 1, GPIO_CFG_INPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_16MA), GPIO_CFG_ENABLE);
	} else {
		gpio_tlmm_config(GPIO_CFG(gpio_scl, 0, GPIO_CFG_OUTPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_16MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(GPIO_CFG(gpio_sda, 0, GPIO_CFG_OUTPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_16MA), GPIO_CFG_ENABLE);
	}
}

static struct msm_i2c_platform_data msm_i2c_pdata = {
	.clk_freq = 100000,
	.rmutex  = 0,
	.pri_clk = 60,
	.pri_dat = 61,
#if 1 //temp
	.aux_clk = 60,
	.aux_dat = 61,
#else
	.aux_clk = 95,
	.aux_dat = 96,
#endif	
	.msm_i2c_config_gpio = msm_i2c_gpio_config,
};

static struct vreg *vreg_synt;
static void __init msm_device_i2c_init(void)
{
    int rc;
	if (gpio_request(60, "i2c_pri_clk"))
		pr_err("failed to request gpio i2c_pri_clk\n");
	if (gpio_request(61, "i2c_pri_dat"))
		pr_err("failed to request gpio i2c_pri_dat\n");
/* temp
	if (gpio_request(95, "i2c_sec_clk"))
		pr_err("failed to request gpio i2c_sec_clk\n");
	if (gpio_request(96, "i2c_sec_dat"))
		pr_err("failed to request gpio i2c_sec_dat\n");
*/
	if (cpu_is_msm7x27())
		msm_i2c_pdata.pm_lat =
		msm7x27_pm_data[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN]
		.latency;
	else
		msm_i2c_pdata.pm_lat =
		msm7x25_pm_data[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN]
		.latency;

	msm_device_i2c.dev.platform_data = &msm_i2c_pdata;
}

static void usb_mpp_init(void)
{
	unsigned rc;
	unsigned mpp_usb = 7;

	if (machine_is_msm7x25_ffa() || machine_is_msm7x27_ffa()) {
		rc = mpp_config_digital_out(mpp_usb,
			MPP_CFG(MPP_DLOGIC_LVL_VDD,
				MPP_DLOGIC_OUT_CTRL_HIGH));
		if (rc)
			pr_err("%s: configuring mpp pin"
				"to enable 3.3V LDO failed\n", __func__);
	}
}

#ifdef	CONFIG_EF31S_32K_BOARD
static void __init pantech_init_misc(void)
{
    printk(KERN_ERR "%s is called\n", __func__);
    
    
#if ((BOARD_VER >=EF31S_PT10) ||(BOARD_VER >=EF32K_PT10)) 
    msm_init_sky_vibrator();
#endif
    
    // added by jwcha100113

    platform_device_register(&msm_device_subsystem);
    platform_device_register(&pantech_backlight_device);
    platform_device_register(&pantech_debug_device);
/// wjmin 2010.09.27@DS2 Disable
#if 0//def CONFIG_ANDROID_RAM_CONSOLE
//    platform_device_register(&ram_console_device);
#endif
}
#endif	// CONFIG_EF31S_32K_BOARD

static void __init msm7x2x_init(void)
{

#ifdef CONFIG_ARCH_MSM7X25
	msm_clock_init(msm_clocks_7x25, msm_num_clocks_7x25);
#elif defined(CONFIG_ARCH_MSM7X27)
	msm_clock_init(msm_clocks_7x27, msm_num_clocks_7x27);
#endif

	printk(KERN_INFO"%s: GingerBread FC 6120 Start + \n",__func__);
#if defined(CONFIG_SMC91X)
	if (machine_is_msm7x25_ffa() || machine_is_msm7x27_ffa()) {
		smc91x_resources[0].start = 0x98000300;
		smc91x_resources[0].end = 0x980003ff;
		smc91x_resources[1].start = MSM_GPIO_TO_INT(85);
		smc91x_resources[1].end = MSM_GPIO_TO_INT(85);
		if (gpio_tlmm_config(GPIO_CFG(85, 0,
					      GPIO_CFG_INPUT,
					      GPIO_CFG_PULL_DOWN,
					      GPIO_CFG_2MA),
				     GPIO_CFG_ENABLE)) {
			printk(KERN_ERR
			       "%s: Err: Config GPIO-85 INT\n",
				__func__);
		}
	}
#endif

	if (cpu_is_msm7x27())
		msm7x2x_clock_data.max_axi_khz = 200000;

	msm_acpu_clock_init(&msm7x2x_clock_data);

#ifdef CONFIG_ARCH_MSM7X27
	/* This value has been set to 160000 for power savings. */
	/* OEMs may modify the value at their discretion for performance */
	/* The appropriate maximum replacement for 160000 is: */
	/* clk_get_max_axi_khz() */
	kgsl_pdata.high_axi_3d = 160000;

	/* 7x27 doesn't allow graphics clocks to be run asynchronously to */
	/* the AXI bus */
	kgsl_pdata.max_grp2d_freq = 0;
	kgsl_pdata.min_grp2d_freq = 0;
	kgsl_pdata.set_grp2d_async = NULL;
	kgsl_pdata.max_grp3d_freq = 0;
	kgsl_pdata.min_grp3d_freq = 0;
	kgsl_pdata.set_grp3d_async = NULL;
	kgsl_pdata.imem_clk_name = "imem_clk";
	kgsl_pdata.grp3d_clk_name = "grp_clk";
	kgsl_pdata.grp3d_pclk_name = "grp_pclk";
	kgsl_pdata.grp2d0_clk_name = NULL;
	kgsl_pdata.idle_timeout_3d = HZ/5;
	kgsl_pdata.idle_timeout_2d = 0;

#ifdef CONFIG_KGSL_PER_PROCESS_PAGE_TABLE
	kgsl_pdata.pt_va_size = SZ_32M;
#else
	kgsl_pdata.pt_va_size = SZ_128M;
#endif
#endif
	usb_mpp_init();

#ifdef CONFIG_USB_FUNCTION
	msm_hsusb_pdata.swfi_latency =
		msm7x27_pm_data
		[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency;

	msm_device_hsusb_peripheral.dev.platform_data = &msm_hsusb_pdata;
#endif

#ifdef CONFIG_USB_MSM_OTG_72K
	msm_device_otg.dev.platform_data = &msm_otg_pdata;
	if (machine_is_msm7x25_surf() || machine_is_msm7x25_ffa()) {
		msm_otg_pdata.pemp_level =
			PRE_EMPHASIS_WITH_20_PERCENT;
		msm_otg_pdata.drv_ampl = HS_DRV_AMPLITUDE_5_PERCENT;
		msm_otg_pdata.cdr_autoreset = CDR_AUTO_RESET_ENABLE;
		msm_otg_pdata.phy_reset = msm_otg_rpc_phy_reset;
	}
	if (machine_is_msm7x27_surf() || machine_is_msm7x27_ffa()) {
		msm_otg_pdata.pemp_level =
			PRE_EMPHASIS_WITH_10_PERCENT;
		msm_otg_pdata.drv_ampl = HS_DRV_AMPLITUDE_5_PERCENT;
		msm_otg_pdata.cdr_autoreset = CDR_AUTO_RESET_DISABLE;
		msm_otg_pdata.phy_reset_sig_inverted = 1;
	}

#ifdef CONFIG_USB_GADGET
	msm_otg_pdata.swfi_latency =
		msm7x27_pm_data
		[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency;
	msm_device_gadget_peripheral.dev.platform_data = &msm_gadget_pdata;
	msm_gadget_pdata.is_phy_status_timer_on = 1;
#endif
#endif
#if defined(CONFIG_TSIF) || defined(CONFIG_TSIF_MODULE)
	msm_device_tsif.dev.platform_data = &tsif_platform_data;
#endif

	platform_add_devices(devices, ARRAY_SIZE(devices));
#ifdef CONFIG_MSM_CAMERA
	config_camera_off_gpios(); /* might not be necessary */
#endif
	msm_device_i2c_init();
	i2c_register_board_info(0, i2c_devices, ARRAY_SIZE(i2c_devices));
#if defined(F_SKYCAM_I2C_GPIO)
	i2c_register_board_info(4, msm_i2c_gpio_board_info, ARRAY_SIZE(msm_i2c_gpio_board_info)); 
#endif 
#ifdef CONFIG_EF31S_32K_BOARD
#if ((BOARD_VER >=EF31S_PT10) ||(BOARD_VER >=EF32K_PT10)) 
	platform_device_register(&keypad_device_EF13);
#endif
#else // !CONFIG_EF31S_32K_BOARD
#ifdef CONFIG_SURF_FFA_GPIO_KEYPAD
	if (machine_is_msm7x25_ffa() || machine_is_msm7x27_ffa())
		platform_device_register(&keypad_device_7k_ffa);
	else
		platform_device_register(&keypad_device_surf);
#endif
#endif // CONFIG_EF31S_32K_BOARD
	//lcdc_gordon_gpio_init();//pantch_lks@PS3_20111117 - prepare ef31s_ef32k  		
	mddi_sky_gpio_init();
	msm_fb_add_devices();
#ifdef CONFIG_USB_EHCI_MSM
	msm7x2x_init_host();
#endif
	msm7x2x_init_mmc();
	bt_power_init();

#ifdef CONFIG_SKY_TDMB_EBI_IF
	msm_tdmb_add_devices();
#endif /* CONFIG_SKY_TDMB_EBI_IF */
#ifdef CONFIG_TOUCH_I2C_GPIO
	i2c_register_board_info(1, i2c_touch_devices,
				ARRAY_SIZE(i2c_touch_devices));
#endif /* CONFIG_TOUCH_I2C_GPIO */
#ifdef CONFIG_SKY_TDMB_I2C_GPIO
	i2c_register_board_info(3, i2c_tdmb_devices,
				ARRAY_SIZE(i2c_tdmb_devices));
#endif /* CONFIG_SKY_TDMB_I2C_GPIO */
//by elecjang 20100320
#ifndef CONFIG_SKY_SND_MAX8981
#if ((BOARD_VER >=EF31S_PT10) ||(BOARD_VER >=EF32K_PT10)) 
#ifdef CONFIG_SKY_SND_EXTAMP
	i2c_register_board_info(5, i2c_subsystem_devices,
				ARRAY_SIZE(i2c_subsystem_devices));
#endif
#endif
#endif
//---	10.07.22 added by hkkwon	---//
#ifdef CONFIG_SENSOR_I2C_GPIO
	i2c_register_board_info(2, i2c_sensor_device,
				ARRAY_SIZE(i2c_sensor_device));
#endif
	if (cpu_is_msm7x27())
		msm_pm_set_platform_data(msm7x27_pm_data,
					ARRAY_SIZE(msm7x27_pm_data));
	else
		msm_pm_set_platform_data(msm7x25_pm_data,
					ARRAY_SIZE(msm7x25_pm_data));
	msm7x27_wlan_init();
#ifdef	CONFIG_EF31S_32K_BOARD
	pantech_init_misc();	// added by jwcha100113
#endif	// CONFIG_EF31S_32K_BOARD
   // wlan_power(0); //kychoi 110103 sleep current

	printk(KERN_INFO"%s: GingerBread FC 6120 Start - \n",__func__);
	
}

static unsigned pmem_kernel_ebi1_size = PMEM_KERNEL_EBI1_SIZE;
static int __init pmem_kernel_ebi1_size_setup(char *p)
{
	pmem_kernel_ebi1_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_kernel_ebi1_size", pmem_kernel_ebi1_size_setup);

static unsigned pmem_mdp_size = MSM_PMEM_MDP_SIZE;
static int __init pmem_mdp_size_setup(char *p)
{
	pmem_mdp_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_mdp_size", pmem_mdp_size_setup);

static unsigned pmem_adsp_size = MSM_PMEM_ADSP_SIZE;
static int __init pmem_adsp_size_setup(char *p)
{
	pmem_adsp_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_adsp_size", pmem_adsp_size_setup);

#ifndef CONFIG_EF31S_32K_BOARD
static unsigned pmem_audio_size = MSM_PMEM_AUDIO_SIZE;
static int __init pmem_audio_size_setup(char *p)
{
	pmem_audio_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_audio_size", pmem_audio_size_setup);
#endif
#if 1// 20091126 KKS : PV_TDMB_SURFACE
static unsigned pmem_tdmb_size = MSM_PMEM_TDMB_SIZE;
static int __init pmem_tdmb_size_setup(char **p)
{
	pmem_tdmb_size = memparse(*p, p);
	return 0;
}
__early_param("pmem_tdmb_size=", pmem_tdmb_size_setup);
#endif

static unsigned fb_size = MSM_FB_SIZE;
static int __init fb_size_setup(char *p)
{
	fb_size = memparse(p, NULL);
	return 0;
}
early_param("fb_size", fb_size_setup);

static void __init msm_msm7x2x_allocate_memory_regions(void)
{
	void *addr;
	unsigned long size;

	size = pmem_mdp_size;
	if (size) {
		addr = alloc_bootmem(size);
		android_pmem_pdata.start = __pa(addr);
		android_pmem_pdata.size = size;
		pr_info("allocating %lu bytes at %p (%lx physical) for mdp "
			"pmem arena\n", size, addr, __pa(addr));
	}

	size = pmem_adsp_size;
	if (size) {
		addr = alloc_bootmem(size);
		android_pmem_adsp_pdata.start = __pa(addr);
		android_pmem_adsp_pdata.size = size;
		pr_info("allocating %lu bytes at %p (%lx physical) for adsp "
			"pmem arena\n", size, addr, __pa(addr));
	}

#ifdef CONFIG_EF31S_32K_BOARD
	size = MSM_PMEM_AUDIO_SIZE ;
	android_pmem_audio_pdata.start = MSM_PMEM_AUDIO_START_ADDR ;
	android_pmem_audio_pdata.size = size;
	pr_info("allocating %lu bytes (at %lx physical) for audio "
		"pmem arena\n", size , MSM_PMEM_AUDIO_START_ADDR);
#else
	size = pmem_audio_size;
	if (size) {
		addr = alloc_bootmem(size);
		android_pmem_audio_pdata.start = __pa(addr);
		android_pmem_audio_pdata.size = size;
		pr_info("allocating %lu bytes (at %lx physical) for audio "
			"pmem arena\n", size , __pa(addr));
	}
#endif

	size = fb_size ? : MSM_FB_SIZE;
    //20100910_c@m_ejkim@_chg [SW RESET DISPLAY 
	//addr = alloc_bootmem(size);
	//msm_fb_resources[0].start = __pa(addr);
	addr =(void*)0xD440000;
	msm_fb_resources[0].start = (unsigned long)addr;
    //20100910_c@m_ejkim@_chg_end ] 
	msm_fb_resources[0].end = msm_fb_resources[0].start + size - 1;
	pr_info("allocating %lu bytes at %p (%lx physical) for fb\n",
		size, addr, __pa(addr));

	size = pmem_kernel_ebi1_size;
	if (size) {
		addr = alloc_bootmem_aligned(size, 0x100000);
		android_pmem_kernel_ebi1_pdata.start = __pa(addr);
		android_pmem_kernel_ebi1_pdata.size = size;
		pr_info("allocating %lu bytes at %p (%lx physical) for kernel"
			" ebi1 pmem arena\n", size, addr, __pa(addr));
	}
#if 1  // 20091126 KKS : PV_TDMB_SURFACE
	size = pmem_tdmb_size;
	if (size) {
		addr = alloc_bootmem(size);
		android_pmem_tdmb_pdata.start = __pa(addr);
		android_pmem_tdmb_pdata.size = size;
		pr_info("allocating %lu bytes at %p (%lx physical) for tdmb "
			"pmem arena\n", size, addr, __pa(addr));
	}
#endif
}

static void __init msm7x2x_map_io(void)
{
	msm_map_common_io();
	msm_msm7x2x_allocate_memory_regions();

	if (socinfo_init() < 0)
		BUG();

#ifdef CONFIG_CACHE_L2X0
	if (machine_is_msm7x27_surf() || machine_is_msm7x27_ffa()) {
		/* 7x27 has 256KB L2 cache:
			64Kb/Way and 4-Way Associativity;
			evmon/parity/share disabled. */
		if ((SOCINFO_VERSION_MAJOR(socinfo_get_version()) > 1)
			|| ((SOCINFO_VERSION_MAJOR(socinfo_get_version()) == 1)
			&& (SOCINFO_VERSION_MINOR(socinfo_get_version()) >= 3)))
			/* R/W latency: 4 cycles; */
			l2x0_init(MSM_L2CC_BASE, 0x0006801B, 0xfe000000);
		else
			/* R/W latency: 3 cycles; */
			l2x0_init(MSM_L2CC_BASE, 0x00068012, 0xfe000000);
	}
#endif
}

MACHINE_START(MSM7X27_SURF, "QCT MSM7x27 SURF")
#ifdef CONFIG_MSM_DEBUG_UART
	.phys_io        = MSM_DEBUG_UART_PHYS,
	.io_pg_offst    = ((MSM_DEBUG_UART_BASE) >> 18) & 0xfffc,
#endif
	.boot_params	= PHYS_OFFSET + 0x100,
	.map_io		= msm7x2x_map_io,
	.init_irq	= msm7x2x_init_irq,
	.init_machine	= msm7x2x_init,
	.timer		= &msm_timer,
MACHINE_END

MACHINE_START(MSM7X27_FFA, "QCT MSM7x27 FFA")
#ifdef CONFIG_MSM_DEBUG_UART
	.phys_io        = MSM_DEBUG_UART_PHYS,
	.io_pg_offst    = ((MSM_DEBUG_UART_BASE) >> 18) & 0xfffc,
#endif
	.boot_params	= PHYS_OFFSET + 0x100,
	.map_io		= msm7x2x_map_io,
	.init_irq	= msm7x2x_init_irq,
	.init_machine	= msm7x2x_init,
	.timer		= &msm_timer,
MACHINE_END

MACHINE_START(MSM7X25_SURF, "QCT MSM7x25 SURF")
#ifdef CONFIG_MSM_DEBUG_UART
	.phys_io        = MSM_DEBUG_UART_PHYS,
	.io_pg_offst    = ((MSM_DEBUG_UART_BASE) >> 18) & 0xfffc,
#endif
	.boot_params	= PHYS_OFFSET + 0x100,
	.map_io		= msm7x2x_map_io,
	.init_irq	= msm7x2x_init_irq,
	.init_machine	= msm7x2x_init,
	.timer		= &msm_timer,
MACHINE_END

MACHINE_START(MSM7X25_FFA, "QCT MSM7x25 FFA")
#ifdef CONFIG_MSM_DEBUG_UART
	.phys_io        = MSM_DEBUG_UART_PHYS,
	.io_pg_offst    = ((MSM_DEBUG_UART_BASE) >> 18) & 0xfffc,
#endif
	.boot_params	= PHYS_OFFSET + 0x100,
	.map_io		= msm7x2x_map_io,
	.init_irq	= msm7x2x_init_irq,
	.init_machine	= msm7x2x_init,
	.timer		= &msm_timer,
MACHINE_END
