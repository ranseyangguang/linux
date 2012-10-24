/*
 *  arch/arc/arch-apollo-a3/include/mach/clock.h
 *
 *  Copyright (C) 2011 AMLOGIC, INC.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __ARCH_ARC_APOLLO_CLOCK_H
#define __ARCH_ARC_APOLLO_CLOCK_H



// -----------------------------------------
// clk_util_clk_msr
// -----------------------------------------
// from twister_core.v
//        .clk_to_msr_in          ( { 1'b0,                       // [31]
//                                    1'b0,                       // [30]
//                                    clk_double50_msr,           // [29]
//                                    cts_sar_adc_clk,            // [28]
//                                    cts_audac_clkpi,            // [27]
//                                    sc_clk_int,                 // [26]
//                                    usb_clk_12mhz,              // [25]
//                                    lvds_fifo_clk,              // [24]
//                                    HDMI_CH3_TMDSCLK,           // [23]
//                                    mod_eth_clk50_i,            // [22]
#define CLK_MOD_ETH_CLK50_I             22
//                                    mod_audin_amclk_i,          // [21]
//                                    cts_btclk27,                // [20]
//                                    cts_hdmi_sys_clk,           // [19]
//                                    cts_led_pll_clk,            // [18]
//                                    cts_vghl_pll_clk,           // [17]
//                                    cts_FEC_CLK_2,              // [16]
//                                    cts_FEC_CLK_1,              // [15]
//                                    cts_FEC_CLK_0,              // [14]
//                                    cts_amclk,                  // [13]
//                                    1'b0,                       // [12]
//                                    cts_eth_rmii,               // [11]
//                                    cts_dclk,                   // [10]
//                                    cts_sys_cpu_clk,            // [9]
#define CLK_SYS                         9
//                                    cts_media_cpu_clk,          // [9]
//                                    clk81,                      // [7]
#define CLK_CLK81                       7
//                                    vid_pll_clk,                // [6]
//                                    aud_pll_clk,                // [5]
//                                    misc_pll_clk,               // [4]
#define CLK_MISC_PLL                    4
//                                    ddr_pll_clk,                // [3]
//                                    sys_pll_clk,                // [2]
//                                    am_ring_osc_clk_out[1],     // [1]
//                                    am_ring_osc_clk_out[0]} ),  // [0]
// For Example
//
// unsigend long    clk81_clk   = clk_util_clk_msr( 2,      // mux select 2
//                                                  50 );   // measure for 50uS
//
// returns a value in "clk81_clk" in Hz
//
// The "uS_gate_time" can be anything between 1uS and 65535 uS, but the limitation is
// the circuit will only count 65536 clocks.  Therefore the uS_gate_time is limited by
//
//   uS_gate_time <= 65535/(expect clock frequency in MHz)
//
// For example, if the expected frequency is 400Mhz, then the uS_gate_time should
// be less than 163.
//
// Your measurement resolution is:
//
//    100% / (uS_gate_time * measure_val )
//

/*
        7-2*XTAL
        6-PAD
        5-video_pll_clk
        4-audio_pll_clk
        3-ddr_pll_clk
        2-misc_pll_clk
        1-sys_pll_clk
        0-XTAL

        clk_freq:50M=50000000
        output_clk:50000000;
        aways,maybe changed for others?
*/





#define ETH_CLKSRC_XTAL                 (0)
#define ETH_CLKSRC_SYS_CLK              (1)
#define ETH_CLKSRC_MISC_CLK             (2)
#define ETH_CLKSRC_DDR_CLK              (3)
#define ETH_CLKSRC_AUDIO_CLK            (4)
#define ETH_CLKSRC_VIDEO_CLK            (5)
#define ETH_CLKSRC_EXTERN_PAD_CLK       (6)
#define ETH_CLKSRC_2XTAL                (7)
#define CLK_1M                          (1000000)
#define ETH_VALIDE_CLKSRC(clk,out_clk)  ((clk%out_clk)==0)

struct clk {
    const char *name;
    unsigned long rate;
    unsigned long min;
    unsigned long max;
    int source_clk;
    /* for clock gate */
    unsigned char clock_index;
    unsigned clock_gate_reg_adr;
    unsigned clock_gate_reg_mask;
    /**/
    unsigned long(*get_rate)(struct clk *);
    int (*set_rate)(struct clk *, unsigned long);
};

struct pll_reg_table {
    unsigned  long xtal_clk;
    unsigned  long out_clk;
    unsigned  long settings;
};

int eth_clk_set(int selectclk, unsigned long clk_freq, unsigned long out_clk);
int sys_clkpll_setting(unsigned crystal_freq, unsigned out_freq);
unsigned long get_xtal_clock(void);
int other_pll_setting(unsigned crystal_freq, unsigned  out_freq);
int audio_pll_setting(unsigned crystal_freq, unsigned  out_freq);
int video_pll_setting(unsigned crystal_freq, unsigned  out_freq, int powerdown, int flags);

int set_usb_phy_clk(int rate);
int set_sata_phy_clk(int sel);

unsigned int get_mpeg_clk(void );
unsigned int get_system_clk(void );
unsigned int get_misc_pll_clk(void );
unsigned int get_ethernet_pad_clk(void);

#endif //__ARCH_ARC_APOLLO_CLOCK_H
