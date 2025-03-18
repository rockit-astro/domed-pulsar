//**********************************************************************************
//  Copyright 2016, 2017, 2023, 2025 Paul Chote
//  This file is part of the Robotic Observatory Control Kit (rockit)
// 
//  rockit is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  rockit is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with rockit.  If not, see <http://www.gnu.org/licenses/>.
//**********************************************************************************

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdint.h>

#define TX_LED_DISABLED   PORTD &= ~_BV(PD5)
#define TX_LED_ENABLED    PORTD |= _BV(PD5)
#define RX_LED_DISABLED   PORTB &= ~_BV(PB0)
#define RX_LED_ENABLED    PORTB |= _BV(PB0)
#define TX_RX_LED_INIT    DDRD |= _BV(DDD5), DDRB |= _BV(DDB0), TX_LED_DISABLED, RX_LED_DISABLED

// Counters (in 9.984ms increments) for blinking the TX/RX LEDs
#define TX_RX_LED_PULSE_MS 10
volatile uint8_t tx_led_pulse = 0;
volatile uint8_t rx_led_pulse = 0;

static uint8_t output_buffer[256];
static volatile uint8_t output_read = 0;
static volatile uint8_t output_write = 0;

static uint8_t input_buffer[256];
static uint8_t input_read = 0;
static volatile uint8_t input_write = 0;

void serial_initialize(void)
{
#define BAUD 4800
#include <util/setbaud.h>
    UBRR1H = UBRRH_VALUE;
    UBRR1L = UBRRL_VALUE;
#if USE_2X
    UCSR1A = _BV(U2X0);
#endif

    // Enable receive, transmit, data received interrupt
    UCSR1B = _BV(RXEN1) | _BV(TXEN1) | _BV(RXCIE1);
    TX_RX_LED_INIT;

    // Configure timer3 to interrupt every 0.009984 seconds
    // for ticking the TX/RX LEDs
    // Note: this should use a timer with lower interrupt
    // priority than the UDRE to avoid race conditions
    OCR3A = 156;
    TCCR3B = _BV(CS32) | _BV(CS30) | _BV(WGM32);
    TIMSK3 |= _BV(OCIE3A);

    tx_led_pulse = rx_led_pulse = 0;
    input_read = input_write = 0;
    output_read = output_write = 0;
}

bool serial_can_read(void)
{
    return input_write != input_read;
}

// Read a byte from the receive buffer
// Will block if the buffer is empty
uint8_t serial_read(void)
{
    while (input_read == input_write);
    return input_buffer[input_read++];
}

// Add a byte to the send buffer.
// Will block if the buffer is full
void serial_write(uint8_t b)
{
    // Don't overwrite data that hasn't been sent yet
    while (output_write == (uint8_t)(output_read - 1));

    output_buffer[output_write++] = b;
    // Enable transmit if necessary
    UCSR1B |= _BV(UDRIE1);
}

ISR(USART1_UDRE_vect)
{
    if (output_write != output_read)
    {
        UDR1 = output_buffer[output_read++];
        TX_LED_ENABLED;
        tx_led_pulse = TX_RX_LED_PULSE_MS;
    }

    // Ran out of data to send - disable the interrupt
    if (output_write == output_read)
        UCSR1B &= ~_BV(UDRIE1);
}

ISR(USART1_RX_vect)
{
    input_buffer[(uint8_t)(input_write++)] = UDR1;
    RX_LED_ENABLED;
    rx_led_pulse = TX_RX_LED_PULSE_MS;
}

ISR(TIMER3_COMPA_vect)
{
    // Runs once every 10ms
    if (tx_led_pulse && !(--tx_led_pulse))
        TX_LED_DISABLED;
    if (rx_led_pulse && !(--rx_led_pulse))
        RX_LED_DISABLED;

    // Work around a bug where the LEDs stay enabled when *_led_pulse == 0
    // TODO: work out what causes this and fix it properly
    if (tx_led_pulse == 0)
        TX_LED_DISABLED;
    if (rx_led_pulse == 0)
        RX_LED_DISABLED;
}