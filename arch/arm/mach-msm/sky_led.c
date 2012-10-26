/*
 * sky_led.c
 * Copyright (c) Moon Jong Soon <cafe2020@pantech.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 * sky leds driver
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include <mach/vreg.h>


static struct vreg *vreg_button_led;
static int nButtonLedOn=0;
/*
 * button backlight led.
 * (it can only be blue flashing led)
 */
 
 

static enum led_brightness sky_button_bl_get(struct led_classdev *cdev)
{
	enum led_brightness brightness = LED_OFF;
	if(nButtonLedOn==0)
		brightness = LED_OFF;
	else
		brightness = LED_FULL;
		
	return brightness;
} 
 
static void sky_button_bl_set(struct led_classdev *led_dev,
			      enum led_brightness value)
{	
	int rc;	
	if(value)
	{
		if(nButtonLedOn==0)
		{
			nButtonLedOn=1;
			rc = vreg_enable(vreg_button_led);
			if (rc) {
				printk(KERN_ERR "%s: vreg enable failed (%d)\n",
			       	__func__, rc);
				return;
			}
		}
	}
	else
	{
		if(nButtonLedOn=1)
		{
			nButtonLedOn=0;
			rc = vreg_disable(vreg_button_led);
			if (rc) {
				printk(KERN_ERR "%s: vreg disable failed (%d)\n",
			       	__func__, rc);
				return;
			}
		}
	}		
}
	
	

static struct led_classdev sky_button_backlight = {
	.name			= "button-backlight",
	.brightness_get 	= sky_button_bl_get,
	.brightness_set		= sky_button_bl_set,
};

#if 1 // EF31
static int skyleds_probe(struct platform_device *pdev)
#else
static int __init skyleds_probe(struct platform_device *pdev)
#endif
{
	int rc;	
	vreg_button_led = vreg_get(NULL, "rfrx2");
	rc = vreg_set_level(vreg_button_led, 3000);
	if (rc) {
		printk(KERN_ERR "%s: vreg set level failed (%d)\n",
	       	__func__, rc);
		return -EIO;
	}	
	
	rc = led_classdev_register(&pdev->dev, &sky_button_backlight);
	if (rc)
		goto err_buttonbl;
		
	return 0;

err_buttonbl:
	led_classdev_unregister(&sky_button_backlight);
	return rc;
}

static int skyleds_remove(struct platform_device *pdev)
{
	led_classdev_unregister(&sky_button_backlight);
	return 0;
}


static struct platform_driver skyleds_driver = {
	.driver		= {
		.name	= "pantech-bl",
		.owner	= THIS_MODULE,
	},
	.probe		= skyleds_probe,
	.remove		= skyleds_remove,
};


#if 1 // EF31
static int skyleds_init(void)
#else
static int __init skyleds_init(void)
#endif
{
	return platform_driver_register(&skyleds_driver);
}

#if 1 // EF31
static void skyleds_exit(void)
#else
static void __exit skyleds_exit(void)
#endif
{
	platform_driver_unregister(&skyleds_driver);
}

module_init(skyleds_init);
module_exit(skyleds_exit);

MODULE_AUTHOR("Moon Jong Soon<cafe2020@pantech.com>");
MODULE_DESCRIPTION("LED driver for the sky handset");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sky-leds");
