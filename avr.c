#include "avr.h"

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

int main(){
	avr_init();	
	
}

int is_pressed(int row, int col){
	//set all rows, cols to n/c
	DDRC=0x00;
	//set col to strong 0
	CLR_BIT(PORTC, col);
	//set row to weak 1
	SET_BIT(PORTC, row);
	
	if(GET_BIT(PINC, row)){
		return 0;
	}
	return 1;
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