#include <REGX52.H>
#include <stdio.h>
#include <stdlib.h>

//LCD
//#define LCD_DATA P1;
sfr  LCD_DATA = 0x80; //P0

sbit LCD_RS = P2^0;
sbit LCD_E  = P2^1;

//SPI
sbit SPI_SCLK = P3^0;
sbit SPI_MOSI = P3^1;
sbit SPI_MISO = P3^2;
sbit SPI_SS		= P3^3;

//DELAY
void delay_ms(unsigned char ms);
unsigned char reverse_BitInByte(unsigned char byte);


//func-LCD
void LCD_Init();
void LCD_Cmd(unsigned char a);
void LCD_DisplayData(unsigned char a);
void LCD_DisplayStr(unsigned char *str);
void hex2ascii(unsigned short value);


//func-SPI
void SPI_Init();
void SPI_SendByte(unsigned int b);
unsigned char SPI_ReceiveByte();
unsigned char SPI_SendAndReceiByte(unsigned char b);

//func-ADC
unsigned short ADC_value();
unsigned short ADCtoVin(unsigned short);


////////////MAIN
void main(){
	unsigned short value;
	//Init ham khoi tao
	LCD_Init();
	SPI_Init();
	
	
	//unsigned char temp;
	
	
	LCD_DisplayStr("___WELLCOM TO___");
	LCD_Cmd(0xC0);												//di chuyen con tro den dau dong 2
	LCD_DisplayStr("___MY PROJECT___");
	delay_ms(2000);
	
	LCD_Cmd(0x01);		//Xoa toan bo man hinh hien thi
	delay_ms(1);
	LCD_Cmd(0x80);		//di chuyen con tro den dau dong 1
	
	LCD_DisplayStr("___MY PROJECT___");
	LCD_Cmd(0xC0);												//di chuyen con tro den dau dong 2
	LCD_DisplayStr("____NHOM  13____");
	delay_ms(2000);
	
	LCD_Cmd(0x01);		//Xoa toan bo man hinh hien thi
	delay_ms(1);
	LCD_Cmd(0x80);		//di chuyen con tro den dau dong 1
	
	//Hien thi gia tri ADC
	LCD_DisplayStr("___ADC  VALUE___");
	
	
	while(1){
		LCD_Cmd(0xC5);  							//di chuyen con tro den vi tri 5 dong 2
		value = ADC_value();					//doc gia tri ADC - 12 bit data
		hex2ascii(ADCtoVin(value));		//Hien thi gia tri thanh Vin
		delay_ms(2000);
	}
}

///DELAY
void delay_ms(unsigned int ms) {
    unsigned int i, j; 
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 114; j++);
    }
}
unsigned char reverse_BitInByte(unsigned char byte){
	unsigned char revByte = ((byte&(1<<7))>>7)|((byte&(1<<6))>>5)|((byte&(1<<5))>>3)|((byte&(1<<4))>>1)|((byte&(1<<3))<<1)|((byte&(1<<2))<<3)|((byte&(1<<1))<<5)|((byte&(1<<0))<<7);
	return revByte; 
}

////func-LCD
// LCD CMD
// 0x01		Clear display screen
// 0x04		Shift cursor to left
// 0x05		Shift display right
// 0x06		Shift cursor to right
// 0x07		Shift display left
// 0x08		Display OFF, cursor OFF
// 0x0C		Display ON, cursor OFF
// 0x0E		Display ON, cursor blinking
// 0x0F		LCD on, cursor ON

// 0x38		2 lines and 5×7 matrix
// 0x3C		Activate second line
// 0x80		Cursor to beginning of first line
// 0x8X		Cursor line 1 position x
// 0xC0		Cursor to beginning of second line
// 0xCX		Cursor line 2 position x

void LCD_Init(){
	LCD_Cmd(0x38);		//Su dung 2 dong va ma tran 5*7 cua LCD
	delay_ms(1);	
	LCD_Cmd(0x0F);		//Bat man hinh hien thi, con tro nhap nhay
	delay_ms(1);
	LCD_Cmd(0x01);		//Xoa toan bo man hinh hien thi
	delay_ms(1);
	LCD_Cmd(0x80);		//di chuyen con tro den dau dong 1
	delay_ms(1);	
}
void LCD_Cmd(unsigned char a){
	unsigned char cmd = reverse_BitInByte(a);
	LCD_RS = 0;			//che do lenh
	LCD_DATA = cmd;		//truyen lenh
	LCD_E = 1;			//Keo E len muc 1
	delay_ms(1);
	LCD_E = 0;			//Keo E tu 1 xuong 0 => Kich hoat LCD (falling)
	delay_ms(1);
}
void LCD_DisplayData(unsigned char a){
	unsigned char display = reverse_BitInByte(a);
	LCD_RS = 1;						//che do hien thi data
	LCD_DATA = display;		//truyen lenh
	LCD_E = 1;						//Keo E len muc 1
	delay_ms(1); 
	LCD_E = 0;						//Keo E tu 1 xuong 0 => Kich hoat LCD (falling)
	delay_ms(1);
}
void LCD_DisplayStr(unsigned char *str){
	while(*str){								// dung khi het string
		LCD_DisplayData(*str++);	//str tu dong tang len 1 sau khi hien thi
	}
}
void hex2ascii(unsigned short value){ //1234
		unsigned char d1, d2, d3, d4, x;
		unsigned short x1;
		
		x1 = value / 10; 							//123
		x  = (unsigned char)(x1 / 10);//12
		d1 = x / 10;									//1
		d2 = x % 10;									//2
		d3 = x1 % 10;									//3
		d4 = value % 10;							//4
	
		//Display ascii
		LCD_DisplayData(d1 + 0x30); 
		LCD_DisplayStr(".");
		LCD_DisplayData(d2 + 0x30);
		LCD_DisplayData(d3 + 0x30);
		LCD_DisplayData(d4 + 0x30);
		LCD_DisplayStr("V");
}


//func-SPI
void SPI_Init(){
	SPI_SS 	 = 1; 	//set SS o muc cao khi ranh, va keo xuong de truyen tin hieu
	SPI_SCLK = 0; 	//set SCLK o muc thap, SPOL = 0 - muc thap o che do idle 
	SPI_MISO = 1; 	//set duong vao master o che do cao, san sang nhan tin hieu tu thiet bi Slave 

}
void SPI_SendByte(unsigned char b){
	unsigned char i;
	for(i=0;i<8;i++){ //1 byte = 8 bits
		SPI_SCLK = 0; 	//keo xung clock xuong muc thap de chuan bi gui di
		if(b&0x80)			//lay bit cao nhat cua b
			SPI_MOSI = 1;	//neu bit xet la 1 thi gui 1 di
		else
			SPI_MOSI = 0;	//neu khong phai 1 thi gui 0 di
		SPI_SCLK = 1;		//keo xung clock len de bat dau gui du lieu di (rising)
		b<<=1;					//dich trai 1 bit => xoa bit da gui
		SPI_SCLK = 0; 	//keo xung clock xuong muc thap de cho gui di
	}
}

unsigned char SPI_ReceiveByte(){
	unsigned char i, b;
	for(i=0;i<8;i++){
		b<<=1;					//b dich trai 1 bits => chuan bi cho qua trinh nhan 1 bit thap
		SPI_SCLK = 1; 	//Keo xung clock len => bat dau nhan tin hieu (rising)
		if(SPI_MISO)		//Neu tin hieu nhan duoc = 1
			b|=0x01;			// set gia tri bits thap nhat len 1
		SPI_SCLK = 0; 	//Keo xung clock xuong, che do cho
	}
	return b;					//Tra ve tin hieu vua nhan duoc
}

unsigned char SPI_SendAndReceiByte(unsigned char b){
	unsigned char i, temp;
	for(i=0;i<8;i++){
		SPI_SCLK = 0; 	//keo xung clock xuong muc thap de cho gui/nhan data
		if(b&0x80)				//SEND: lay bit cao nhat cua b
			SPI_MOSI = 1;		//SEND: neu bit xet la 1 thi gui 1 di
		else
			SPI_MOSI = 0;		//SEND:neu khong phai 1 thi gui 0 di
		temp<<=1;					//RECEIVE: temp dich trai 1 bits => chuan bi cho qua trinh nhan 1 bit thap
		SPI_SCLK = 1;		//keo xung clock len de bat dau gui/nhan du lieu di (rising)
		b<<=1;						//SEND: dich trai 1 bit => xoa bit da gui
		
		if(SPI_MISO)			//RECEIVE: Neu tin hieu nhan duoc = 1
			temp|=0x01;			//RECEIVE: set gia tri bits thap nhat len 1
		
		SPI_SCLK = 0; 	//keo xung clock xuong muc thap de cho gui di
	}
	return temp;			//Tra ve tin hieu vua nhan duoc
}


////func-ADC
//MCP3204 SPI ADC Pin Configuration
//The SPI Packet for one ADC Conversion is made up of 3 bytes.
//Afer MCP3204 return 12bit result of Analog to Digital Conversion (Bit B11 to B0)
//	MOSI	0 0 0 0 0 1 S D2  || D1  D0 x x  x   x  x  x  ||  x  x  x  x  x  x  x  x	=>>
//  MISO	? ? ? ? ? ? ?  ?  || ?   ?  ? N B11 B10 B9 B8 ||  B7 B6 B5 B4 B3 B2 B1 B0	<<=
//Where S is the bit which selects between differential and single ended 
//vi ta chi su dung 1 kenh don CH0 de nhan tin hieu ADC => S=1 single ended
//Where D2,D1,D0 selects the input channel.
//						D2			D1			D0
// Channel 0			0			0			0
// Channel 1			0			0			1
// Channel 2			0			1			0
// Channel 3			0			1			1
unsigned short ADC_value(){
	unsigned char data_high, data_low;
	SPI_SS = 0;		//keo chan chon xuong 0 de bat dau truyen/nhan du lieu
	
	SPI_SendByte(0x06);											//Mode: single-ended. Channel-0 => Byte1:0b00000110=0x06
	data_high = SPI_SendAndReceiByte(0x00);						//Send Byte2:0b00000000=0x00 and receive 4 bit cao cua du lieu (B11-B8)
	data_high&= 0x0F;											//Chi lay 4 bit sau la 4 bit cao cua du lieu (B11-B8)
	data_low  = SPI_ReceiveByte();								//Nhan not 8 bit tiep theo cua data (B7-B0)
	
	SPI_SS = 1;		//set chan chon len 1  de dung truyen/nhan du lieu
	
	//hex2ascii(data_high);
	//hex2ascii(data_low);
	
	return ((data_high<<8) | data_low);
}
unsigned short ADCtoVin(unsigned short value){
	unsigned short Vref = 5000, Vin = 0;//Vin = 5.000
	unsigned char i;
	for(i=0;i<11;i++){
		Vref/= 2;
		if(value&0x800){
			Vin+=Vref;
		}
		value = value << 1;
	}
	return Vin;
}

