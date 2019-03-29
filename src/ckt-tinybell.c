/*************************************************************************
Title:    CKT-TINYBELL Railroad Crossing Bell Circuit
Authors:  Nathan D. Holmes <maverick@drgw.net>
          Based on the work of David Johnson-Davies - www.technoblogy.com - 23rd October 2017
           and used under his Creative Commons Attribution 4.0 International license
File:     $Id: $
License:  GNU General Public License v3

CREDIT:
    The basic idea behind this playback design came from David Johson-Davies, who
    provided the basic framework and the place where I started.

LICENSE:
    Copyright (C) 2019 Michael Petersen, Nathan Holmes, with portions from 
     David Johson-Davies under a Creative Commons Attribution 4.0 license

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

*************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>

// 16kHz 8 bit unsigned
#include "tinybell.h"

uint8_t playBell = 0;


// 16kHz interrupt to load high speed PWMs
ISR(TIMER0_COMPA_vect) 
{
	static uint16_t wavIdx = 0;
	uint8_t sample = pgm_read_byte(&tinybell_16k_wav[wavIdx++]);
	OCR1A = sample; 
	OCR1B = sample ^ 255;

	if (wavIdx == tinybell_16k_wav_len)
	{
		wavIdx = 0;
		if (!playBell)
		{
			// Stop timer0 ISR
			TIMSK = 0;
			// Stop speaker output
			OCR1A = OCR1B = 0x7F;
		}
	}
}

int main(void)
{
	// Enable 64 MHz PLL and use as source for Timer1
	PLLCSR = _BV(PCKE) | _BV(PLLE);

	// Set up Timer/Counter1 for PWM output
	TIMSK = 0;                              // Timer interrupts OFF
	TCCR1 = _BV(PWM1A) | 2<<COM1A0 | 1<<CS10; // PWM A, clear on match, 1:1 prescale
	GTCCR = _BV(PWM1B) | 2<<COM1B0;           // PWM B, clear on match
	OCR1A = OCR1B = 0x7F;               // 50% duty at start

	// Set up Timer/Counter0 for 8kHz interrupt to output samples.
	TCCR0A = 3<<WGM00;                        // Fast PWM
	TCCR0B = 1<<WGM02 | 2<<CS00;              // 1/8 prescale
	TIMSK = _BV(OCIE0A);                      // Enable compare match
	OCR0A = 63;                               // Divide by ~500 (16kHz)

	DDRB |= _BV(PB4) | _BV(PB1);
	PORTB |= _BV(PB3); // Turn on pullup for PB3

	sei();

	while(1)
	{
		if (PINB & _BV(PB3))
			playBell = 0;
		else
		{
			playBell = 1;
			TIMSK = 1<<OCIE0A;
		}
	}
	
}

