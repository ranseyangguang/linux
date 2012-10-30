/**
 * @file pinmux.h
 * @brief the pinmux functions for A3
 * @author Lee Rong <lee.rong@amlogic.com>
 * @version 1.0.0
 * @date 2011-03-15
*/
/* Copyright (c) 2007-2011, Amlogic Inc.
 * All right reserved
 * 
*/

#ifndef _APOLLO_PINMUX_H_
#define _APOLLO_PINMUX_H_

/* General functions */
void clear_mio_mux(unsigned mux_index, unsigned mux_mask);
void set_mio_mux(unsigned mux_index, unsigned mux_mask);
void clearall_pinmux(void);


/* UART */
#define UART_PORT_A             (0)
#define UART_PORT_B             (1)

#define UART_A_GPIO_A1_A2       (0)
#define UART_A_GPIO_B2_B3       (1)

#define UART_B_GPIO_B6_B7       (2)

void uart_set_pinmux(int port, int uart_bank);

/* Ethernet */
/*
"RMII_MDIOREG2[8]"
"RMII_MDCREG2[9]"
"RMII_TX_DATA0REG2[10]"
"RMII_TX_DATA1REG2[11]"
"RMII_TX_ENREG2[12]"
"RMII_RX_DATA0REG2[13]"
"RMII_RX_DATA1REG2[14]"
"RMII_RX_CRS_DVREG2[15]"
"RMII_RX_ERRREG2[16]"
Bank0_GPIOC3-C11
*/
#define ETH_BANK0_GPIOC3_C11		0
#define ETH_BANK0_REG1			2
#define ETH_BANK0_REG1_VAL		(0x1ff<<8)
/*
"RMII_MDIOREG4[10]"
"RMII_MDCREG4[9]"
"RMII_TX_DATA0REG4[8]"
"RMII_TX_DATA1REG4[7]"
"RMII_TX_ENREG4[6]"
"RMII_RX_DATA0REG4[5]"
"RMII_RX_DATA1REG4[4]"
"RMII_RX_CRS_DVREG4[3]"
"RMII_RX_ERRREG4[2]"
Bank1_GPIOD0-D8
*/
#define ETH_BANK1_GPIOD0_D8		1
#define ETH_BANK1_REG1			4
#define ETH_BANK1_REG1_VAL		(0x1ff<<2)

#define ETH_CLK_IN_GPIOC2_REG2_18	0
#define ETH_CLK_IN_GPIOD9_REG4_0	1

#define ETH_CLK_OUT_GPIOC2_REG2_17	2
#define ETH_CLK_OUT_GPIOD9_REG4_1	3

void eth_set_pinmux(int bank_id, int clk_in_out_id, unsigned long ext_msk);

/*AUDIO*/

#define AUDIO_OUT_TEST_N            0
#define AUDIO_OUT_GPIO_A17          1
#define AUDIO_IN_GPIO_A17           2
#define SPDIF_OUT_GPIO_A3           3
#define SPDIF_OUT_GPIO_A13          4
#define SPDIF_OUT_TEST_N            5
#define SPDIF_IN_GPIO_A12           6

void set_audio_pinmux(int type);

#endif //_APOLLO_PINMUX_H_
