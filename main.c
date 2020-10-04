/*
 * project4.c
 *
 * Created: 3/5/20 10:37:47 PM
 * Author : cesar
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <math.h>
#include "avr.h"
#include "lcd.h"

#define VOLTS(i) 5.0 * (double)(i) / 1023.0

uint16_t ADCresult = 0;
double average = 0.0;
double total = 0.0;
double min = 99999.0;
double max = 0;
int counter = 1;

char keypad[17] = {
	'1', '2', '3', 'A',
	'4', '5', '6', 'B',
	'7', '8', '9', 'C',
	'*', '0', '#', 'D'
};


int is_pressed(int r, int c)
{
	DDRC = 0x00;
	PORTC = 0x00;
	
	SET_BIT(DDRC, r);
	SET_BIT(PORTC, c + 4);
	avr_wait(1);
	
	if (GET_BIT(PINC,c + 4)) {
		return 0;
	}
	
	return 1;
}

int get_key(void)
{
	int r, c;
	for (r = 0; r < 4; ++r)
	{
		for(c = 0; c < 4; ++c)
		{
			if(is_pressed(r,c))
			return (r *4) + c + 1;
		}
	}
	return 0;
}

void reverse(char* str, int len)
{
	int i = 0, j = len - 1, temp;
	while (i < j) {
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++;
		j--;
	}
}

int intToStr(int x, char str[], int d)
{
	int i = 0;
	while (x) {
		str[i++] = (x % 10) + '0';
		x = x / 10;
	}
	
	while (i < d)
	str[i++] = '0';
	
	reverse(str, i);
	str[i] = '\0';
	return i;
}

void ftoa(float n, char* res, int afterpoint)
{
	int ipart = (int)n;

	float fpart = n - (float)ipart;

	int i = intToStr(ipart, res, 0);

	if (afterpoint != 0) {
		res[i] = '.';

		fpart = fpart * pow(10, afterpoint);
		
		intToStr((int)fpart, res + i + 1, afterpoint);
	}
}


void get_sample()
{
	SET_BIT(ADCSRA, 7);	//Sets ADEN to 1
	SET_BIT(ADCSRA, 6); //Sets ADSC to 1
	SET_BIT(ADMUX, 6); //AVCC with external capacitor at AREF pin
	
	while(! (ADCSRA & (1 << ADIF)));
	ADCresult = ADC;
	
	
	CLR_BIT(ADCSRA, 7); //ADC is turned off, terminates conversion
	
}

void reset()
{
	min = 99999.0;
	max = 0.0;
	average = 0.0;
	total = 0.0;
	counter = 1;
}

int main(void)
{
	lcd_init();
	ADCSRA |= 7; //Sets ADPS2...0 to 1
	while (1)
	{
		get_sample();
		total = total + VOLTS(ADCresult);
		average = total / counter++;
		if (min > VOLTS(ADCresult))
			min = VOLTS(ADCresult);
		if (max < VOLTS(ADCresult))
			max = VOLTS(ADCresult);
		char current_buffer[10];
		char average_buffer[10];
		char min_buffer[10];
		char max_buffer[10];
		char buffer[20];
		char buffer2[20];
		ftoa(VOLTS(ADCresult), current_buffer, 2);
		ftoa(average, average_buffer, 2);
		ftoa(min, min_buffer, 2);
		ftoa(max, max_buffer, 2);
		
		sprintf(buffer, "C:%s A:%s", current_buffer, average_buffer);
		sprintf(buffer2, "M:%s m:%s", max_buffer, min_buffer);
		lcd_clr();
		lcd_pos(0, 0);
		lcd_puts2(buffer);
		lcd_pos(1, 0);
		lcd_puts2(buffer2);
		avr_wait(500);
		
		if (keypad[get_key() - 1] == '1')
		{
			lcd_clr();
			lcd_puts2("RESET");
			avr_wait(500);
			reset();
			while (1)
			{
				if (keypad[get_key() - 1] == '3')
				{
					lcd_clr();
					lcd_puts2("RESUME");
					avr_wait(500);
					break;
				}
				get_sample();
				ftoa(VOLTS(ADCresult), current_buffer, 2);
				sprintf(buffer, "C:%s A:----",  current_buffer);
				sprintf(buffer2, "M:---- m:----");
				lcd_clr();
				lcd_pos(0, 0);
				lcd_puts2(buffer);
				lcd_pos(1, 0);
				lcd_puts2(buffer2);
				avr_wait(500);
			}
		}
		if (keypad[get_key() - 1] == '2')
		{
			lcd_clr();
			lcd_puts2("HOLD");
			avr_wait(500);
			reset();
			while (1)
			{
				if (keypad[get_key() - 1] == '3')
				{
					lcd_clr();
					lcd_puts2("RESUME");
					avr_wait(500);
					reset();
					break;
				}
				
				if (keypad[get_key() - 1] == '1')
				{
					lcd_clr();
					lcd_puts2("RESET");
					avr_wait(500);
					reset();
					sprintf(min_buffer, "----");
					sprintf(max_buffer, "----");
					sprintf(average_buffer, "----");
				}
				
				get_sample();
				ftoa(VOLTS(ADCresult), current_buffer, 2);
		
				sprintf(buffer, "C:%s A:%s", current_buffer, average_buffer);
				sprintf(buffer2, "M:%s m:%s", max_buffer, min_buffer);
				lcd_clr();
				lcd_pos(0, 0);
				lcd_puts2(buffer);
				lcd_pos(1, 0);
				lcd_puts2(buffer2);
				avr_wait(500);
			}
		}
	}
	

}

