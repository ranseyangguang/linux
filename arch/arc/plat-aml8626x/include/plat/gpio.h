#ifndef __MESON_GPIO_H__
#define	 __MESON_GPIO_H__
/*
// Pre-defined GPIO addresses
// ----------------------------
#define PREG_EGPIO_EN_N                            0x200c
#define PREG_EGPIO_O                               0x200d
#define PREG_EGPIO_I                               0x200e
// ----------------------------
#define PREG_FGPIO_EN_N                            0x200f
#define PREG_FGPIO_O                               0x2010
#define PREG_FGPIO_I                               0x2011
// ----------------------------
#define PREG_GGPIO_EN_N                            0x2012
#define PREG_GGPIO_O                               0x2013
#define PREG_GGPIO_I                               0x2014
// ----------------------------
#define PREG_HGPIO_EN_N                            0x2015
#define PREG_HGPIO_O                               0x2016
#define PREG_HGPIO_I                               0x2017
// ----------------------------
*/

#ifdef CONFIG_TCA6424
#include <linux/tca6424.h>
#define CONFIG_EXGPIO
#endif

#ifdef CONFIG_SN7325
#include <linux/sn7325.h>
#ifndef CONFIG_EXGPIO
#define CONFIG_EXGPIO
#endif
#endif

typedef enum gpio_bank
{
	PREG_EGPIO=0,
	PREG_FGPIO,
	PREG_GGPIO,
	PREG_HGPIO,
	PREG_JTAG_GPIO,
#ifdef CONFIG_EXGPIO
	EXGPIO_BANK0,
	EXGPIO_BANK1,
	EXGPIO_BANK2,
	EXGPIO_BANK3
#endif
}gpio_bank_t;

typedef  enum  gpio_group{
	GPIOA_0=0,
	GPIOA_1,
	GPIOA_2,
	GPIOA_3,
	GPIOA_4,
	GPIOA_5,
	GPIOA_6,
	GPIOA_7,
	GPIOA_8,
	GPIOA_9,
	GPIOA_10,
	GPIOA_11,
	GPIOA_12,
	GPIOA_13,
	GPIOA_14,
	GPIOA_15,
	GPIOA_16,
	GPIOA_17,

	GPIOB_0,
	GPIOB_1,
	GPIOB_2,
	GPIOB_3,
	GPIOB_4,
	GPIOB_5,
	GPIOB_6,
	GPIOB_7,

	GPIOC_0,
	GPIOC_1,
	GPIOC_2,
	GPIOC_3,
	GPIOC_4,
	GPIOC_5,
	GPIOC_6,
	GPIOC_7,
	GPIOC_8,
	GPIOC_9,
	GPIOC_10,
	GPIOC_11,
	GPIOC_12,
	GPIOC_13,
	GPIOC_14,
	GPIOC_15,
	GPIOC_16,
	GPIOC_17,
	GPIOC_18,
	GPIOC_19,
	GPIOC_20,
	GPIOC_21,
	GPIOC_22,
	GPIOC_23,

	GPIOD_0,
	GPIOD_1,
	GPIOD_2,
	GPIOD_3,
	GPIOD_4,
	GPIOD_5,
	GPIOD_6,
	GPIOD_7,
	GPIOD_8,
	GPIOD_9,
	GPIOD_10,
	GPIOD_11,

	GPIOE_0,
	GPIOE_1,
	GPIOE_2,
	GPIOE_3,
	GPIOE_4,
	GPIOE_5,
	GPIOE_6,
	GPIOE_7,
	GPIOE_8,
	GPIOE_9,
	GPIOE_10,
	GPIOE_11,
	GPIOE_12,
	GPIOE_13,
	GPIOE_14,
	GPIOE_15,

	GPIOF_0,
	GPIOF_1,
	GPIOF_2,
	GPIOF_3,
	GPIOF_4,
	GPIOF_5,
	GPIOF_6,
	GPIOF_7,
	GPIOF_8,
	GPIOF_9,

	MAX_GROUP_INDEX,
}gpio_group_t;

typedef enum gpio_mode
{
	GPIO_OUTPUT_MODE,
	GPIO_INPUT_MODE,
}gpio_mode_t;

int set_gpio_mode(gpio_bank_t bank,int bit,gpio_mode_t mode);
gpio_mode_t get_gpio_mode(gpio_bank_t bank,int bit);

int set_gpio_val(gpio_bank_t bank,int bit,unsigned long val);
unsigned long  get_gpio_val(gpio_bank_t bank,int bit);


#define GPIOA_bank_bit(bit)		(PREG_EGPIO)

#define GPIOA_BANK				(PREG_EGPIO)
#define GPIOA_bit(bit)				(bit)
#define GPIOA_MAX_BIT			(17)

#define GPIOB_BANK				(PREG_FGPIO)
#define GPIOB_bit(bit)				(bit)	
#define GPIOB_MAX_BIT			(7)

#define GPIOC_BANK				(PREG_FGPIO)
#define GPIOC_bit(bit)				(bit+8)	
#define GPIOC_MAX_BIT			(23)

#define GPIOD_BANK				(PREG_GGPIO)
#define GPIOD_bit(bit)				(bit+16)	
#define GPIOD_MAX_BIT			(11)

#define GPIOE_BANK				(PREG_GGPIO)
#define GPIOE_bit(bit)				(bit)	
#define GPIOE_MAX_BIT			(15)

#define GPIOF_BANK				(PREG_EGPIO)
#define GPIOF_bit(bit)				(bit+18)	
#define GPIOF_MAX_BIT			(9)
		

#define GPIOJTAG_bank_bit(bit)		(PREG_JTAG_GPIO)
#define GPIOJTAG_bit_bit0_3(bit)	(bit)
#define GPIOJTAG_bit_bit16(bit)		(bit)

#define GPIO_JTAG_TCK_BIT			0
#define GPIO_JTAG_TMS_BIT			1
#define GPIO_JTAG_TDI_BIT			2
#define GPIO_JTAG_TDO_BIT			3
#define GPIO_TEST_N_BIT				16

enum {
    GPIOA_IDX = 70,
    GPIOB_IDX = 52,
    GPIOC_IDX = 28,
    GPIOD_IDX = 16,
    GPIOE_IDX = 0,
};

extern int	control_gpio_function(u32  group_index,u32  io_enable);
extern int gpio_to_idx(unsigned gpio);

/**
 * enable gpio edge interrupt
 *	
 * @param [in] pin  index number of the chip, start with 0 up to 255 
 * @param [in] flag rising(0) or falling(1) edge 
 * @param [in] group  this interrupt belong to which interrupt group  from 0 to 7
 */
extern void gpio_enable_edge_int(int pin , int flag, int group);
/**
 * enable gpio level interrupt
 *	
 * @param [in] pin  index number of the chip, start with 0 up to 255 
 * @param [in] flag high(0) or low(1) level 
 * @param [in] group  this interrupt belong to which interrupt group  from 0 to 7
 */
extern void gpio_enable_level_int(int pin , int flag, int group);

/**
 * enable gpio interrupt filter
 *
 * @param [in] filter from 0~7(*222ns)
 * @param [in] group  this interrupt belong to which interrupt group  from 0 to 7
 */
extern void gpio_enable_int_filter(int filter, int group);

extern int gpio_is_valid(int number);
extern int gpio_request(unsigned gpio, const char *label);
extern void gpio_free(unsigned gpio);
extern int gpio_direction_input(unsigned gpio);
extern int gpio_direction_output(unsigned gpio, int value);
extern void gpio_set_value(unsigned gpio, int value);
extern int gpio_get_value(unsigned gpio);


#ifdef CONFIG_EXGPIO
#define MAX_EXGPIO_BANK 4

static inline unsigned char get_exgpio_port(gpio_bank_t bank)
{
    unsigned char port = bank - EXGPIO_BANK0;
    
    return (port >= MAX_EXGPIO_BANK)? MAX_EXGPIO_BANK : port;        
}

static inline int set_exgpio_mode(unsigned char port,int bit,gpio_mode_t mode)
{
    if( port == MAX_EXGPIO_BANK )
        return -1;
    
    int bank_mode = get_configIO(port);
    
    bank_mode &= ~(1 << bit);
    bank_mode |= mode << bit;
    configIO(port, bank_mode);
    return 0;
}

static inline gpio_mode_t get_exgpio_mode(unsigned char port,int bit)
{
    if( port == MAX_EXGPIO_BANK )
        return -1;
    return (get_configIO(port) >> bit) & 1;
}

static inline int set_exgpio_val(unsigned char port,int bit,unsigned long val)
{
    if( port == MAX_EXGPIO_BANK )
        return -1;
    
#ifdef CONFIG_TCA6424
    int bank_val = getIO_level(port);
    
    bank_val &= ~(1 << bit);
    bank_val |= val << bit;
    setIO_level(port, bank_val);
#endif

#ifdef CONFIG_SN7325
    setIO_level(port, val, bit);
#endif
    return 0;
}

static inline unsigned long  get_exgpio_val(unsigned char port, int bit)
{
    if( port == MAX_EXGPIO_BANK )
        return -1;
#ifdef CONFIG_TCA6424    
    return (getIO_level(port) >> bit) & 1;
#endif

#ifdef CONFIG_SN7325
    return getIObit_level(port, bit);
#endif
}
#endif

#endif
