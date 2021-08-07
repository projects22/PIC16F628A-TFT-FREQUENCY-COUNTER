
// TFT Frequency Counter, by www.moty22.co.uk
//
// XC8 and MPLABX.
// pic16f628a
   
#include <htc.h>
#include "tftfont.c"

#pragma config WDTE=OFF, MCLRE=OFF, BOREN=OFF, FOSC=HS, CP=OFF, CPD=OFF, LVP=OFF

#define _XTAL_FREQ 16000000
#define __delay_us(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000000.0)))
#define __delay_ms(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000.0)))
#define CS   RB4 // pin definition for Arduino UNO
#define DC   RB3   // AO
#define SDA  RB1
#define SCK  RB0
#define RES  RB2
// Color definitions
#define black   0x0000
#define blue    0x001F
#define red     0xF800
#define green   0x07E0
#define cyan    0x07FF
#define magenta 0xF81F
#define yellow  0xFFE0  
#define white   0xFFFF
#define yf 10  
#define yp 74 

//prototypes
void SPI(unsigned char data);
void command(unsigned char cmd);
void TFTinit(void);
void main(void);
void send_data(unsigned char data);
void area(unsigned char x0,unsigned char y0, unsigned char x1,unsigned char y1);
void rectan(unsigned char x0,unsigned char y0, unsigned char x1,unsigned char y1, unsigned int color);
//void pixel(unsigned char x,unsigned char y, unsigned int color);
void chr(unsigned char x, unsigned char y, unsigned char fig, unsigned int seg_color);
void draw(unsigned char x, unsigned char y, unsigned char c, unsigned int color, unsigned char size);

void main(void)
{
	unsigned long total;
	unsigned char timebase,nz,i,d[7];
	unsigned int freq2;
		
	// PIC I/O init
    CMCON = 0b111;		//comparator off
	TRISB = 0b00000000;		 //RB 5,6,7 inputs
    TRISA = 0b11110100;

    OPTION_REG = 0b11111000;	//tmr0 1:1
    T1CON=0b100000;		//timer OFF, 1:4
	CCP1CON=0b1011;		//1011 = Compare mode, trigger special event (CCP1IF bit is set; CCP1 resets TMR1.
	CCPR1H=0x9c; CCPR1L=0x40;	//CCP in compare mode sets TMR1 to a period of 40 ms  , 9C40=40000
    SDA=1;
    SCK=1;
    CS=1;
	RES=1;
	
	TFTinit();

	rectan(0,0,159,63,white); 	//
    rectan(0,64,159,127,green); 	//

    draw(135,yp+30,12,red,2);     //S
    draw(119,yf+30,11,blue,2);     //H
    draw(135,yf+30,15,blue,2);     //z


	while(1){

		//Frequency Counter
			nz=0;
			freq2 = 0;	//clear timers
			timebase=25;	//25 * 40ms = 1sec
			TMR1L=0;  TMR1H=0;			
			
			TMR0 = 0;
			T0IF = 0;
			TMR1ON = 1;	//start count
		    
			while(timebase){		//1 sec 
				if(T0IF){++freq2; T0IF = 0;}
				if(CCP1IF){CCP1IF=0; --timebase;}
			}
			TMR1ON = 0;	//stop count
			
			total=(unsigned long)TMR0 + ((unsigned long)freq2 * 256);	//calculate frequency
			//convert binary to 7 decimal digits
			d[6]=total/1000000;		//1MHz digit
			d[5]=(total/100000) %10;	//100KHz digit
			d[4]=(total/10000) %10;
			d[3]=(total/1000) %10;
			d[2]=(total/100) %10;
			d[1]=(total/10) %10;		//10Hz digit	
			d[0]=total %10;
				//display digits 7 to 2
			for(i=6;i>0;i--){
				if(!d[i] && !nz){draw(135-i*20,yf,10,white,3);}	
				else{draw(135-i*20,yf,10,white,3); draw(135-i*20,yf,d[i],red,3); nz=1;}
 			}	
            draw(135,yf,10,white,3) ;draw(135,yf,d[0],red,3);
			
			nz=0;
			draw(119,yp+30,10,green,2);
			if(total<1000){
                total=1000000/total; 
                draw(119,yp+30,14,red,2);	//u
            }else{
                total=1000000000/total;
                draw(119,yp+30,13,red,2);}	//n
			
			//convert binary to 7 decimal digits
			d[6]=total/1000000;		//
			d[5]=(total/100000) %10;
			d[4]=(total/10000) %10;
			d[3]=(total/1000) %10;
			d[2]=(total/100) %10;
			d[1]=(total/10) %10;		//	
			d[0]=total %10;
				//display digits 7 to 2			
			for(i=6;i>0;i--){
                if(!d[i] && !nz){draw(135-i*20,yp,10,green,3);}	
				else{draw(135-i*20,yp,10,green,3); draw(135-i*20,yp,d[i],black,3); nz=1;}
			}	
			draw(135,yp,10,green,3) ;draw(135,yp,d[0],black,3);
            
			__delay_ms(2000);
	}	
}

void draw(unsigned char x, unsigned char y, unsigned char c, unsigned int color, unsigned char size) //character
{
	unsigned char i, j, line;
  for (i=0; i<6; i++ ) {
     if (i == 5) 
      line = 0x0;
    else 
      line = font[(c*5)+i];
    for (j = 0; j<8; j++) {
      if (line & 0x1) {
         rectan(x+(i*size), y+(j*size), x+(i*size)+size, y+(j*size)+size, color);
       }
      line >>= 1;
    }
  }
}

void SPI(unsigned char data)		// send character over SPI
{
	unsigned char b;
    
    SDA=1; SCK=1;
    for(b=0;b<8;b++){
    SCK=0;
    SDA=(data >> (7-b)) % 2;
    SCK=1;
	}
}

void command(unsigned char cmd)
{
	DC=0;	// Command Mode
	CS=0;	// Select the LCD	(active low)
	SPI(cmd);	// set up data on bus
	CS=1;	// Deselect LCD (active low)
}

void send_data(unsigned char data)
{
	DC=1;       // data mode
	CS=0;       // chip selected
	SPI(data);	// set up data on bus
	CS=1;       // deselect chip
}

void TFTinit(void)
{
	unsigned char i;
	RES=1;			//hardware reset
	__delay_ms(200);
	RES=0;
	__delay_ms(10);
	RES=1;
	__delay_ms(10);
	
	command(0x01); // sw reset
	__delay_ms(200);

  	command(0x11); // Sleep out
 	__delay_ms(200);
	  
	  command(0x3A); //color mode
	  send_data(0x05);	//16 bits
	    
	  command(0x36); //Memory access ctrl (directions)
	  send_data(0B1100000);  //0x60
//	  command(0x21); //inversion on
	  
	command(0x2D);	//color look up table
	send_data(0); for(i=1;i<32;i++){send_data(i*2);}
	for(i=0;i<64;i++){send_data(i);}	
	send_data(0); for(i=1;i<32;i++){send_data(i*2);}	  
	  
	  command(0x13); //Normal display on
	  command(0x29); //Main screen turn on
}	  

void area(unsigned char x0,unsigned char y0, unsigned char x1,unsigned char y1)
{
  command(0x2A); // Column addr set
  send_data(0x00);
  send_data(x0);     // XSTART 
  send_data(0x00);
  send_data(x1);     // XEND

  command(0x2B); // Row addr set
  send_data(0x00);
  send_data(y0);     // YSTART
  send_data(0x00);
  send_data(y1);     // YEND

  command(0x2C); // write to RAM
}  
	
void rectan(unsigned char x0,unsigned char y0, unsigned char x1,unsigned char y1, unsigned int color) 
{
 unsigned int i;
  area(x0,y0,x1,y1);
  for(i=(y1 - y0 + 1) * (x1 - x0 + 1); i > 0; i--) {		  

	    DC=1;       // data mode
		CS=0;
		SPI(color >> 8);
		SPI(color);
		CS=1;
  }
}

//void pixel(unsigned char x,unsigned char y, unsigned int color)
//{
//	  area(x,y,x+1,y+1);
//	  send_data(color >> 8);
//	  send_data(color);
//}

