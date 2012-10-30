/**
 * @file pinmux.c
 * @brief the pinmux functions for A3
 * @author Lee Rong <lee.rong@amlogic.com>
 * @version 1.0.0
 * @date 2011-03-15
*/
/* Copyright (c) 2007-2011, Amlogic Inc.
 * All right reserved
 *
*/
#include <linux/module.h>
#include <plat/am_regs.h>
#include <plat/pinmux.h>

/* --------------------------------------------------------------------------*/
/**
* @brief  clear_mio_mux
*         clear the multi function io
*
* @param[in]  mux_index  the index of the pinmux registers
* @param[in]  mux_mask   the bit(s) you want clear
*/
/* --------------------------------------------------------------------------*/
void clear_mio_mux(unsigned mux_index, unsigned mux_mask)
{
    unsigned mux_reg[] = {PERIPHS_PIN_MUX_0, PERIPHS_PIN_MUX_1, PERIPHS_PIN_MUX_2, PERIPHS_PIN_MUX_3,
                          PERIPHS_PIN_MUX_4, PERIPHS_PIN_MUX_5, PERIPHS_PIN_MUX_6, PERIPHS_PIN_MUX_7,
                          PERIPHS_PIN_MUX_8, PERIPHS_PIN_MUX_9, PERIPHS_PIN_MUX_10, PERIPHS_PIN_MUX_11,
                          PERIPHS_PIN_MUX_12
                         };
    if (mux_index < 13) {
        CLEAR_PERI_REG_MASK(mux_reg[mux_index], mux_mask);
    }
}
EXPORT_SYMBOL(clear_mio_mux);

/* --------------------------------------------------------------------------*/
/**
* @brief  set_mio_mux
*         set the multi function io
*
* @param[in]  mux_index  the index of the pinmux registers
* @param[in]  mux_mask   the bit(s) you want set
*/
/* --------------------------------------------------------------------------*/
void set_mio_mux(unsigned mux_index, unsigned mux_mask)
{
    unsigned mux_reg[] = {PERIPHS_PIN_MUX_0, PERIPHS_PIN_MUX_1, PERIPHS_PIN_MUX_2, PERIPHS_PIN_MUX_3,
                          PERIPHS_PIN_MUX_4, PERIPHS_PIN_MUX_5, PERIPHS_PIN_MUX_6, PERIPHS_PIN_MUX_7,
                          PERIPHS_PIN_MUX_8, PERIPHS_PIN_MUX_9, PERIPHS_PIN_MUX_10, PERIPHS_PIN_MUX_11,
                          PERIPHS_PIN_MUX_12
                         };
    if (mux_index < 13) {
        SET_PERI_REG_MASK(mux_reg[mux_index], mux_mask);
    }
}
EXPORT_SYMBOL(set_mio_mux);

/* --------------------------------------------------------------------------*/
/**
* @brief  clearall_pinmux
*         call it before pinmux init;
*         call it before soft reset;
*/
/* --------------------------------------------------------------------------*/
void   clearall_pinmux(void)
{
    int i;

    for (i = 0; i < 13; i++) {
        clear_mio_mux(i, 0xffffffff);
    }
    return;
}
EXPORT_SYMBOL(clearall_pinmux);

/* --------------------------------------------------------------------------*/
/**
* @brief  eth_set_pinmux
*         ethernet pinmux setting
*
* @param[in]  bank_id        bank id of ethernet
* @param[in]  clk_in_out_id  clock ethernet setting
* @param[in]  ext_msk        use the other value to replace the MACRO
*
* @return  0:success  -1:fail
*/
/* --------------------------------------------------------------------------*/
void eth_set_pinmux(int bank_id, int clk_in_out_id, unsigned long ext_msk)
{

    switch (bank_id) {
    case    ETH_BANK0_GPIOC3_C11:
        if (ext_msk > 0) {
            set_mio_mux(ETH_BANK0_REG1, ext_msk);
        } else {
            set_mio_mux(ETH_BANK0_REG1, ETH_BANK0_REG1_VAL);
        }
        break;
    case    ETH_BANK1_GPIOD0_D8:
        if (ext_msk > 0) {
            set_mio_mux(ETH_BANK1_REG1, ext_msk);
        } else {
            set_mio_mux(ETH_BANK1_REG1, ETH_BANK1_REG1_VAL);
        }
        break;
    default:
        printk("UNknow pinmux setting of ethernet!error bankid=%d,must be 0-2\n", bank_id);
    }

    switch (clk_in_out_id) {
    case  ETH_CLK_IN_GPIOC2_REG2_18:
        set_mio_mux(2, 1 << 18);
        break;
    case  ETH_CLK_IN_GPIOD9_REG4_0:
        set_mio_mux(4, 1 << 0);
        break;
    case  ETH_CLK_OUT_GPIOC2_REG2_17:
        set_mio_mux(2, 1 << 17);
        break;
    case  ETH_CLK_OUT_GPIOD9_REG4_1:
        set_mio_mux(4, 1 << 1);
        break;
    default:
        printk("UNknow clk_in_out_id setting of ethernet!error clk_in_out_id=%d,must be 0-3\n", clk_in_out_id);
    }
}
EXPORT_SYMBOL(eth_set_pinmux);

/* --------------------------------------------------------------------------*/
/**
* @brief  uart_set_pinmux
*         uart pinmux setting
*
* @param[in]  port        UART_A/UART_B
* @param[in]  uart_bank   uart bank based on the platform.
*/
/* --------------------------------------------------------------------------*/
void uart_set_pinmux(int port, int uart_bank)
{

    if (port == UART_PORT_A) { /* UART_A */
        switch (uart_bank) {
        case UART_A_GPIO_A1_A2: /* GPIO A1_A2 */
            SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_6, (1 << 4) | (1 << 5));
            break;
        case UART_A_GPIO_B2_B3: /* GPIO B2_B3 */
            SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_6, (1 << 14) | (1 << 15));
            break;
        default:
            printk("UartA pinmux set error\n");
        }
    } else  if (port == UART_PORT_B) { /* UART_B */
        switch (uart_bank) {
        case UART_B_GPIO_B6_B7://GPIO B6_B7
            SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_6, (1 << 26) | (1 << 27));
            break;
        default:
            printk("UartB pinmux set error\n");
        }
    } else {
        printk("Unknow Uart Port%d!(0,1)\n", port);
    }
}

/* --------------------------------------------------------------------------*/
/**
* @brief  set_audio_pinmux
*         audio pinmux setting
*
* @param[in]  type
*/
/* --------------------------------------------------------------------------*/
void set_audio_pinmux(int type)
{
    switch (type) {
    case AUDIO_OUT_TEST_N:
        clear_mio_mux(1, (1 << 10));
        set_mio_mux(1, (1 << 11) | (1 << 12) | (1 << 13));
        set_mio_mux(5, (1 << 30));
        break;
    case AUDIO_OUT_GPIO_A17:
        clear_mio_mux(5, (1 << 30));
        set_mio_mux(1, (1 << 10) | (1 << 11) | (1 << 12) | (1 << 13));
        break;
    case AUDIO_IN_GPIO_A17:
        clear_mio_mux(1, (1 << 10) | (1 << 11) | (1 << 12) | (1 << 13));
        set_mio_mux(1, (1 << 14) | (1 << 15) | (1 << 16) | (1 << 17));
        break;
    case SPDIF_OUT_GPIO_A3:
        clear_mio_mux(0, (1 << 28));
        clear_mio_mux(1, (1 << 24));
        clear_mio_mux(1, (1 << 31));
        clear_mio_mux(5, (1 << 10));
        set_mio_mux(1, (1 << 31));
        break;
    case SPDIF_OUT_GPIO_A13:
        clear_mio_mux(1, (1 << 24));
        clear_mio_mux(1, (1 << 31));
        clear_mio_mux(5, (1 << 10));
        set_mio_mux(0, (1 << 28));
        break;
    case SPDIF_OUT_TEST_N:
        clear_mio_mux(0, (1 << 28));
        clear_mio_mux(1, (1 << 24));
        clear_mio_mux(1, (1 << 31));
        set_mio_mux(5, (1 << 10));
        break;
    case SPDIF_IN_GPIO_A12:
        set_mio_mux(0, (1 << 27));
        break;
    }
}
EXPORT_SYMBOL(set_audio_pinmux);
