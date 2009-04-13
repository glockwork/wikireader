#ifndef WIKIREADER_H
#define WIKIREADER_H

#include "config.h"

#if BOARD_S1C33E07
	#include "boards/s1c33e07.h"
#elif BOARD_PROTO1
	#define LCD_INVERTED		1
	#include "boards/proto1.h"
#elif BOARD_PROTO2
	#define LCD_INVERTED		1
	#include "boards/proto2.h"
#elif BOARD_SAMO_A1
	#include "boards/samo_a1.h"
#elif BOARD_PRT33L17LCD
	#include "boards/prt33l17lcd.h"
#else
	#error "unsupported board type"
#endif

#define DEBUGLED1_ON()  do { REG_P1_P1D &= ~(1 << 4); } while (0)
#define DEBUGLED1_OFF() do { REG_P1_P1D |=  (1 << 4); } while (0)

#define DEBUGLED2_ON()  do { REG_P1_P1D &= ~(1 << 3); } while (0)
#define DEBUGLED2_OFF() do { REG_P1_P1D |=  (1 << 3); } while (0)

#define EEPROM_CS_LO()  do { REG_P5_P5D &= ~(1 << 2); } while (0)
#define EEPROM_CS_HI()  do { REG_P5_P5D |=  (1 << 2); } while (0)

#define LCD_CS_LO()     do { REG_P8_P8D &= ~(1 << 5); } while (0)
#define LCD_CS_HI()     do { REG_P8_P8D |=  (1 << 5); } while (0)

#define TFT_CTL1_LO()   do { REG_P8_P8D &= ~(1 << 3); } while (0)
#define TFT_CTL1_HI()   do { REG_P8_P8D |=  (1 << 3); } while (0)

static inline void init_rs232(void)
{
	/* serial line 0: 8-bit async, no parity, internal clock, 1 stop bit */
	REG_EFSIF0_CTL = 0xc3;

	/* DIVMD = 1/8, General I/F mode */
	REG_EFSIF0_IRDA = 0x10;

	/* by default MCLKDIV = 0 which means that the internal MCLK is OSC/1,
	 * where OSC = OSC3 as OSCSEL[1:0] = 00b
	 * Hence, MCLK is 48MHz */

	/* set up baud rate timer reload data */
	/*
	 * BRTRD = ((F[brclk] * DIVMD) / (2 * bps)) - 1;
	 * where
	 *      F[brclk] = 48MHz
	 *      DIVMD = 1/8
	 *      bps = 57600
	 *
	 *   = 51
	 */

	REG_EFSIF0_BRTRDL = 51 & 0xff;
	REG_EFSIF0_BRTRDM = 51 >> 8;

	/* baud rate timer: run! */
	REG_EFSIF0_BRTRUN = 0x01;
}

#if BOARD_PROTO2
static inline int get_battery_voltage(void)
{
	int val;

	/* switch on A/D converter clock and select MCLK/256 */
	REG_AD_CLKCTL = 0xf;

	/* A/D Trigger/Channel Select Register: channel 0,
	 * one-shot, software triggered */
	REG_AD_TRIG_CHNL = 0;

	/* select P70 pin function for AIN0 */
	REG_P7_03_CFP = 0x01;

	/* A/D Control/Status Register: start conversion (9 clock cycles) */
	REG_AD_EN_SMPL_STAT = 0x304;

	/* A/D Control/Status Register: trigger ADST */
	REG_AD_EN_SMPL_STAT |= (1 << 1);

	/* wait for the conversion to complete */
	do { asm("nop"); } while (!(REG_AD_EN_SMPL_STAT & (1 << 3)));

	/* read the value */
	val = REG_AD_ADD;

	/* select P70 pin function for P70 */
	REG_P7_03_CFP = 0;

	/* A/D Control/Status Register: disable controller */
	REG_AD_EN_SMPL_STAT = 0;
	
	/* switch off A/D converter clock */
	REG_AD_CLKCTL = 0;

	return val;
}
#else
static inline int get_battery_voltage(void)
{
	/* return some sane value for platforms with no hardware support
	 * for voltage measurement so the logic using this function will
	 * not bail. */
	return 1000;
}
#endif

#endif /* WIKIREADER_H */

