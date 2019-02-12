#include "avr.h"
#include <stdio.h>
#include <string.h>
#include "lcd.h"

void
avr_init(void)
{
	WDTCR = 15;
}

void
avr_wait(unsigned short msec)
{
	TCCR0 = 3;
	while (msec--) {
		TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 64) * 0.001);
		SET_BIT(TIFR, TOV0);
		WDR();
		while (!GET_BIT(TIFR, TOV0));
	}
	TCCR0 = 0;
}
char str[25];
char out[25];
int total;
char day;
char month;
short year;
char hour;
char minute;
char second;

int main(void){
	avr_init();	
	lcd_init();
	lcd_pos(0,1);
	for(;;){
		int key = get_key();
		switch(key){
			case 4:
				set_time();
				break;
			case 8:
				// Toggle 12hr/24hr
				break;
			default:
				break;
		}
		/*
		if(key){
			sprintf(str, "%d", key);
			lcd_puts2(str);
			avr_wait(200);
		}
		*/
	}
}

void set_time(void){
	strcpy(str, "Enter day: ");
	lcd_puts2(str);
	day = get_num();
	lcd_clr();
	lcd_pos(0,1);
	strcpy(str, "Enter month: ");
	month = get_num();
	lcd_clr();
	lcd_pos(0,1);
	strcpy(str, "Enter year: ");
	year = get_num();
}

int get_num(void){
	char num = 0;
	for(;;){
		int key = get_key();
		switch(key){
			case 16:
				return num;
			case 4 || 8 || 12 || 13 || 15: // do nothing in this case
				break;
			default:
				num = (num * 10) + key;
				break;
		}
	}
}

void blink(int num){
	int i;
	for(i=0;i<num;i++){
		SET_BIT(PORTB, 0);
		avr_wait(500);
		CLR_BIT(PORTB, 0);
		avr_wait(500);
	}
}

int is_pressed(int row, int col){
	//set all rows, cols to n/c
	DDRC=0;
	PORTC=0;
	//set col to strong 0
	SET_BIT(DDRC, col+4);
	//set row to weak 1
	SET_BIT(PORTC, row);
	avr_wait(1);
	return !GET_BIT(PINC, row);
}
int get_key(){
	int r,c;
	for(r=0;r<4;++r){
		for(c=0;c<4;++c){
			if(is_pressed(r,c)){
				return 1+(r*4)+c;
			}
		}
	}
	return 0;
}