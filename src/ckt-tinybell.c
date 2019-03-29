/* Audio Sample Player v2

   David Johnson-Davies - www.technoblogy.com - 23rd October 2017
   ATtiny85 @ 8 MHz (internal oscillator; BOD disabled)
      
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

/* Direct-coupled capacitorless output */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>

// 16kHz 8 bit unsigned
#include "tinybell.h"

uint8_t playBell = 0;

// Sample interrupt
ISR(TIMER0_COMPA_vect) 
{
	static uint16_t wavIdx = 0;
	uint8_t sample = pgm_read_byte(&tinybell_16k_wav[wavIdx++]);
	OCR1A = sample; OCR1B = sample ^ 255;

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
	PLLCSR = 1<<PCKE | 1<<PLLE;     

	// Set up Timer/Counter1 for PWM output
	TIMSK = 0;                              // Timer interrupts OFF
	TCCR1 = 1<<PWM1A | 2<<COM1A0 | 1<<CS10; // PWM A, clear on match, 1:1 prescale
	GTCCR = 1<<PWM1B | 2<<COM1B0;           // PWM B, clear on match
	OCR1A = 128; OCR1B = 128;               // 50% duty at start

	// Set up Timer/Counter0 for 8kHz interrupt to output samples.
	TCCR0A = 3<<WGM00;                      // Fast PWM
	TCCR0B = 1<<WGM02 | 2<<CS00;            // 1/8 prescale
	TIMSK = 1<<OCIE0A;                      // Enable compare match
	OCR0A = 63;                             // Divide by 1000

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

