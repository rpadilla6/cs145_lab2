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
char str[17];
char out[17];
int total;

int main(void){
	avr_init();	
	lcd_init();
	struct datetime dt = {2019, 2, 11, 9, 12, 15, 16};
	display_time(&dt);
	for(;;){
		avr_wait(10);
		keep_time(&dt);
		display_time(&dt);
		int key = get_key();
		switch(key){
			case 4:
				set_date(&dt);
				set_time(&dt);
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

void display_time(struct datetime *dt){
	sprintf(out, "%d/%d/%d", dt->month, dt->day, dt->year);
	lcd_clr();
	lcd_pos(0,1);
	lcd_puts2(out);
	lcd_pos(1,1);
	sprintf(out, "%d:%d:%d:%d", dt->hour, dt->minute, dt->second, dt->subsecond);
	lcd_puts2(out);
}

void set_time(struct datetime *dt){
	/*
	lcd_clr();
	lcd_pos(0,1);
	strcpy(str, "24 hr?: ");
	dt->military = get_num();
	avr_wait(200);
	*/
	
	lcd_clr();
	lcd_pos(0,1);
	strcpy(str, "Minute: ");
	lcd_puts2(str);
	dt->minute = get_num();
	avr_wait(200);
	
	lcd_clr();
	lcd_pos(0,1);
	strcpy(str, "Hour: ");
	lcd_puts2(str);
	dt->hour = get_num();
	avr_wait(200);
	dt->second = 0;
	dt->subsecond = 0;
}

void set_date(struct datetime *dt){
	lcd_clr();
	lcd_pos(0,1);
	strcpy(str, "Day: ");
	lcd_puts2(str);
	dt->day = get_num();
	avr_wait(200);
	
	lcd_clr();
	lcd_pos(0,1);
	strcpy(str, "Month: ");
	lcd_puts2(str);
	dt->month = get_num();
	avr_wait(200);
	
	lcd_clr();
	lcd_pos(0,1);
	strcpy(str, "Year: ");
	lcd_puts2(str);
	dt->year = get_num();
	avr_wait(200);
}

int get_num(void){
	int num = 0;
	for(;;){
		int key = get_key();
		switch(key){
			case 15:
				return num;
			case 0:
			case 4:
			case 8:
			case 12: 
			case 13:
			case 16: // do nothing in this case
				break;
			default:
				key = key - ((key-1)/4);
				if(key == 11) key = 0;
				num = (num * 10) + key;
				sprintf(out, "%d", key);
				lcd_puts2(out);
				avr_wait(200);
				break;
		}
	}
}

void keep_time(struct datetime *date){
	if(++(date->subsecond) > 9){
		date->subsecond = 0;
		if(++(date->second) > 59){
			date->second = 0;
			if(++(date->minute) > 59){
				date->minute = 0;
				if(++(date->hour) > 23){
					date->hour = 0;
					keep_date(date);
				}
			}
		}
	}
}

void keep_date(struct datetime *date){
	date->day++;
	char extra = 0;
	switch(date->month){
		case 2:
			if((date->year % 4) == 0) extra = 1;
			if(date->day > (28+extra)){
				date->month++;
				date->day = 1;
			}
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			if(date->day > 30){
				date->month++;
				date->day = 1;
			}
			break;
		default:
			if(date->day > 31){
				date->day = 1;
				if(++(date->month)>12){
					date->month = 1;
					++(date->year);
				}
			}
			break;
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