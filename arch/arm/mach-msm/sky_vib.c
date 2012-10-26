/* include/asm/mach-msm/htc_pwrsink.h
 *
 * Copyright (C) 2008 HTC Corporation.
 * Copyright (C) 2007 Google, Inc.
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
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/hrtimer.h>
#include <../../../drivers/staging/android/timed_output.h>
#include <linux/sched.h>

#include <mach/msm_rpcrouter.h>
#include <mach/gpio.h>
#include <mach/vreg.h>

#if	0
// removed by jwcha100203
static struct work_struct work_vibrator_on;
static struct work_struct work_vibrator_off;
#endif
static struct hrtimer vibe_timer;

static struct vreg *vreg_motor;
static int nVibratorOn=0;

extern int g_call_status;

// syncronize with $/drivers/pmic/pmic3/rpc/lib/sw/pm_lib_rpc.c's RPM ID & VER
#define PM_LIBPROG					0x30000061
#define PM_LIBVERS					0x00010001 /* 0x00010001 */


#define SKY_PROCEDURE_PM_VIB_MOT_SET_VOLT_PROC 		22 
#define SKY_PROCEDURE_PM_VIB_MOT_SET_MODE_PROC 		23 
#define SKY_PROCEDURE_PM_VIB_MOT_SET_POLARITY_PROC 	24


#define VIB_DEFAULT_VOLTAGE				3000
#define VIB_MAX_VOLTAGE					3000
#define VIB_MIN_VOLTAGE					2000
#define VIB_INCALL_MAX_VOLTAGE				2000
static int vib_level = VIB_DEFAULT_VOLTAGE;

static void set_sky_vibrator(int on)
{
	int rc;	
	if(on)
	{
		if(nVibratorOn==0)
		{
			nVibratorOn=1;
			rc = vreg_set_level(vreg_motor, vib_level);
			rc = vreg_enable(vreg_motor);
			if (rc) {
				printk(KERN_ERR "%s: vreg enable failed (%d)\n",
			       	__func__, rc);
				return;
			}
		}
	}
	else
	{
		if(nVibratorOn=1)
		{
			nVibratorOn=0;
			rc = vreg_disable(vreg_motor);
			if (rc) {
				printk(KERN_ERR "%s: vreg disable failed (%d)\n",
			       	__func__, rc);
				return;
			}
		}
	}		

}

#if	0
// removed by jwcha100203
static void sky_vibrator_on(struct work_struct *work)
{
	set_sky_vibrator(1);
}

static void sky_vibrator_off(struct work_struct *work)
{
	set_sky_vibrator(0);
}

static void timed_vibrator_on(struct timed_output_dev *sdev)
{
	schedule_work(&work_vibrator_on);
}

static void timed_vibrator_off(struct timed_output_dev *sdev)
{
	schedule_work(&work_vibrator_off);
}
#endif

static void vibrator_enable(struct timed_output_dev *dev, int value)
{
	int intensity;
	//printk("%s: %d ms\n", __func__, value);
	hrtimer_cancel(&vibe_timer);

	if (value == 0)
		set_sky_vibrator(0);
	else {
				
				

#if	0
		value = (value > 15000 ? 15000 : value);
		timed_vibrator_on(dev);
#else
		// high 16bit : intensity
		// low 16bit  : timeout		
		intensity = (value >> 16) & 0x0000FFFF;	// get 'intensity'
		if( (intensity < 1) || (intensity>20) )
			return;
			
		if(g_call_status) 
			vib_level=VIB_MIN_VOLTAGE+ intensity*(VIB_INCALL_MAX_VOLTAGE-VIB_MIN_VOLTAGE)/20;
		else
			vib_level=VIB_MIN_VOLTAGE+ intensity*(VIB_MAX_VOLTAGE-VIB_MIN_VOLTAGE)/20;
			
		vib_level=(vib_level/100)*100;
		value = value & 0x0000FFFF;
		
		printk("vib level : %d, duration :  %d ms\n", vib_level, value);
				
		// changed by jwcha100203
		// directly on
		set_sky_vibrator(1);
#endif

		hrtimer_start(&vibe_timer,
			      ktime_set(value / 1000, (value % 1000) * 1000000),
			      HRTIMER_MODE_REL);
	}
}

static int vibrator_get_time(struct timed_output_dev *dev)
{
	if (hrtimer_active(&vibe_timer)) {
		ktime_t r = hrtimer_get_remaining(&vibe_timer);
		return r.tv.sec * 1000 + r.tv.nsec / 1000000;
	} else
		return 0;
}

static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{
#if	0
	timed_vibrator_off(NULL);
#else
	// changed by jwcha100203
	// directly off
	set_sky_vibrator(0);
#endif
	return HRTIMER_NORESTART;
}

static struct timed_output_dev sky_vibrator = {
	.name = "vibrator",
	.get_time = vibrator_get_time,
	.enable = vibrator_enable,
};

void __init msm_init_sky_vibrator(void)
{
	int rc;
#if	0
// removed by jwcha100203
	INIT_WORK(&work_vibrator_on, sky_vibrator_on);
	INIT_WORK(&work_vibrator_off, sky_vibrator_off);
#endif

	vreg_motor = vreg_get(NULL, "gp6");
	rc = vreg_set_level(vreg_motor, 3000);
	if (rc) {
		printk(KERN_ERR "%s: vreg set level failed (%d)\n",
	       	__func__, rc);
		return -EIO;
	}
	

	hrtimer_init(&vibe_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	vibe_timer.function = vibrator_timer_func;

	timed_output_dev_register(&sky_vibrator);
}

MODULE_AUTHOR("Moon Jong Soon<cafe2020@pantech.com>");
MODULE_DESCRIPTION("timed output sky vibrator device");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sky_vib");

