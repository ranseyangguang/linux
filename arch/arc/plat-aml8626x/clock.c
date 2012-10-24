/*
 *
 * arch/arc/arch-apollo-a3/clock.c
 *
 *  Copyright (C) 2010 AMLOGIC, INC.
 *
 * License terms: GNU General Public License (GPL) version 2
 * Define clocks in the app platform.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

#include <plat/clkdev.h>
#include <plat/clock.h>
#include <plat/am_regs.h>
#include <plat/power_gate.h>

static DEFINE_SPINLOCK(clockfw_lock);

#ifdef CONFIG_INIT_CPU_CLOCK_FREQ
static unsigned long __initdata init_clock = CONFIG_INIT_CPU_CLOCK;
#else
static unsigned long __initdata init_clock = 0;
#endif

unsigned long get_xtal_clock(void)
{
    unsigned long clk;

    clk = READ_CBUS_REG_BITS(PREG_CTLREG0_ADDR, 4, 5);
    clk = clk * 1000 * 1000;
    return clk;
}

/*
Get two number's max common divisor;
*/

static int get_max_common_divisor(int a, int b)
{
    while (b) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}


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

int  eth_clk_set(int selectclk, unsigned long clk_freq, unsigned long out_clk)

{

    int n;

    printk("select eth clk-%d,source=%ld,out=%ld\n", selectclk, clk_freq, out_clk);

    if (((clk_freq) % out_clk) != 0)

    {
        printk("ERROR:source clk must n times of out_clk = %ld ,source clk = %ld\n", out_clk, clk_freq);

        return -1;

    } else {
        n = (int)((clk_freq) / out_clk);

    }

    WRITE_CBUS_REG(HHI_ETH_CLK_CNTL,
                   (n - 1) << 0 |
                   selectclk << 9 |
                   1 << 8 //enable clk
                  );

    udelay(100);

    return 0;

}

int auto_select_eth_clk(void)
{

    return -1;

}


int sys_clkpll_setting(unsigned crystal_freq, unsigned  out_freq)
{
    int n, m;
    unsigned long crys_M, out_M, middle_freq, flags;
    if (!crystal_freq) {
        crystal_freq = get_xtal_clock();
    }
    crys_M = crystal_freq / 1000000;
    out_M = out_freq / 1000000;
    middle_freq = get_max_common_divisor(crys_M, out_M);
    n = crys_M / middle_freq;
    m = out_M / (middle_freq);
    if (n > (1 << 5) - 1) {
        printk(KERN_ERR "sys_clk_setting  error, n is too bigger n=%d,crys_M=%ldM,out=%ldM\n",n,crys_M,out_M);
        return -1;
    }
    if (m > (1 << 9) - 1) {
        printk(KERN_ERR "sys_clk_setting  error, m is too bigger m=%d,crys_M=%ldM,out=%ldM\n",m,crys_M,out_M);
        return -2;
    }
    if (out_freq > 1300 * CLK_1M || out_freq < 700 * CLK_1M) {
        printk(KERN_WARNING"sys_clk_setting  warning,VCO may no support out_freq,crys_M=%ldM,out=%ldM\n", crys_M, out_M);
    }
    printk(KERN_INFO "cpu_clk_setting crystal_req=%ld,out_freq=%ld,n=%d,m=%d\n",crys_M,out_M,n,m);
    local_irq_save(flags);
    WRITE_CBUS_REG(HHI_SYS_PLL_CNTL, m << 0 | n << 9); // system PLL
    WRITE_CBUS_REG(HHI_SYS_CPU_CLK_CNTL,  // cpu clk set to system clock/2
                        (1 << 12) |       // clock gate
                        (1 << 9 ) |       // select sys pll
                        (1 << 7 ) |       // 1 - sys/audio pll clk, 0 - XTAL
                        (2-1));           //divider = 2
    udelay(10);
    local_irq_restore(flags);
    return 0;
}

int misc_pll_setting(unsigned crystal_freq, unsigned  out_freq)
{
    int n, m, od;
    unsigned long crys_M, out_M, middle_freq;
    if (!crystal_freq) {
        crystal_freq = get_xtal_clock();
    }
    crys_M = crystal_freq / 1000000;
    out_M = out_freq / 1000000;
    if (out_M < 400) {
        /*if <400M, Od=1*/
        od = 1;/*out=pll_out/(1<<od)
                 */
        out_M = out_M << 1;
    } else {
        od = 0;
    }

    middle_freq = get_max_common_divisor(crys_M, out_M);
    n = crys_M / middle_freq;
    m = out_M / (middle_freq);
    if (n > (1 << 5) - 1) {
        printk(KERN_ERR "misc_pll_setting  error, n is too bigger n=%d,crys_M=%ldM,out=%ldM\n",
               n, crys_M, out_M);
        return -1;
    }
    if (m > (1 << 9) - 1) {
        printk(KERN_ERR "misc_pll_setting  error, m is too bigger m=%d,crys_M=%ldM,out=%ldM\n",
               m, crys_M, out_M);
        return -2;
    }
    WRITE_CBUS_REG(HHI_OTHER_PLL_CNTL,
                   m |
                   n << 9 |
                   (od & 1) << 16
                  ); // other PLL
    printk(KERN_INFO "other pll setting to crystal_req=%ld,out_freq=%ld,n=%d,m=%d,od=%d\n", crys_M, out_M / (od + 1), n, m, od);
    return 0;
}


int audio_pll_setting(unsigned crystal_freq, unsigned  out_freq)
{
    int n, m, od;
    unsigned long crys_M, out_M, middle_freq;
    /*
    FIXME:If we need can't exact setting this clock,Can used a pll table?
    */
    if (!crystal_freq) {
        crystal_freq = get_xtal_clock();
    }
    crys_M = crystal_freq / 1000000;
    out_M = out_freq / 1000000;
    if (out_M < 400) {
        /*if <400M, Od=1*/
        od = 1;/*out=pll_out/(1<<od)
                 */
        out_M = out_M << 1;
    } else {
        od = 0;
    }
    middle_freq = get_max_common_divisor(crys_M, out_M);
    n = crys_M / middle_freq;
    m = out_M / (middle_freq);
    if (n > (1 << 5) - 1) {
        printk(KERN_ERR "audio_pll_setting  error, n is too bigger n=%d,crys_M=%ldM,out=%ldM\n",
               n, crys_M, out_M);
        return -1;
    }
    if (m > (1 << 9) - 1) {
        printk(KERN_ERR "audio_pll_setting  error, m is too bigger m=%d,crys_M=%ldM,out=%ldM\n",
               m, crys_M, out_M);
        return -2;
    }
    WRITE_CBUS_REG(HHI_AUD_PLL_CNTL,
                   m |
                   n << 9 |
                   (od & 1) << 14
                  ); // other PLL
    printk(KERN_INFO "audio_pll_setting to crystal_req=%ld,out_freq=%ld,n=%d,m=%d,od=%d\n", crys_M, out_M / (od + 1), n, m, od);
    return 0;
}

int video_pll_setting(unsigned crystal_freq, unsigned  out_freq, int powerdown, int flags)
{
    int n, m;
    unsigned int od = 0;
    unsigned long crys_M, out_M, middle_freq;
    int ret = 0;
    /*
    flags can used for od1/xd settings
    FIXME:If we need can't exact setting this clock,Can used a pll table?
    */
    if (!crystal_freq) {
        crystal_freq = get_xtal_clock();
    }
    crys_M = crystal_freq / 1000000;
    out_M = out_freq / 1000000;

    /*the A3 video pll support from 1G-1.5GHz*/
    while (out_M<1000) {
	od++;
	out_M <<= od;
    }
    
    if (out_M > 1500) {
        printk(KERN_WARNING"%s warning, VCO may no support out_freq,crys_M=%ldM,out=%ldM\n", __FUNCTION__, crys_M, out_M);
    }

    middle_freq = get_max_common_divisor(crys_M, out_M);
    n = crys_M / middle_freq;
    m = out_M / (middle_freq);
    if (n > (1 << 5) - 1) {
        printk(KERN_ERR "video_pll_setting  error, n is too bigger n=%d,crys_M=%ldM,out=%ldM\n",
               n, crys_M, out_M);
        ret = -1;
    }
    if (m > (1 << 9) - 1) {
        printk(KERN_ERR "video_pll_setting  error, m is too bigger m=%d,crys_M=%ldM,out=%ldM\n",
               m, crys_M, out_M);
        ret = -2;
    }
    if (ret) {
        return ret;
    }

    WRITE_CBUS_REG(HHI_VID_PLL_CNTL,
                   m |
                   n << 9 |
                   (od & 1) << 16 |
                   (!!powerdown) << 15 /*is power down mode?*/
                  ); // other PLL
    printk(KERN_INFO "video_pll_setting to crystal_req=%ld,out_freq=%ld,n=%d,m=%d,od=%d\n", crys_M, out_M / (od + 1), n, m, od);

    return 0;
}



long clk_round_rate(struct clk *clk, unsigned long rate)
{
    if (rate < clk->min) {
        return clk->min;
    }
    if (rate > clk->max) {
        return clk->max;
    }
    return rate;
}
EXPORT_SYMBOL(clk_round_rate);


unsigned long clk_get_rate(struct clk *clk)
{
    if (!clk) {
        return 0;
    }
    if (clk->get_rate) {
        return clk->get_rate(clk);
    }
    return clk->rate;
}
EXPORT_SYMBOL(clk_get_rate);

int clk_set_rate(struct clk *clk, unsigned long rate)
{
    unsigned long flags;
    int ret = -EINVAL;

    if (clk == NULL || clk->set_rate == NULL) {
        return ret;
    }

    spin_lock_irqsave(&clockfw_lock, flags);

    ret = clk->set_rate(clk, rate);

    spin_unlock_irqrestore(&clockfw_lock, flags);

    return ret;
}
EXPORT_SYMBOL(clk_set_rate);

static unsigned long xtal_get_rate(struct clk *clk)
{
    unsigned long rate;
    rate = get_xtal_clock(); /*refresh from register*/
    clk->rate = rate;
    return rate;
}

static int clk_set_rate_sys_pll(struct clk *clk, unsigned long rate)
{
    unsigned long r = rate;
    int ret = -EINVAL;

    if (r < 1000) {
        r = r * 1000000;
    }
    ret = sys_clkpll_setting(0, r);
    return ret;
}

static int clk_set_rate_misc_pll(struct clk *clk, unsigned long rate)
{
    unsigned long r = rate;
    int ret = -EINVAL;

    if (r < 1000) {
        r = r * 1000000;
    }
    ret = misc_pll_setting(0, r);
    return ret;
}

static int clk_set_rate_clk81(struct clk *clk, unsigned long rate)
{
    unsigned long r = rate;
    struct clk *father_clk;
    unsigned long r1;
    int ret = -1;

    if (r < 1000) {
        r = r * 1000000;
    }

    father_clk = clk_get_sys("clk_misc_pll", NULL);
    r1 = clk_get_rate(father_clk);
    if (r1 != r * 4) {
        ret = father_clk->set_rate(father_clk, r * 4);
        if (ret != 0) {
            return ret;
        }
    }
    clk->rate = r;
    /*for current it is alway equal=misc_clk/4*/
    WRITE_CBUS_REG(HHI_MPEG_CLK_CNTL,   // MPEG clk81 set to misc/4
                   (2 << 12) |          // select misc PLL
                   ((4 - 1) << 0) |     // div1
                   (1 << 7) |           // cntl_hi_mpeg_div_en, enable gating
                   (1 << 8)             // Connect clk81 to the PLL divider output
                  );
    return 0;
}


static int clk_set_rate_cpu_clk(struct clk *clk, unsigned long rate)
{
    unsigned long r = rate;
    struct clk *father_clk;
    unsigned long r1;
    int ret = -1;

    if (r < 1000) {
        r = r * 1000000;
    }
    father_clk = clk_get_sys("clk_sys_pll", NULL);
    r1 = clk_get_rate(father_clk);
    if (!r1) {
        return -1;
    }
    if (r1 != r * 2 && r != 0) {
        ret = father_clk->set_rate(father_clk, r * 2);
        if (ret != 0) {
            return ret;
        }
    }
    clk->rate = r;
    /*for current it is alway equal=sys_pll/2*/
    WRITE_CBUS_REG(HHI_SYS_PLL_CNTL,               // CPU clk set to system clock/2
                   (1 << 9) |                      // 1 - sys_pll_clk, 2 - misc_pll_clk
                   (1 << 7) |                      // 1 - sys/misc pll clk, 0 - XTAL
                   (0 << 2));                      // div1
    return 0;
}
static int clk_set_rate_audio_clk(struct clk *clk, unsigned long rate)
{
    unsigned long r = rate;
    int ret = -EINVAL;

    if (r < 1000) {
        r = r * 1000000;
    }
    ret = audio_pll_setting(0, r);
    return ret;
}

static int clk_set_rate_video_clk(struct clk *clk, unsigned long rate)
{
    unsigned long r = rate;
    int ret = -EINVAL;

    if (r < 1000) {
        r = r * 1000000;
    }
    ret = video_pll_setting(0, r , 0, 0);
    return ret;
}

static struct clk xtal_clk = {
    .name       = "clk_xtal",
    .rate       = 24000000,
    .get_rate   = xtal_get_rate,
    .set_rate   = NULL,
};

static struct clk clk_sys_pll = {
    .name       = "clk_sys_pll",
    .rate       = 1200000000,
    .min        =  700000000,
    .max        = 1300000000,
    .set_rate   = clk_set_rate_sys_pll,
};

static struct clk clk_misc_pll = {
    .name       = "clk_misc_pll",
    .rate       = 800000000,
    .min        = 400000000,
    .max        = 800000000,
    .set_rate   = clk_set_rate_misc_pll,
};


static struct clk clk_ddr_pll = {
    .name       = "clk_ddr",
    .rate       = 400000000,
    .set_rate   = NULL,
};


static struct clk clk81 = {
    .name       = "clk81",
    .rate       = 200000000,
    .min        = 100000000,
    .max        = 400000000,
    .set_rate   = clk_set_rate_clk81,
};


static struct clk cpu_clk = {
    .name       = "cpu_clk",
    .rate       = 600000000,
    .min        = 200000000,
    .max        = 800000000,
    .set_rate   = clk_set_rate_cpu_clk,
};

static struct clk audio_clk = {
    .name       = "audio_clk",
    .rate       = 300000000,
    .min        = 200000000,
    .max        = 1000000000,
    .set_rate   = clk_set_rate_audio_clk,
};

static struct clk video_clk = {
    .name       = "video_clk",
    .rate       = 300000000,
    .min        = 100000000,
    .max        = 750000000,
    .set_rate   = clk_set_rate_video_clk,
};

REGISTER_CLK(AHB_BRIDGE);
REGISTER_CLK(AHB_SRAM);
REGISTER_CLK(AIU_ADC);
REGISTER_CLK(AIU_MIXER_REG);
REGISTER_CLK(AIU_AUD_MIXER);
REGISTER_CLK(AIU_AIFIFO2);
REGISTER_CLK(AIU_AMCLK_MEASURE);
REGISTER_CLK(AIU_I2S_OUT);
REGISTER_CLK(AIU_IEC958);
REGISTER_CLK(AIU_AI_TOP_GLUE);
REGISTER_CLK(AIU_AUD_DAC);
REGISTER_CLK(AIU_ICE958_AMCLK);
REGISTER_CLK(AIU_I2S_DAC_AMCLK);
REGISTER_CLK(AIU_I2S_SLOW);
REGISTER_CLK(AIU_AUD_DAC_CLK);
REGISTER_CLK(ASSIST_MISC);
REGISTER_CLK(AMRISC);
REGISTER_CLK(AUD_BUF);
REGISTER_CLK(AUD_IN);
REGISTER_CLK(BLK_MOV);
REGISTER_CLK(BT656_IN);
REGISTER_CLK(DEMUX);
REGISTER_CLK(MMC_DDR);
REGISTER_CLK(DDR);
REGISTER_CLK(ETHERNET);
REGISTER_CLK(GE2D);
REGISTER_CLK(HDMI_MPEG_DOMAIN);
REGISTER_CLK(HIU_PARSER_TOP);
REGISTER_CLK(HIU_PARSER);
REGISTER_CLK(ISA);
REGISTER_CLK(MEDIA_CPU);
REGISTER_CLK(MISC_USB0_TO_DDR);
REGISTER_CLK(MISC_USB1_TO_DDR);
REGISTER_CLK(MISC_SATA_TO_DDR);
REGISTER_CLK(AHB_CONTROL_BUS);
REGISTER_CLK(AHB_DATA_BUS);
REGISTER_CLK(AXI_BUS);
REGISTER_CLK(ROM_CLK);
REGISTER_CLK(EFUSE);
REGISTER_CLK(AHB_ARB0);
REGISTER_CLK(RESET);
REGISTER_CLK(MDEC_CLK_PIC_DC);
REGISTER_CLK(MDEC_CLK_DBLK);
REGISTER_CLK(MDEC_CLK_PSC);
REGISTER_CLK(MDEC_CLK_ASSIST);
REGISTER_CLK(MC_CLK);
REGISTER_CLK(IQIDCT_CLK);
REGISTER_CLK(VLD_CLK);
REGISTER_CLK(NAND);
REGISTER_CLK(RESERVED0);
REGISTER_CLK(VGHL_PWM);
REGISTER_CLK(LED_PWM);
REGISTER_CLK(UART1);
REGISTER_CLK(SDIO);
REGISTER_CLK(ASYNC_FIFO);
REGISTER_CLK(STREAM);
REGISTER_CLK(RTC);
REGISTER_CLK(UART0);
REGISTER_CLK(RANDOM_NUM_GEN);
REGISTER_CLK(SMART_CARD_MPEG_DOMAIN);
REGISTER_CLK(SMART_CARD);
REGISTER_CLK(SAR_ADC);
REGISTER_CLK(I2C);
REGISTER_CLK(IR_REMOTE);
REGISTER_CLK(_1200XXX);
REGISTER_CLK(SATA);
REGISTER_CLK(SPI1);
REGISTER_CLK(USB1);
REGISTER_CLK(USB0);
REGISTER_CLK(VI_CORE);
REGISTER_CLK(LCD);
REGISTER_CLK(ENC480P_MPEG_DOMAIN);
REGISTER_CLK(ENC480I);
REGISTER_CLK(VENC_MISC);
REGISTER_CLK(ENC480P);
REGISTER_CLK(HDMI);
REGISTER_CLK(VCLK3_DAC);
REGISTER_CLK(VCLK3_MISC);
REGISTER_CLK(VCLK3_DVI);
REGISTER_CLK(VCLK2_VIU);
REGISTER_CLK(VCLK2_VENC_DVI);
REGISTER_CLK(VCLK2_VENC_ENC480P);
REGISTER_CLK(VCLK2_VENC_BIST);
REGISTER_CLK(VCLK1_VENC_656);
REGISTER_CLK(VCLK1_VENC_DVI);
REGISTER_CLK(VCLK1_VENC_ENCI);
REGISTER_CLK(VCLK1_VENC_BIST);
REGISTER_CLK(VIDEO_IN);
REGISTER_CLK(WIFI);


/*
 * Here we only define clocks that are meaningful to
 * look up through clockdevice.
 */
static struct clk_lookup lookups[] = {
    {
        .dev_id = "clk_xtal",
        .clk    = &xtal_clk,
    },
    {
        .dev_id = "clk_sys_pll",
        .clk    = &clk_sys_pll,
    },
    {
        .dev_id = "clk_misc_pll",
        .clk    = &clk_misc_pll,
    },
    {
        .dev_id = "clk_ddr_pll",
        .clk    = &clk_ddr_pll,
    },
    {
        .dev_id = "clk81",
        .clk    = &clk81,
    },
    {
        .dev_id = "cpu_clk",
        .clk    = &cpu_clk,
    },
    {
        .dev_id = "audio_clk",
        .clk    = &audio_clk,
    },
    {
        .dev_id = "video_clk",
        .clk    = &video_clk,
    },
    CLK_LOOKUP_ITEM(AHB_BRIDGE),
    CLK_LOOKUP_ITEM(AHB_SRAM),
    CLK_LOOKUP_ITEM(AIU_ADC),
    CLK_LOOKUP_ITEM(AIU_MIXER_REG),
    CLK_LOOKUP_ITEM(AIU_AUD_MIXER),
    CLK_LOOKUP_ITEM(AIU_AIFIFO2),
    CLK_LOOKUP_ITEM(AIU_AMCLK_MEASURE),
    CLK_LOOKUP_ITEM(AIU_I2S_OUT),
    CLK_LOOKUP_ITEM(AIU_IEC958),
    CLK_LOOKUP_ITEM(AIU_AI_TOP_GLUE),
    CLK_LOOKUP_ITEM(AIU_AUD_DAC),
    CLK_LOOKUP_ITEM(AIU_ICE958_AMCLK),
    CLK_LOOKUP_ITEM(AIU_I2S_DAC_AMCLK),
    CLK_LOOKUP_ITEM(AIU_I2S_SLOW),
    CLK_LOOKUP_ITEM(AIU_AUD_DAC_CLK),
    CLK_LOOKUP_ITEM(ASSIST_MISC),
    CLK_LOOKUP_ITEM(AMRISC),
    CLK_LOOKUP_ITEM(AUD_BUF),
    CLK_LOOKUP_ITEM(AUD_IN),
    CLK_LOOKUP_ITEM(BLK_MOV),
    CLK_LOOKUP_ITEM(BT656_IN),
    CLK_LOOKUP_ITEM(DEMUX),
    CLK_LOOKUP_ITEM(MMC_DDR),
    CLK_LOOKUP_ITEM(DDR),
    CLK_LOOKUP_ITEM(ETHERNET),
    CLK_LOOKUP_ITEM(GE2D),
    CLK_LOOKUP_ITEM(HDMI_MPEG_DOMAIN),
    CLK_LOOKUP_ITEM(HIU_PARSER_TOP),
    CLK_LOOKUP_ITEM(HIU_PARSER),
    CLK_LOOKUP_ITEM(ISA),
    CLK_LOOKUP_ITEM(MEDIA_CPU),
    CLK_LOOKUP_ITEM(MISC_USB0_TO_DDR),
    CLK_LOOKUP_ITEM(MISC_USB1_TO_DDR),
    CLK_LOOKUP_ITEM(MISC_SATA_TO_DDR),
    CLK_LOOKUP_ITEM(AHB_CONTROL_BUS),
    CLK_LOOKUP_ITEM(AHB_DATA_BUS),
    CLK_LOOKUP_ITEM(AXI_BUS),
    CLK_LOOKUP_ITEM(ROM_CLK),
    CLK_LOOKUP_ITEM(EFUSE),
    CLK_LOOKUP_ITEM(AHB_ARB0),
    CLK_LOOKUP_ITEM(RESET),
    CLK_LOOKUP_ITEM(MDEC_CLK_PIC_DC),
    CLK_LOOKUP_ITEM(MDEC_CLK_DBLK),
    CLK_LOOKUP_ITEM(MDEC_CLK_PSC),
    CLK_LOOKUP_ITEM(MDEC_CLK_ASSIST),
    CLK_LOOKUP_ITEM(MC_CLK),
    CLK_LOOKUP_ITEM(IQIDCT_CLK),
    CLK_LOOKUP_ITEM(VLD_CLK),
    CLK_LOOKUP_ITEM(NAND),
    CLK_LOOKUP_ITEM(RESERVED0),
    CLK_LOOKUP_ITEM(VGHL_PWM),
    CLK_LOOKUP_ITEM(LED_PWM),
    CLK_LOOKUP_ITEM(UART1),
    CLK_LOOKUP_ITEM(SDIO),
    CLK_LOOKUP_ITEM(ASYNC_FIFO),
    CLK_LOOKUP_ITEM(STREAM),
    CLK_LOOKUP_ITEM(RTC),
    CLK_LOOKUP_ITEM(UART0),
    CLK_LOOKUP_ITEM(RANDOM_NUM_GEN),
    CLK_LOOKUP_ITEM(SMART_CARD_MPEG_DOMAIN),
    CLK_LOOKUP_ITEM(SMART_CARD),
    CLK_LOOKUP_ITEM(SAR_ADC),
    CLK_LOOKUP_ITEM(I2C),
    CLK_LOOKUP_ITEM(IR_REMOTE),
    CLK_LOOKUP_ITEM(_1200XXX),
    CLK_LOOKUP_ITEM(SATA),
    CLK_LOOKUP_ITEM(SPI1),
    CLK_LOOKUP_ITEM(USB1),
    CLK_LOOKUP_ITEM(USB0),
    CLK_LOOKUP_ITEM(VI_CORE),
    CLK_LOOKUP_ITEM(LCD),
    CLK_LOOKUP_ITEM(ENC480P_MPEG_DOMAIN),
    CLK_LOOKUP_ITEM(ENC480I),
    CLK_LOOKUP_ITEM(VENC_MISC),
    CLK_LOOKUP_ITEM(ENC480P),
    CLK_LOOKUP_ITEM(HDMI),
    CLK_LOOKUP_ITEM(VCLK3_DAC),
    CLK_LOOKUP_ITEM(VCLK3_MISC),
    CLK_LOOKUP_ITEM(VCLK3_DVI),
    CLK_LOOKUP_ITEM(VCLK2_VIU),
    CLK_LOOKUP_ITEM(VCLK2_VENC_DVI),
    CLK_LOOKUP_ITEM(VCLK2_VENC_ENC480P),
    CLK_LOOKUP_ITEM(VCLK2_VENC_BIST),
    CLK_LOOKUP_ITEM(VCLK1_VENC_656),
    CLK_LOOKUP_ITEM(VCLK1_VENC_DVI),
    CLK_LOOKUP_ITEM(VCLK1_VENC_ENCI),
    CLK_LOOKUP_ITEM(VCLK1_VENC_BIST),
    CLK_LOOKUP_ITEM(VIDEO_IN),
    CLK_LOOKUP_ITEM(WIFI)
};

static int __init apollo_clock_init(void)
{
    if (init_clock && init_clock != cpu_clk.rate) {
        if (sys_clkpll_setting(0, init_clock << 1) == 0) {
            cpu_clk.rate = init_clock;
            clk_sys_pll.rate = init_clock << 1;
        }
    }

    /* Register the lookups */
    clkdev_add_table(lookups, ARRAY_SIZE(lookups));
    return 0;
}

/* initialize clocking early to be available later in the boot */
core_initcall(apollo_clock_init);

unsigned long long clkparse(const char *ptr, char **retptr)
{
    char *endptr;   /* local pointer to end of parsed string */

    unsigned long long ret = simple_strtoull(ptr, &endptr, 0);

    switch (*endptr) {
    case 'G':
    case 'g':
        ret *= 1000;
    case 'M':
    case 'm':
        ret *= 1000;
    case 'K':
    case 'k':
        ret *= 1000;
        endptr++;
    default:
        break;
    }

    if (retptr) {
        *retptr = endptr;
    }

    return ret;
}

static int __init cpu_clock_setup(char *ptr)
{
    init_clock = clkparse(ptr, 0);
    if (sys_clkpll_setting(0, init_clock << 1) == 0) {
        cpu_clk.rate = init_clock;
        clk_sys_pll.rate = init_clock << 1;
    }
    return 0;
}
__setup("cpu_clk=", cpu_clock_setup);

static int __init clk81_clock_setup(char *ptr)
{
    int clock = clkparse(ptr, 0);
    if (misc_pll_setting(0, clock * 4) == 0) {
        int baudrate = (clock / (115200 * 4)) - 1;

        clk_misc_pll.rate = clock * 4;
        clk81.rate = clock;

        WRITE_CBUS_REG(HHI_MPEG_CLK_CNTL,              // MPEG clk81 set to misc/3
                       (1 << 12) |                     // select other PLL
                       ((3 - 1) << 0) |                // div1
                       (1 << 7) |                      // cntl_hi_mpeg_div_en, enable gating
                       (1 << 8));                      // Connect clk81 to the PLL divider output

        CLEAR_CBUS_REG_MASK(UART0_CONTROL, (1 << 19) | 0xFFF);
        SET_CBUS_REG_MASK(UART0_CONTROL, (baudrate & 0xfff));
        CLEAR_CBUS_REG_MASK(UART1_CONTROL, (1 << 19) | 0xFFF);
        SET_CBUS_REG_MASK(UART1_CONTROL, (baudrate & 0xfff));
    }

    return 0;
}
__setup("clk81=", clk81_clock_setup);

int clk_enable(struct clk *clk)
{
    unsigned long   flags;

    spin_lock_irqsave(&clockfw_lock, flags);
    if (clk->clock_index < GCLK_IDX_MAX && clk->clock_gate_reg_adr != 0) {
        if (GCLK_ref[clk->clock_index]++ == 0) {
            SET_CBUS_REG_MASK(clk->clock_gate_reg_adr, clk->clock_gate_reg_mask);
        }
    }
    spin_unlock_irqrestore(&clockfw_lock, flags);
    return 0;
}
EXPORT_SYMBOL(clk_enable);

void clk_disable(struct clk *clk)
{
    unsigned long   flags;

    spin_lock_irqsave(&clockfw_lock, flags);

    if (clk->clock_index < GCLK_IDX_MAX && clk->clock_gate_reg_adr != 0) {
        do {
            if (GCLK_ref[clk->clock_index] == 0) {
                break;
            }
            if (--GCLK_ref[clk->clock_index] == 0) {
                CLEAR_CBUS_REG_MASK(clk->clock_gate_reg_adr, clk->clock_gate_reg_mask);
            }
        } while (0);
    }

    spin_unlock_irqrestore(&clockfw_lock, flags);
}
EXPORT_SYMBOL(clk_disable);

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
//                                    cts_media_cpu_clk,          // [9]
//                                    clk81,                      // [7]
//                                    vid_pll_clk,                // [6]
//                                    aud_pll_clk,                // [5]
//                                    misc_pll_clk,               // [4]
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
unsigned int clk_util_clk_msr(unsigned int clk_mux)
{
    unsigned int regval = 0;
    WRITE_CBUS_REG(MSR_CLK_REG0, 0);
    // Set the measurement gate to 64uS
    CLEAR_CBUS_REG_MASK(MSR_CLK_REG0, 0xffff);
    SET_CBUS_REG_MASK(MSR_CLK_REG0, 64 - 1);
    // Disable continuous measurement
    // disable interrupts
    CLEAR_CBUS_REG_MASK(MSR_CLK_REG0, ((1 << 18) | (1 << 17)));
    CLEAR_CBUS_REG_MASK(MSR_CLK_REG0, (0xf << 20));
    SET_CBUS_REG_MASK(MSR_CLK_REG0, (clk_mux << 20) | (1 << 19) | (1 << 16));

    // Wait for the measurement to be done
    regval = READ_CBUS_REG(MSR_CLK_REG0);
    do {
        regval = READ_CBUS_REG(MSR_CLK_REG0);
    } while (regval & (1 << 31));

    // disable measuring
    CLEAR_CBUS_REG_MASK(MSR_CLK_REG0, (1 << 16));
    regval = (READ_CBUS_REG(MSR_CLK_REG2) + 31) & 0x000FFFFF;
    // Return value in MHz*measured_val
    return (regval >> 6);
}

unsigned  int get_system_clk(void)
{
    static unsigned int sys_freq = 0;
    if (sys_freq == 0) {
        sys_freq = (clk_util_clk_msr(CLK_SYS) * 1000000);
    }
    return sys_freq;
}
EXPORT_SYMBOL(get_system_clk);

unsigned int get_mpeg_clk(void)
{
    static unsigned int clk81_freq = 0;
    if (clk81_freq == 0) {
        clk81_freq = (clk_util_clk_msr(CLK_CLK81) * 1000000);
    }
    return clk81_freq;
}
EXPORT_SYMBOL(get_mpeg_clk);

unsigned int get_misc_pll_clk(void)
{
    static unsigned int freq = 0;
    if (freq == 0) {
        freq = (clk_util_clk_msr(CLK_MISC_PLL) * 1000000);
    }
    return freq;
}
EXPORT_SYMBOL(get_misc_pll_clk);

unsigned int get_ethernet_pad_clk(void)
{
    static unsigned int freq = 0;
    if (freq == 0) {
        freq = (clk_util_clk_msr(CLK_MOD_ETH_CLK50_I) * 1000000);
    }
    return freq;
}
EXPORT_SYMBOL(get_ethernet_pad_clk);
