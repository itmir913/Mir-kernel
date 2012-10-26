/*
 * Support for the Headset Detection(GPIO IRQ).
 * These are implemented in linux kernel 2.6ver
 *
 * Copyright (c) 2009 Jang Dusin <elecjang@pantech.com>
 *
 * This file may be distributed under the terms of the GNU GPL license.
 */


#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/switch.h>
#include <linux/mutex.h>

#include <linux/init.h>
#include <linux/platform_device.h>
#include "proc_comm.h"
#include "pantech-headset.h"

struct headset_gpio_info {
	//struct timer_list htimer;
	//struct mutex mutex_lock;
        struct switch_dev headset_dev;
        struct input_dev  *hook_key;
	unsigned gpio_num;
	int	headset_state;
	struct work_struct work;
};


static struct headset_gpio_info *soulmate_headset_info;

#if (1)

static void PantechHeadset_work_func(struct work_struct *work)
{
	unsigned long irqflags;

	printk(KERN_INFO "PantechHeadset_work_func [%d]\r\n",soulmate_headset_info->headset_state); 

	if (soulmate_headset_info->headset_state )
	{
		irqflags = IRQ_TYPE_LEVEL_LOW;
	}
	else
	{
		irqflags = IRQ_TYPE_LEVEL_HIGH;
		switch_set_state(&soulmate_headset_info->headset_dev, 0);		// Why Need ?
	}

//modified by elecjang, earjack detection for AudioService.java 
#if 1//ndef T_SATURN 		// Needless code by dscheon Why? Micro USB erased
    printk(KERN_INFO "Headset state[%d]\n",soulmate_headset_info->headset_state);
	Send2Msm_Handset_Block(soulmate_headset_info->headset_state);
#endif
	set_irq_type(MSM_GPIO_TO_INT(soulmate_headset_info->gpio_num), irqflags);	

	enable_irq(MSM_GPIO_TO_INT(soulmate_headset_info->gpio_num));
}

static int SoulMate_Headset_interrupt(int irq, void *dev_id)
{

	struct headset_gpio_info *data = dev_id;

	soulmate_headset_info->headset_state = gpio_get_value(soulmate_headset_info->gpio_num);

printk(KERN_ERR "SoulMate_Headset_interrupt\n");	

	disable_irq(MSM_GPIO_TO_INT(soulmate_headset_info->gpio_num));
	
	schedule_work(&data->work);

	return 0;
}
#endif
#if (0)
// Headset detection
static irqreturn_t soulmate_headset_isr(int irq, void *dummy)
{
	struct	headset_gpio_info *info = (struct headset_gpio_info *)dummy;
	unsigned long irqflags;

	printk(KERN_ERR "%s: ISR is called (irq:%d/gpio:%d)\n", __func__, irq, info->gpio_num);
	disable_irq(MSM_GPIO_TO_INT(info->gpio_num));
	info->headset_state = gpio_get_value(info->gpio_num);

	//if (!timer_pending(&info->htimer))
	//	mod_timer(&info->htimer, jiffies + msecs_to_jiffies(MHINGE_SCAN_INTERVAL));
	
	printk(KERN_ERR "%s: ISR is called (state: %d)\n", __func__, info->headset_state);

	//if(headset_initialized){
	//	printk(KERN_ERR "%s: elecjang headset_initialized\n", __func__); 
		//mutex_lock(&info->mutex_lock);
	//	switch_set_state(&soulmate_headset_info->headset_dev, info->headset_state);
		//mutex_unlock(&info->mutex_lock);
	//}
	//else {
	//	printk(KERN_ERR "%s: heaset driver is NOT registered\n", __func__); 
	//}
	//struct msm_handset *hs = input_get_drvdata(soulmate_headset_info);

	if (info->headset_state){
		irqflags = IRQ_TYPE_LEVEL_LOW;
	}
	else{
		irqflags = IRQ_TYPE_LEVEL_HIGH;
		switch_set_state(&soulmate_headset_info->headset_dev, 0);
	}

	set_irq_type(MSM_GPIO_TO_INT(info->gpio_num), irqflags);

	enable_irq(MSM_GPIO_TO_INT(info->gpio_num));


	if (info->headset_state)
		switch_set_state(&soulmate_headset_info->headset_dev, 1);
		//report_headset_switch(hsdev, SW_HEADPHONE_INSERT, 1);
	else
		switch_set_state(&soulmate_headset_info->headset_dev, 0);

	return IRQ_HANDLED;
}
#endif


static int soulmate_headset_irq_init(void)
{
	int	rc;
	int	ret;
	unsigned long irqflags;


	soulmate_headset_info = kzalloc(sizeof(struct headset_gpio_info), GFP_KERNEL);

		printk(KERN_ERR "soulmate_headset_irq_init\n");
	if (!soulmate_headset_info) {
		return -1;
	}

	soulmate_headset_info->headset_dev.name = "soulmate-headset";
	//soulmate_headset_info->headset_dev.index = 1;
	soulmate_headset_info->headset_dev.state = 0;

	//soulmate_headset_info->hook_key.state = 0;
	soulmate_headset_info->gpio_num = IRQ_HEADSET_GPIO_PORT;	// HEADSET Detect GPIO
	soulmate_headset_info->headset_state = 0;

	printk(KERN_ERR "soulmate_headset_info->gpio_num (num=%d)\n", soulmate_headset_info->gpio_num);

	ret = switch_dev_register(&soulmate_headset_info->headset_dev);
	if (ret < 0)
		return -1;


	printk(KERN_ERR "soulmate_headset switch_dev_register\n");

	INIT_WORK(&soulmate_headset_info->work, PantechHeadset_work_func);

	rc = gpio_request(soulmate_headset_info->gpio_num, "gpio_headset_detect");
	if (rc) {
		gpio_free(soulmate_headset_info->gpio_num);
		printk(KERN_ERR "gpio_request failed on pin %d (rc=%d)\n", 
				soulmate_headset_info->gpio_num, rc);
		return -1;
	}

	rc = gpio_direction_input(soulmate_headset_info->gpio_num);
	if (rc) {
		gpio_free(soulmate_headset_info->gpio_num);
		printk(KERN_ERR "gpio_direction_input failed on pin %d (rc=%d)\n",
				soulmate_headset_info->gpio_num, rc);
		return -1;
	}

	soulmate_headset_info->headset_state = gpio_get_value(soulmate_headset_info->gpio_num);
	printk(KERN_ERR "%s: current headset info is %d\n", __func__, soulmate_headset_info->headset_state);

	// we have to change this level trigger method
	if (soulmate_headset_info->headset_state) 	
		irqflags = IRQF_TRIGGER_LOW;
	else	
		irqflags = IRQF_TRIGGER_HIGH;
#if (0)
	rc = request_irq(MSM_GPIO_TO_INT(soulmate_headset_info->gpio_num), &soulmate_headset_isr, irqflags, "gpio_headset", soulmate_headset_info);
#else
	rc = request_irq(MSM_GPIO_TO_INT(soulmate_headset_info->gpio_num), &SoulMate_Headset_interrupt, irqflags, "gpio_headset", soulmate_headset_info);
#endif
	if (rc < 0) {
		printk(KERN_ERR "Could not register for interrupt (rc = %d)\n", rc);
		return -EIO;
	}

	set_irq_wake(MSM_GPIO_TO_INT(soulmate_headset_info->gpio_num), 1);

	disable_irq(MSM_GPIO_TO_INT(soulmate_headset_info->gpio_num));

	return	0;
}



static int __init headset_detect_init(void)
{
  int err;
	
  printk(KERN_INFO "soulmate headset probe start \n");


  // for headset detection 
  err = soulmate_headset_irq_init();	
  if (err == -EIO)
	goto fail3;
  else if (err == -1)
	goto fail2;



  // enable headset detect gpio interrupt
  enable_irq(MSM_GPIO_TO_INT(soulmate_headset_info->gpio_num));

 //printk(KERN_INFO "%s : soulmate headset probe successful! (0x%x)\n",__func__, headset_dev);
  printk(KERN_INFO "%s : soulmate headset probe successful! \n",__func__);

  return 0;

 fail3:	free_irq(IRQ_HEADSET_GPIO_PORT, NULL);
 //fail2:	input_free_device(headset_dev);
 fail2: 	printk(KERN_ERR "H2W: Failed to register driver\n");
 //fail1:		printk(KERN_ERR "%s : soulmate headset probe failed!\n",__func__);
 return err;
}

static void __exit headset_detect_exit(void)
{
	free_irq(IRQ_HEADSET_GPIO_PORT, NULL);
	//input_unregister_device(headset_dev);
////	switch_dev_unregister(&soulmate_headset_info->headset_dev);
	gpio_free(IRQ_HEADSET_GPIO_PORT);

    printk(KERN_ERR "headset: soulmate headset_detect_exit\n");
}


#if 0
module_init(headset_detect_init);
module_exit(headset_detect_exit);
#endif
late_initcall(headset_detect_init);

MODULE_AUTHOR("Dusin Jang <elecjang@pantech.com>");
MODULE_DESCRIPTION("soulmate headset detect driver");
MODULE_LICENSE("GPL");
