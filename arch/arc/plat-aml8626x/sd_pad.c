#include <plat/card_io.h>
#include <plat/cardreader/card_block.h>
#include <plat/cardreader/cardreader.h>


static unsigned sd_backup_input_val = 0;
static unsigned sd_backup_output_val = 0;
static unsigned SD_BAKUP_INPUT_REG = (unsigned)&sd_backup_input_val;
static unsigned SD_BAKUP_OUTPUT_REG = (unsigned)&sd_backup_output_val;

unsigned SD_CMD_OUTPUT_EN_REG;
unsigned SD_CMD_OUTPUT_EN_MASK;
unsigned SD_CMD_INPUT_REG;
unsigned SD_CMD_INPUT_MASK;
unsigned SD_CMD_OUTPUT_REG;
unsigned SD_CMD_OUTPUT_MASK;

unsigned SD_CLK_OUTPUT_EN_REG;
unsigned SD_CLK_OUTPUT_EN_MASK;
unsigned SD_CLK_OUTPUT_REG;
unsigned SD_CLK_OUTPUT_MASK;

unsigned SD_DAT_OUTPUT_EN_REG;
unsigned SD_DAT0_OUTPUT_EN_MASK;
unsigned SD_DAT0_3_OUTPUT_EN_MASK;
unsigned SD_DAT_INPUT_REG;
unsigned SD_DAT_OUTPUT_REG;
unsigned SD_DAT0_INPUT_MASK;
unsigned SD_DAT0_OUTPUT_MASK;
unsigned SD_DAT0_3_INPUT_MASK;
unsigned SD_DAT0_3_OUTPUT_MASK;
unsigned SD_DAT_INPUT_OFFSET;
unsigned SD_DAT_OUTPUT_OFFSET;

unsigned SD_INS_OUTPUT_EN_REG;
unsigned SD_INS_OUTPUT_EN_MASK;
unsigned SD_INS_INPUT_REG;
unsigned SD_INS_INPUT_MASK;

unsigned SD_WP_OUTPUT_EN_REG;
unsigned SD_WP_OUTPUT_EN_MASK;
unsigned SD_WP_INPUT_REG;
unsigned SD_WP_INPUT_MASK;

unsigned SD_PWR_OUTPUT_EN_REG;
unsigned SD_PWR_OUTPUT_EN_MASK;
unsigned SD_PWR_OUTPUT_REG;
unsigned SD_PWR_OUTPUT_MASK;
unsigned SD_PWR_EN_LEVEL;

unsigned SD_WORK_MODE;


void sd_io_init(struct memory_card *card)
{
	struct aml_card_info *aml_card_info = card->card_plat_info;
	SD_WORK_MODE = aml_card_info->work_mode;

	switch (aml_card_info->io_pad_type) {

		case SDIO_GPIOA_6_11:
			SD_CMD_OUTPUT_EN_REG = EGPIO_GPIOA_ENABLE;
			SD_CMD_OUTPUT_EN_MASK = PREG_IO_7_MASK;
			SD_CMD_INPUT_REG = EGPIO_GPIOA_INPUT;
			SD_CMD_INPUT_MASK = PREG_IO_7_MASK;
			SD_CMD_OUTPUT_REG = EGPIO_GPIOA_OUTPUT;
			SD_CMD_OUTPUT_MASK = PREG_IO_7_MASK;

			SD_CLK_OUTPUT_EN_REG = EGPIO_GPIOA_ENABLE;
			SD_CLK_OUTPUT_EN_MASK = PREG_IO_6_MASK;
			SD_CLK_OUTPUT_REG = EGPIO_GPIOA_OUTPUT;
			SD_CLK_OUTPUT_MASK = PREG_IO_6_MASK;

			SD_DAT_OUTPUT_EN_REG = EGPIO_GPIOA_ENABLE;
			SD_DAT0_OUTPUT_EN_MASK = PREG_IO_8_MASK;
			SD_DAT0_3_OUTPUT_EN_MASK = PREG_IO_8_11_MASK;
			SD_DAT_INPUT_REG = EGPIO_GPIOA_INPUT;
			SD_DAT_OUTPUT_REG = EGPIO_GPIOA_OUTPUT;
			SD_DAT0_INPUT_MASK = PREG_IO_8_MASK;
			SD_DAT0_OUTPUT_MASK = PREG_IO_8_MASK;
			SD_DAT0_3_INPUT_MASK = PREG_IO_8_11_MASK;
			SD_DAT0_3_OUTPUT_MASK = PREG_IO_8_11_MASK;
			SD_DAT_INPUT_OFFSET = 8;
			SD_DAT_OUTPUT_OFFSET = 8;
			break;

		case SDIO_GPIOB_0_5:
			SD_CMD_OUTPUT_EN_REG = EGPIO_GPIOB_ENABLE;
			SD_CMD_OUTPUT_EN_MASK = PREG_IO_0_MASK;
			SD_CMD_INPUT_REG = EGPIO_GPIOB_INPUT;
			SD_CMD_INPUT_MASK = PREG_IO_0_MASK;
			SD_CMD_OUTPUT_REG = EGPIO_GPIOB_OUTPUT;
			SD_CMD_OUTPUT_MASK = PREG_IO_0_MASK;

			SD_CLK_OUTPUT_EN_REG = EGPIO_GPIOB_ENABLE;
			SD_CLK_OUTPUT_EN_MASK = PREG_IO_1_MASK;
			SD_CLK_OUTPUT_REG = EGPIO_GPIOB_OUTPUT;
			SD_CLK_OUTPUT_MASK = PREG_IO_1_MASK;

			SD_DAT_OUTPUT_EN_REG = EGPIO_GPIOB_ENABLE;
			SD_DAT0_OUTPUT_EN_MASK = PREG_IO_2_MASK;
			SD_DAT0_3_OUTPUT_EN_MASK = PREG_IO_2_5_MASK;
			SD_DAT_INPUT_REG = EGPIO_GPIOB_INPUT;
			SD_DAT_OUTPUT_REG = EGPIO_GPIOB_OUTPUT;
			SD_DAT0_INPUT_MASK = PREG_IO_2_MASK;
			SD_DAT0_OUTPUT_MASK = PREG_IO_2_MASK;
			SD_DAT0_3_INPUT_MASK = PREG_IO_2_5_MASK;
			SD_DAT0_3_OUTPUT_MASK = PREG_IO_2_5_MASK;
			SD_DAT_INPUT_OFFSET = 2;
			SD_DAT_OUTPUT_OFFSET = 2;
			break;

		case SDIO_GPIOE_8_13:
			SD_CMD_OUTPUT_EN_REG = EGPIO_GPIOE_ENABLE;
			SD_CMD_OUTPUT_EN_MASK = PREG_IO_13_MASK;
			SD_CMD_INPUT_REG = EGPIO_GPIOE_INPUT;
			SD_CMD_INPUT_MASK = PREG_IO_13_MASK;
			SD_CMD_OUTPUT_REG = EGPIO_GPIOE_OUTPUT;
			SD_CMD_OUTPUT_MASK = PREG_IO_13_MASK;

			SD_CLK_OUTPUT_EN_REG = EGPIO_GPIOE_ENABLE;
			SD_CLK_OUTPUT_EN_MASK = PREG_IO_12_MASK;
			SD_CLK_OUTPUT_REG = EGPIO_GPIOE_OUTPUT;
			SD_CLK_OUTPUT_MASK = PREG_IO_12_MASK;

			SD_DAT_OUTPUT_EN_REG = EGPIO_GPIOE_ENABLE;
			SD_DAT0_OUTPUT_EN_MASK = PREG_IO_8_MASK;
			SD_DAT0_3_OUTPUT_EN_MASK = PREG_IO_8_11_MASK;
			SD_DAT_INPUT_REG = EGPIO_GPIOE_INPUT;
			SD_DAT_OUTPUT_REG = EGPIO_GPIOE_OUTPUT;
			SD_DAT0_INPUT_MASK = PREG_IO_8_MASK;
			SD_DAT0_OUTPUT_MASK = PREG_IO_8_MASK;
			SD_DAT0_3_INPUT_MASK = PREG_IO_8_11_MASK;
			SD_DAT0_3_OUTPUT_MASK = PREG_IO_8_11_MASK;
			SD_DAT_INPUT_OFFSET = 8;
			SD_DAT_OUTPUT_OFFSET = 8;
			break;

		case SDIO_GPIOF_4_9:
			SD_CMD_OUTPUT_EN_REG = EGPIO_GPIOF_ENABLE;
			SD_CMD_OUTPUT_EN_MASK = PREG_IO_22_MASK;
			SD_CMD_INPUT_REG = EGPIO_GPIOF_INPUT;
			SD_CMD_INPUT_MASK = PREG_IO_22_MASK;
			SD_CMD_OUTPUT_REG = EGPIO_GPIOF_OUTPUT;
			SD_CMD_OUTPUT_MASK = PREG_IO_22_MASK;

			SD_CLK_OUTPUT_EN_REG = EGPIO_GPIOF_ENABLE;
			SD_CLK_OUTPUT_EN_MASK = PREG_IO_23_MASK;
			SD_CLK_OUTPUT_REG = EGPIO_GPIOF_OUTPUT;
			SD_CLK_OUTPUT_MASK = PREG_IO_23_MASK;

			SD_DAT_OUTPUT_EN_REG = EGPIO_GPIOF_ENABLE;
			SD_DAT0_OUTPUT_EN_MASK = PREG_IO_24_MASK;
			SD_DAT0_3_OUTPUT_EN_MASK = PREG_IO_24_27_MASK;
			SD_DAT_INPUT_REG = EGPIO_GPIOF_INPUT;
			SD_DAT_OUTPUT_REG = EGPIO_GPIOF_OUTPUT;
			SD_DAT0_INPUT_MASK = PREG_IO_24_MASK;
			SD_DAT0_OUTPUT_MASK = PREG_IO_24_MASK;
			SD_DAT0_3_INPUT_MASK = PREG_IO_24_27_MASK;
			SD_DAT0_3_OUTPUT_MASK = PREG_IO_24_27_MASK;
			SD_DAT_INPUT_OFFSET = 24;
			SD_DAT_OUTPUT_OFFSET = 24;
			break;

        default:
			printk("Warning couldn`t find any valid hw io pad!!!\n");
            break;
	}

	if (aml_card_info->card_ins_en_reg) {
		SD_INS_OUTPUT_EN_REG = aml_card_info->card_ins_en_reg;
		SD_INS_OUTPUT_EN_MASK = aml_card_info->card_ins_en_mask;
		SD_INS_INPUT_REG = aml_card_info->card_ins_input_reg;
		SD_INS_INPUT_MASK = aml_card_info->card_ins_input_mask;
	}
	else {
		SD_INS_OUTPUT_EN_REG = SD_BAKUP_OUTPUT_REG;
		SD_INS_OUTPUT_EN_MASK = 1;
		SD_INS_INPUT_REG = SD_BAKUP_INPUT_REG;
		SD_INS_INPUT_MASK =
		SD_WP_INPUT_MASK = 1;
	}

	if (aml_card_info->card_power_en_reg) {
		SD_PWR_OUTPUT_EN_REG = aml_card_info->card_power_en_reg;
		SD_PWR_OUTPUT_EN_MASK = aml_card_info->card_power_en_mask;
		SD_PWR_OUTPUT_REG = aml_card_info->card_power_output_reg;
		SD_PWR_OUTPUT_MASK = aml_card_info->card_power_output_mask;
		SD_PWR_EN_LEVEL = aml_card_info->card_power_en_lev;
	}
	else {
		SD_PWR_OUTPUT_EN_REG = SD_BAKUP_OUTPUT_REG;
		SD_PWR_OUTPUT_EN_MASK = 1;
		SD_PWR_OUTPUT_REG = SD_BAKUP_OUTPUT_REG;
		SD_PWR_OUTPUT_MASK = 1;
		SD_PWR_EN_LEVEL = 0;	
	}

	if (aml_card_info->card_wp_en_reg) {
		SD_WP_OUTPUT_EN_REG = aml_card_info->card_wp_en_reg;
		SD_WP_OUTPUT_EN_MASK = aml_card_info->card_wp_en_mask;
		SD_WP_INPUT_REG = aml_card_info->card_wp_input_reg;
		SD_WP_INPUT_MASK = aml_card_info->card_wp_input_mask;
	}
	else {
		SD_WP_OUTPUT_EN_REG = SD_BAKUP_OUTPUT_REG;
		SD_WP_OUTPUT_EN_MASK = 1;
		SD_WP_INPUT_REG = SD_BAKUP_INPUT_REG;
		SD_WP_INPUT_MASK = 1;
	}

	return;
}


void sd_sdio_enable(SDIO_Pad_Type_t io_pad_type)
{
	switch (io_pad_type) {

		case SDIO_GPIOA_6_11:
			SET_CBUS_REG_MASK(CARD_PIN_MUX_0, (0x3F<<20));
			SET_CBUS_REG_MASK(SDIO_MULT_CONFIG, (1));
			break;

		case SDIO_GPIOB_0_5:
			SET_CBUS_REG_MASK(CARD_PIN_MUX_2, (0x3f<<0));
			SET_CBUS_REG_MASK(SDIO_MULT_CONFIG, (0));
			break;

		case SDIO_GPIOE_8_13:
			SET_CBUS_REG_MASK(CARD_PIN_MUX_4, (0x3F<<22));
			SET_CBUS_REG_MASK(SDIO_MULT_CONFIG, (2));
			break;

		case SDIO_GPIOF_4_9:
			SET_CBUS_REG_MASK(CARD_PIN_MUX_3, (0x3f<<26));
			SET_CBUS_REG_MASK(SDIO_MULT_CONFIG, (2));
			break;

		default :
			printk("invalid hw io pad!!!\n");
			break;
	}
	
	return;
}

void sd_gpio_enable(SDIO_Pad_Type_t io_pad_type)
{
	switch (io_pad_type) {

		case SDIO_GPIOA_6_11:
			CLEAR_CBUS_REG_MASK(CARD_PIN_MUX_0, (0x3F<<20));
			CLEAR_CBUS_REG_MASK(SDIO_MULT_CONFIG, (1));
			break;

		case SDIO_GPIOB_0_5:
			CLEAR_CBUS_REG_MASK(CARD_PIN_MUX_2, (0x3f<<0));
			CLEAR_CBUS_REG_MASK(SDIO_MULT_CONFIG, (0));
			break;

		case SDIO_GPIOE_8_13:
			CLEAR_CBUS_REG_MASK(CARD_PIN_MUX_4, (0x3F<<22));
			CLEAR_CBUS_REG_MASK(SDIO_MULT_CONFIG, (2));
			break;

		case SDIO_GPIOF_4_9:
			CLEAR_CBUS_REG_MASK(CARD_PIN_MUX_3, (0x3f<<26));
			CLEAR_CBUS_REG_MASK(SDIO_MULT_CONFIG, (2));
			break;

		default :
			printk("invalid hw io pad!!!\n");
			break;
	}
	
	return;
}

