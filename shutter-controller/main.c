//**********************************************************************************
//  Copyright 2024, 2025 Paul Chote
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

#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "gpio.h"
#include "serial.h"

// Amount of time to power the motors in units of 0.1s
#define MOVE_STEPS 650

#define DIR_STOPPED 0
#define DIR_OPEN 1
#define DIR_CLOSE 2

// Open/close flags should match open/close direction bits
#define FLAG_LIMIT_OPEN 1
#define FLAG_LIMIT_CLOSED 2
#define FLAG_MOVING 4
#define FLAG_BUTTON_OPEN 8
#define FLAG_BUTTON_CLOSE 16

char output_buffer[16];

volatile uint8_t requested_direction = DIR_STOPPED;
volatile uint8_t current_direction = DIR_STOPPED;
volatile uint8_t current_flags = 0;
volatile uint16_t move_counter = 0;
volatile uint8_t status_counter = 0;
volatile bool send_status = false;

// Number of seconds remaining until triggering the force-close
volatile uint8_t heartbeat_seconds_remaining = 0;

// Sticky status for whether the heartbeat has timed out
// and is either closing or has closed the roof.
volatile bool heartbeat_triggered = false;

gpin_t drive_en_l = { &PORTF, &PINF, &DDRF, PF4 };
gpin_t drive_en_r = { &PORTF, &PINF, &DDRF, PF5 };
gpin_t drive_pwm_l = { &PORTF, &PINF, &DDRF, PF6 };
gpin_t drive_pwm_r = { &PORTF, &PINF, &DDRF, PF7 };

gpin_t button_open = { &PORTB, &PINB, &DDRB, PB1 };
gpin_t button_close = { &PORTB, &PINB, &DDRB, PB3 };

gpin_t limit_open = { &PORTD, &PIND, &DDRD, PD1 };
gpin_t limit_closed = { &PORTD, &PIND, &DDRD, PD0 };

void poll_serial(void)
{
    // Check for commands from the host PC
    while (serial_can_read())
    {
        int16_t value = serial_read();
        if (value < 0)
            break;
        
        // Values between 0-240 are treated as heartbeat pings
        // Values greater than 240 (0xF0) are reserved for commands
        switch (value)
        {
            // Open roof
            case 0xF1:
                if (!heartbeat_triggered)
                {
                    cli();
                    requested_direction = DIR_OPEN;
                    move_counter = MOVE_STEPS;
                    sei();
                }
                break;

            // Close shutter
            case 0xF2:
                if (!heartbeat_triggered)
                {
                    cli();
                    requested_direction = DIR_CLOSE;
                    move_counter = MOVE_STEPS;
                    sei();
                }
            break;

            // Stop shutter movement
            case 0xFF:
                if (!heartbeat_triggered)
                    requested_direction = DIR_STOPPED;
                break;

            // Clear the sticky trigger flag when disabling the heartbeat
            // Also stops an active close
            case 0:
                cli();
                heartbeat_triggered = false;
                heartbeat_seconds_remaining = 0;
                requested_direction = DIR_STOPPED;
                sei();
            break;

            // Reset the heartbeat timer
            default:
                // If the heatbeat has triggered the status must be manually
                // cleared by sending a 0 byte
                cli();
                if (!heartbeat_triggered && value <= 240)
                    heartbeat_seconds_remaining = value;
                sei();
        }
    }

    if (send_status)
    {
        // Send current status back to the host computer
        // Disable interrupts while updating to ensure data consistency
        cli();
        uint8_t heartbeat = heartbeat_triggered ? 0xFF : heartbeat_seconds_remaining;
        uint8_t state = DIR_STOPPED;
        if (current_flags & FLAG_MOVING)
            state = current_direction;
        else if (current_flags & FLAG_LIMIT_OPEN)
            state = DIR_OPEN + 2;
        else if (current_flags & FLAG_LIMIT_CLOSED)
            state = DIR_CLOSE + 2;
        
        snprintf(output_buffer, 16, "%02d,%03d\r\n", state, heartbeat);
        sei();

        for (uint8_t i = 0; i < 8; i++)
            serial_write(output_buffer[i]);

        send_status = false;
    }
}

int main(void)
{
    // Make sure the relays are disabled before doing anything else
    gpio_output_set_low(&drive_en_l);
    gpio_configure_output(&drive_en_l);
    gpio_output_set_low(&drive_en_r);
    gpio_configure_output(&drive_en_r);
    gpio_output_set_low(&drive_pwm_l);
    gpio_configure_output(&drive_pwm_l);
    gpio_output_set_low(&drive_pwm_r);
    gpio_configure_output(&drive_pwm_r);

    gpio_configure_input_pullup(&button_open);
    gpio_configure_input_pullup(&button_close);

    gpio_configure_input_pullup(&limit_open);
    gpio_configure_input_pullup(&limit_closed);

    // Configure timer1 to interrupt at 10Hz
    OCR1A = 1563;
    TCCR1B = _BV(CS12) | _BV(CS10) | _BV(WGM12);
    TIMSK1 |= _BV(OCIE1A);

    serial_initialize();

    sei();
    for (;;)
        poll_serial();
}

ISR(TIMER1_COMPA_vect)
{
    if (++status_counter == 10)
    {
        send_status = true;
        status_counter = 0;

        // Decrement the heartbeat counter and trigger a close if it reaches 0
        if (!heartbeat_triggered && heartbeat_seconds_remaining != 0)
        {
            if (--heartbeat_seconds_remaining == 0)
            {
                heartbeat_triggered = true;
                requested_direction = DIR_CLOSE;
                move_counter = MOVE_STEPS;
            }
        }
    }
    
    // Read limits via optocouplers
    if (!gpio_input_read(&limit_open))
        current_flags |= FLAG_LIMIT_OPEN;
    else
        current_flags &= ~FLAG_LIMIT_OPEN;        
    
    if (!gpio_input_read(&limit_closed))
        current_flags |= FLAG_LIMIT_CLOSED;
    else
        current_flags &= ~FLAG_LIMIT_CLOSED;  

    // Debounce by requiring button to be pressed for two consecutive loops (0.2s)
    // Buttons are normally open, so read low when pressed
    if (!gpio_input_read(&button_open))
    {
        if ((current_flags & FLAG_BUTTON_OPEN) && !(current_flags & FLAG_LIMIT_OPEN))
        {
            requested_direction = DIR_OPEN;
            move_counter = 2;            
        }
        else
            current_flags |= FLAG_BUTTON_OPEN;
    }
    else
    {
        if (current_flags & FLAG_BUTTON_OPEN)
            requested_direction = DIR_STOPPED;
            
        current_flags &= ~FLAG_BUTTON_OPEN;        
    }
    
    if (!gpio_input_read(&button_close))
    {
        if ((current_flags & FLAG_BUTTON_CLOSE) && !(current_flags & FLAG_LIMIT_CLOSED))
        {
            requested_direction = DIR_CLOSE;
            move_counter = 2;            
        }
        else
            current_flags |= FLAG_BUTTON_CLOSE;
    }
    else
    {
        if (current_flags & FLAG_BUTTON_CLOSE)
            requested_direction = DIR_STOPPED;
            
        current_flags &= ~FLAG_BUTTON_CLOSE;        
    }      

    if ((current_direction == DIR_OPEN && (current_flags & FLAG_LIMIT_OPEN)) ||
        (current_direction == DIR_CLOSE && (current_flags & FLAG_LIMIT_CLOSED)) ||
        (move_counter > 0 && --move_counter == 0))
    {
        gpio_output_set_low(&drive_en_l);
        gpio_output_set_low(&drive_en_r);
        current_direction = requested_direction = DIR_STOPPED;
        move_counter = 0;
    }
    
    // Make sure we are stopped before changing direction
    if (current_direction != DIR_STOPPED && requested_direction != current_direction)
    {
        gpio_output_set_low(&drive_en_l);
        gpio_output_set_low(&drive_en_r);
        current_direction = DIR_STOPPED;
    }
    else if (requested_direction == DIR_OPEN)
    {
        gpio_output_set_high(&drive_pwm_l);
        gpio_output_set_low(&drive_pwm_r);

        gpio_output_set_high(&drive_en_l);
        gpio_output_set_high(&drive_en_r);
        current_direction = DIR_OPEN;
    }
    else if (requested_direction == DIR_CLOSE)
    {
        gpio_output_set_low(&drive_pwm_l);
        gpio_output_set_high(&drive_pwm_r);

        gpio_output_set_high(&drive_en_l);
        gpio_output_set_high(&drive_en_r);
        current_direction = DIR_CLOSE;
    }
    
    if (current_direction != DIR_STOPPED)
        current_flags |= FLAG_MOVING;
    else
        current_flags &= ~FLAG_MOVING;
}
