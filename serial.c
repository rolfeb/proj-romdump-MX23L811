/*
 * ROM dumper
 *
 * Copyright: Rolfe Bozier, rolfe@pobox.com, 2013
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <util/delay.h>
#include MCU_H

#include "avr-common.h"

#include "common.h"

#define BUFLEN      128

volatile static char        buffer[BUFLEN];
volatile static uint8_t     buffer_busy;
volatile static uint8_t     pending_chars;
volatile static char        *write_ptr;

/*
 * ISR called when we have successfully transmitted a character. If there
 * is another one to send, the transmit it.
 */
ISR(USARTE0_TXC_vect)
{
    if (pending_chars > 0)
    {
        USARTE0_DATA = *write_ptr++;
        pending_chars--;
    }
    else
        buffer_busy = 0;
}

/*
 * Set up USARTE0 for serial comms
 */
void
serial_init(void)
{
    /* set TX interrupt level to Medium */
    sbi(USARTE0_CTRLA, USART_TXCINTLVL1_bp);
    cbi(USARTE0_CTRLA, USART_TXCINTLVL0_bp);

    /* set the baud rate to 115200 */
    /* BSEL = f / (16 * 115200) -1 = 0 */
    /* BSCALE = 0 */
    USARTE0_BAUDCTRLA = 0;
    USARTE0_BAUDCTRLB = 0;

    /* enable the transmitter */
    sbi(USARTE0_CTRLB, USART_TXEN_bp);

    buffer_busy = 0;
    pending_chars = 0;
    write_ptr = NULL;
}

#define UART_TX_BUFFER_EMPTY()   ((USARTE0_STATUS & USART_DREIF_bm) != 0)

void
serial_write(char *fmt, ...)
{
    va_list ap;

    /*
     * Wait until the transmit buffer is empty
     */
    for (;;)
    {
        cli();
        if (!buffer_busy)
        {
            sei();
            break;
        }
        sei();

        _delay_ms(1);
    }

    /*
     * Format the message in the buffer
     */
    va_start(ap, fmt);
    pending_chars = vsnprintf((char *)buffer, BUFLEN, fmt, ap);
    va_end(ap);

    write_ptr = buffer;
    buffer_busy = 1;

    /*
     * Transmit the first character
     */
    while (!UART_TX_BUFFER_EMPTY())
        continue;

    USARTE0_DATA = *write_ptr++;
    pending_chars--;
}

void
serial_write_string(char *string)
{
    /*
     * Wait until the transmit buffer is empty
     */
    for (;;)
    {
        cli();
        if (!buffer_busy)
        {
            sei();
            break;
        }
        sei();

        _delay_ms(1);
    }

    strcpy((char *)buffer, string);
    pending_chars = strlen(string);

    write_ptr = buffer;
    buffer_busy = 1;

    /*
     * Transmit the first character
     */
    while (!UART_TX_BUFFER_EMPTY())
        continue;

    USARTE0_DATA = *write_ptr++;
    pending_chars--;
}
