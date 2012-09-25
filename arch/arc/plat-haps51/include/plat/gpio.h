/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __ARC_PLAT_GPIO_H
#define __ARC_PLAT_GPIO_H

#include <linux/errno.h>
#include <asm-generic/gpio.h>

#ifdef MAX_GPIOS
#undef MAX_GPIOS
#endif
#define MAX_GPIOS 32

/* PORT A */
#define GPIO_BUTTON_1   0  /* INPUT */
#define GPIO_BUTTON_2   1  /* INPUT */
#define GPIO_BUTTON_3   2  /* INPUT */
#define GPIO_BUTTON_4   3  /* INPUT */
#define GPIO_BUTTON_5   4  /* INPUT */
#define GPIO_BUTTON_6   5  /* INPUT */
#define GPIO_BUTTON_7   6  /* INPUT */
#define GPIO_BUTTON_8   7  /* INPUT */

/* PORT B */
#ifdef HAPS_EXTENSIONS
#define GPIO_OUTPUT_1   8  /* HEADER P0  - RESERVED FOR HAPS, HLT_ON_RST */
#define GPIO_OUTPUT_2   9  /* HEADER P1  - RESERVED FOR HAPS, SOFT_RST_EN */
#define GPIO_OUTPUT_3  10  /* HEADER P2  - RESERVED FOR HAPS */
#endif
#define GPIO_OUTPUT_4  11  /* HEADER P3  - INPUT & OUTPUT */
#define GPIO_OUTPUT_5  12  /* HEADER P4  - INPUT & OUTPUT */
#define GPIO_OUTPUT_6  13  /* HEADER P5  - INPUT & OUTPUT */
#define GPIO_OUTPUT_7  14  /* HEADER P6  - INPUT & OUTPUT */
#define GPIO_OUTPUT_8  15  /* HEADER P7  - INPUT & OUTPUT */

#ifdef HAPS_EXTENSIONS
#define GPIO_INPUT_1    8  /* HEADER P0  - RESERVED FOR HAPS, HLT_ON_RST */
#define GPIO_INPUT_2    9  /* HEADER P1  - RESERVED FOR HAPS, SOFT_RST_EN */
#define GPIO_INPUT_3   10  /* HEADER P2  - RESERVED FOR HAPS */
#endif
#define GPIO_INPUT_4   11  /* HEADER P3  - INPUT & OUTPUT */
#define GPIO_INPUT_5   12  /* HEADER P4  - INPUT & OUTPUT */
#define GPIO_INPUT_6   13  /* HEADER P5  - INPUT & OUTPUT */
#define GPIO_INPUT_7   14  /* HEADER P6  - INPUT & OUTPUT */
#define GPIO_INPUT_8   15  /* HEADER P7  - INPUT & OUTPUT */

/* PORT C */
#define GPIO_INPUT_9   16  /* HEADER P8  - INPUT */
#define GPIO_INPUT_10  17  /* HEADER P9  - INPUT */
#define GPIO_INPUT_11  18  /* HEADER P10 - INPUT */
#define GPIO_INPUT_12  19  /* HEADER P11 - INPUT */
#define GPIO_INPUT_13  20  /* HEADER P12 - INPUT */
#define GPIO_INPUT_14  21  /* HEADER P13 - INPUT */
#define GPIO_INPUT_15  22  /* HEADER P14 - INPUT */
#define GPIO_INPUT_16  23  /* HEADER P15 - INPUT */

#define GPIO_LED_1     16  /* OUTPUT */
#define GPIO_LED_2     17  /* OUTPUT */
#define GPIO_LED_3     18  /* OUTPUT */
#define GPIO_LED_4     19  /* OUTPUT */
#define GPIO_LED_5     20  /* OUTPUT */
#define GPIO_LED_6     21  /* OUTPUT */

/* PORT D */
#ifdef HAPS_RESERVED_EXTENSIONS
#define GPIO_IO_D1     24  /* NOT CONNECTED ON SYSTEM */
#define GPIO_IO_D2     25  /* NOT CONNECTED ON SYSTEM */
#define GPIO_IO_D3     26  /* NOT CONNECTED ON SYSTEM */
#define GPIO_IO_D4     27  /* NOT CONNECTED ON SYSTEM */
#define GPIO_IO_D5     28  /* NOT CONNECTED ON SYSTEM */
#define GPIO_IO_D6     29  /* NOT CONNECTED ON SYSTEM */
#define GPIO_IO_D7     30  /* NOT CONNECTED ON SYSTEM */
#define GPIO_IO_D8     31  /* NOT CONNECTED ON SYSTEM */
#endif

#ifdef CONFIG_GPIOLIB
static inline int gpio_get_value(unsigned int gpio)
{
	return __gpio_get_value(gpio);
}

static inline void gpio_set_value(unsigned int gpio, int value)
{
	__gpio_set_value(gpio, value);
}

static inline int gpio_cansleep(unsigned int gpio)
{
	return __gpio_cansleep(gpio);
}

static inline int gpio_to_irq(unsigned int gpio)
{
	return __gpio_to_irq(gpio);
}

static inline int irq_to_gpio(unsigned int irq)
{
	return -EINVAL;
}
#endif /* CONFIG_GPIOLIB */
#endif /* __ARC_PLAT_GPIO_H */
