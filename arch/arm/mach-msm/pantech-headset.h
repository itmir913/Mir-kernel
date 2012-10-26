/*
 * Support for the Headset Detection(GPIO IRQ).
 * These are implemented in linux kernel 2.6ver
 *
 * Copyright (c) 2009 Jang Dusin <elecjang@pantech.com>
 *
 * This file may be distributed under the terms of the GNU GPL license.
 */

#define IRQ_HEADSET_GPIO_PORT 39

struct generic_switch_platform_data {
	const char *name;

	/* if NULL, switch_dev.name will be printed */
	const char *name_on;
	const char *name_off;
	/* if NULL, "0" or "1" will be printed */
	const char *state_on;
	const char *state_off;
};
