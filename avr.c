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


int main(void){
	// Initialize everything
	avr_init();	
	lcd_init();
	struct datetime dt = {2019, 2, 11, 19, 12, 5, 16, 0};
	display_time(&dt);
	for(;;){
		//Main loop will check if key pressed, and if it is A or B, do something
		avr_wait(85);
		keep_time(&dt);
		display_time(&dt);
		int key = get_key();
		switch(key){
			// Set date and time
			case 4:
				set_date(&dt);
				set_time(&dt);
				break;
			// Toggle military time
			case 8:
				dt.military = dt.military^1;
				avr_wait(150);
				break;
			default:
				break;
		}
	}
}
// Display the time to lcd
void display_time(struct datetime *dt){
	sprintf(out, "%d/%d/%d", dt->month, dt->day, dt->year);
	lcd_clr();
	lcd_pos(0,1);
	lcd_puts2(out);
	lcd_pos(1,1);
	// If military time, display without special formatting (default)
	if(dt->military){
		sprintf(out, "%02d:%02d:%02d:%d", dt->hour, dt->minute, dt->second, dt->subsecond);
	}
	else{
		// Otherwise some special cases are in play for AM/PM
		if(dt->hour > 12){
			sprintf(out, "%02d:%02d:%02d:%d %s", (dt->hour - 12), dt->minute, dt->second, dt->subsecond, "PM");
		}
		else if(dt->hour == 0){
			sprintf(out, "%02d:%02d:%02d:%02d %s", (dt->hour + 12), dt->minute, dt->second, dt->subsecond, "AM");
		}
		else{
			sprintf(out, "%02d:%02d:%02d:%d %s", dt->hour, dt->minute, dt->second, dt->subsecond, "AM");
		}
	}
	lcd_puts2(out);
}

/************************************************************************/
/* Prompt user to enter in first minute, then hour.                     */
/************************************************************************/
void set_time(struct datetime *dt){
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
/************************************************************************/
/* Prompt user to enter in first day, then month, then year             */
/************************************************************************/
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
/************************************************************************/
/* Gets actual keypad value (numbers 0-9)                               */
/************************************************************************/
int get_num(void){
	int num = 0;
	for(;;){
		int key = get_key();
		switch(key){
			// # is enter key
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

/************************************************************************/
/* Runs within main loop to keep time                                   */
/************************************************************************/
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
/************************************************************************/
/* Called by keep time function                                         */
/************************************************************************/
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

/************************************************************************/
/* Check for if a certain button is pressed                             */
/************************************************************************/
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

/************************************************************************/
/* Get raw key pressed, different than get_num which does some conversion*/
/* for keypad numbers                                                   */
/************************************************************************/
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