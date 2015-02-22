/*
 * MX23L8111 512K x 16 Mask ROM dumper.
 *
 * Target: ATxmega16a4
 *
 * Copyright: Rolfe Bozier, rolfe@pobox.com, 2013
 */
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <avr/sleep.h>
#include <util/delay.h>
#include MCU_H

#include "avr-common.h"
#include "common.h"

/***************************************************************************
 * Initialisation
 ***************************************************************************/

static void
uc_init(void)
{
    /*
     * I/O port usage:
     *
     * PORTA    = D7-D0     input from ROM
     * PORTB:
     *  PB3     = A18       output to ROM
     *  PB2     = A17       output to ROM
     *  PB1     = A16       output to ROM
     *  PB0     = A-1       output to ROM
     * PORTC    = A15-A8    output to ROM
     * PORTD    = A7-A0     output to ROM
     * PORTE:
     *  PE3     = TXD0      output to serial connection
     *  PE2     = (RXD0)    input from serial connection
     *  PE1     = C̅E̅        output to ROM
     *  PE0     = O̅E̅        output to ROM
     *
     * other:
     *  PDI     = PDI_DATA  I/O to PDI programmer
     *  R̅E̅S̅E̅T̅   = PDI_CLOCK I/O to PDI programmer
     */

    /*
     * Wait for external crystal oscillator to become stable
     */
    cbi(OSC_XOSCCTRL, OSC_XOSCSEL3_bp); /* XTAL source, 1K CLK startup */
    sbi(OSC_XOSCCTRL, OSC_XOSCSEL2_bp);
    sbi(OSC_XOSCCTRL, OSC_XOSCSEL1_bp);
    sbi(OSC_XOSCCTRL, OSC_XOSCSEL0_bp);

    sbi(OSC_CTRL, OSC_XOSCEN_bp);
    while ((OSC_STATUS & OSC_XOSCRDY_bm) == 0)
        continue;

    /*
     * Switch clock sources
     */
    CCP = CCP_IOREG_gc;             /* enable access to protected I/O registers */
    CLK_CTRL = CLK_SCLKSEL_XOSC_gc; /* source = external oscillator */

    /*
     * Set up ports for GPIO
     */
    PORTA_DIR = 0x00;           /* D7-D0 */

    PORTB_DIR = 0xFF;           /* A18-A16, A-1 */
    PORTB_OUT = 0x00;

    PORTC_DIR = 0xFF;           /* A15-A8 */
    PORTC_OUT = 0x00;

    PORTD_DIR = 0xFF;           /* A7-A0 */
    PORTD_OUT = 0x00;

    sbi(PORTE_DIR, PIN3_bp);    /* TXD0 */
    cbi(PORTE_OUT, PIN3_bp);

#if 0
    cbi(PORTE_DIR, PIN2_bp);    /* RXD0 */
#endif
    sbi(PORTE_DIR, PIN2_bp);    /* RXD0 */

    sbi(PORTE_DIR, PIN1_bp);    /* C̅E̅ */
    sbi(PORTE_OUT, PIN1_bp);

    sbi(PORTE_DIR, PIN0_bp);    /* O̅E̅ */
    sbi(PORTE_OUT, PIN0_bp);

    /*
     * Set up USARTE0 for serial comms
     */
    serial_init();

    /*
     * Global settings
     */
    sbi(PMIC_CTRL, PMIC_MEDLVLEX_bp);   /* enable medium-level interrupts */
}

static inline void
set_nCE(uint8_t b) { if (b) sbi(PORTE_OUT, PIN1_bp); else cbi(PORTE_OUT, PIN1_bp); }

static inline void
set_nOE(uint8_t b) { if (b) sbi(PORTE_OUT, PIN0_bp); else cbi(PORTE_OUT, PIN0_bp); }

static inline void
set_A_1(uint8_t b) { if (b) sbi(PORTB_OUT, PIN0_bp); else cbi(PORTB_OUT, PIN0_bp); }

static inline void
set_A7_A0(uint8_t v) { PORTD_OUT = v; }

static inline void
set_A15_A8(uint8_t v) { PORTC_OUT = v; }

static inline void
set_A18_A16(uint8_t v) { PORTB_OUT = (PORTB_OUT & 0x1) | ((v & 0x7) << 1); }

static inline uint8_t
get_D7_D0(void) { return PORTA_IN; }

/***************************************************************************
 * Main program
 ***************************************************************************/

int
main(void)
{
    const uint32_t  address_size    = (uint32_t)1 << 20;
    const uint32_t  num_rows        = address_size >> 4;    /* 16 words */
    uint32_t        row;
    uint32_t        addr;
    uint8_t         buffer[32];
    uint8_t         word;
    uint8_t         i;

    uc_init();
    serial_write("\r\nReady\r\n\r\n");

    addr = 0;
    for (row = 0; row < num_rows; row++)
    {
        set_nCE(0);
        set_nOE(0);

        for (word = 0; word < 16; word++)
        {
            uint8_t a7_a0   = (uint8_t)(addr & 0x000000ff);
            uint8_t a15_a8  = (uint8_t)((addr & 0x0000ff00) >> 8);
            uint8_t a18_a16 = (uint8_t)((addr & 0x00070000) >> 16);

            set_A7_A0(a7_a0);
            set_A15_A8(a15_a8);
            set_A18_A16(a18_a16);

            /*
             * XXX: The minimum time between presenting the address and reading
             * the data is 100ns, but we are running with a 1.8MHz clock,
             * so there is no need to delay.
             */
            set_A_1(1);         /* high byte */
            _delay_us(1);
            buffer[word*2 + 0] = get_D7_D0();

            set_A_1(0);         /* low byte */
            _delay_us(1);
            buffer[word*2 + 1] = get_D7_D0();

            addr++;
        }

        set_nCE(1);
        set_nOE(1);

        for (i = 0; i < 32; i++)
        {
            char    buf[3];
            uint8_t n;
            char    lut[]   = { '0', '1', '2', '3', '4', '5', '6', '7',
                                '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

            /*
            serial_write("%02x", buffer[i]);
            */

            n = (buffer[i] & 0xf0) >> 4;
            buf[0] = lut[n];
            n = buffer[i] & 0x0f;
            buf[1] = lut[n];
            buf[2] = '\0';

            serial_write_string(buf);
        }
        serial_write("\r\n");
    }

    for (;;)
        continue;

    return 0;
}
