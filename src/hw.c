#include <p18cxxx.h>
#include <delays.h>
#include <stdio.h>
#include <math.h>
#include "hw.h"
#include "main_tasks.h"

extern unsigned char lcd_buff[25];

void init_hw_fast (void)
{
    ANCON0 = 0xFE;
    ANCON1 = 0x1B;
    TRISA = 0x01;
    TRISC = 0xFC;
    TRISB = 0xF3;
    T4CON = 0xFF;
    TMR4 = 0;
    PR4 = 249;
    PIE3bits.TMR4IE = 1;
    INTCON = 0xC0;
    SSP1CON1 = 0x28;
    SSP1CON2 = 0x00;
    SSP1STAT = 0x00;
    SSP1ADD = 40;
    LATBbits.LATB2 = 0;
}

void init_hw_slow (void)
{
    ANCON0 = 0xFE;
    ANCON1 = 0x1B;
    TRISA = 0x01;
    TRISC = 0xFC;
    TRISB = 0xF3;
    T4CON = 0x27;
    TMR4 = 0;
    PR4 = 249;
    PIE3bits.TMR4IE = 1;
    INTCON = 0xC0;
    SSP1CON1 = 0x28;
    SSP1CON2 = 0x00;
    SSP1STAT = 0x00;
    SSP1ADD = 10;
    LATBbits.LATB2 = 0;
}


void iic_write_reg (unsigned char hw_addr, unsigned char addr, unsigned char data)
{
iic_start();
iic_write(hw_addr);
iic_write(addr);
iic_write(data);
iic_stop();
}

unsigned char iic_read_reg (unsigned char hw_addr, unsigned char addr)
{
unsigned char temp;
iic_start();
iic_write(hw_addr);
iic_write(addr);
iic_restart();
iic_write(hw_addr+1);
temp = iic_read(1);
iic_stop();
return temp;
}


void iic_init (void)
{
}

void iic_start (void)
{
SSP1CON2bits.SEN = 1;
while (SSP1CON2bits.SEN);
}

void iic_restart (void)
{
SSP1CON2bits.RSEN = 1;
while (SSP1CON2bits.RSEN);
}

unsigned char iic_read (unsigned char ack)
{
unsigned char temp;
PIR1bits.SSP1IF = 0;
SSP1CON2bits.RCEN = 1;
while (PIR1bits.SSP1IF==0);
temp = SSP1BUF;

if (ack==0) SSP1CON2bits.ACKDT = 0;
else SSP1CON2bits.ACKDT = 1;
PIR1bits.SSP1IF = 0;
SSP1CON2bits.ACKEN = 1;
while (PIR1bits.SSP1IF==0);
return temp;
}

void iic_stop (void)
{
SSP1CON2bits.PEN = 1;
while (SSP1CON2bits.PEN);
}

void iic_write (unsigned char data)
{
PIR1bits.SSP1IF = 0;
SSP1BUF = data;
while (PIR1bits.SSP1IF==0);
}


void lcd_ll_set_dl (unsigned char data)
{
if (data&0x01) 	DISP_DB4 = 1;
	else		DISP_DB4 = 0;
if (data&0x02) 	DISP_DB5 = 1;
	else		DISP_DB5 = 0;
if (data&0x04) 	DISP_DB6 = 1;
	else		DISP_DB6 = 0;
if (data&0x08) 	DISP_DB7 = 1;
	else		DISP_DB7 = 0;
}

void __delay_ms(unsigned int val)
{
unsigned int i;
for (i=0;i<val;i++) Delay1KTCYx(1);
}


void initLCD(void)
{
	DISP_RS = 0;
	__delay_ms(15);
	lcd_ll_set_dl(3);
	DISP_EN=1;
	DISP_EN=0;
	__delay_ms(4);
	DISP_EN=1;
	DISP_EN=0;
	__delay_ms(4);
	DISP_EN=1;
	DISP_EN=0;
	__delay_ms(4);
	lcd_ll_set_dl(2);
	DISP_EN=1;
	DISP_EN=0;
	__delay_ms(4);
	lcdc(0x28);		//!!!2lines!!!, 4bit,5*7font
	lcdc(0x0F);		//disp on,blink & cursor off
	lcdc(0x01);		//erase disp, cursor home
	__delay_ms(4);

    lcdc(0x40+8);
    lcdt(0b01110);
    lcdt(0b10000);
    lcdt(0b10000);
    lcdt(0b11111);
    lcdt(0b11011);
    lcdt(0b11011);
    lcdt(0b11111);
    lcdt(0b00000);

    lcdc(0x40+16);
    lcdt(0b01110);
    lcdt(0b10001);
    lcdt(0b10001);
    lcdt(0b11111);
    lcdt(0b11011);
    lcdt(0b11011);
    lcdt(0b11111);
    lcdt(0b00000);


}

void lcdc (unsigned char data)
{
	unsigned char datat;
	datat=data;
	DISP_RS=0;
	DISP_EN=1;
	datat>>=4;
	lcd_ll_set_dl(datat&0x0F);
	DISP_EN=0;
	DISP_RS=0;
	DISP_EN=1;
	lcd_ll_set_dl(data&0x0F);
	DISP_EN=0;
	__delay_ms(1);
}


void lcdt (unsigned char data)
{
	unsigned char datat;
        if (data=='~') data  = 0;
        if (data=='!') data  = 1;
        if (data=='@') data  = 2;
        if (data=='#') data  = 3;
	datat=data;
	DISP_RS=1;
	DISP_EN=1;
	datat>>=4;
	lcd_ll_set_dl(datat&0x0F);
	DISP_EN=0;
	DISP_RS=1;
	DISP_EN=1;
	lcd_ll_set_dl(data&0x0F);
	DISP_EN=0;
	if (data==0x01)
		__delay_ms(4);
	else
		__delay_ms(1);
}

void refresh_disp(unsigned char * data)
{
unsigned char i;
lcdc(0x80);
for (i=0;i<8;i++)
	{
	if ((data[i]>=0x20)&(data[i]<0x7F))
		lcdt(data[i]);
	else
		lcdt(0x20);
	}
lcdc(0xC0);
for (i=0;i<8;i++)
	{
	if ((data[i+8]>=0x20)&(data[i+8]<0x7F))
		lcdt(data[i+8]);
	else
		lcdt(0x20);
	}
}

void clear_disp_buffer (void)
{
    unsigned char i;
for (i=0;i<16;i++)
    lcd_buff[i]=' ';
}

void set_battery_char (unsigned char level)
{
if (level==0)
    {
    lcdc(0x40+0);
    lcdt(0b01110);
    lcdt(0b11111);
    lcdt(0b10001);
    lcdt(0b10001);
    lcdt(0b10001);
    lcdt(0b10001);
    lcdt(0b11111);
    }
if (level==1)
    {
    lcdc(0x40+0);
    lcdt(0b01110);
    lcdt(0b11111);
    lcdt(0b10001);
    lcdt(0b10001);
    lcdt(0b10001);
    lcdt(0b11111);
    lcdt(0b11111);
    }
if (level==2)
    {
    lcdc(0x40+0);
    lcdt(0b01110);
    lcdt(0b11111);
    lcdt(0b10001);
    lcdt(0b10001);
    lcdt(0b11111);
    lcdt(0b11111);
    lcdt(0b11111);
    }
if (level==3)
    {
    lcdc(0x40+0);
    lcdt(0b01110);
    lcdt(0b11111);
    lcdt(0b10001);
    lcdt(0b11111);
    lcdt(0b11111);
    lcdt(0b11111);
    lcdt(0b11111);
    }
if (level==4)
    {
    lcdc(0x40+0);
    lcdt(0b01110);
    lcdt(0b11111);
    lcdt(0b11111);
    lcdt(0b11111);
    lcdt(0b11111);
    lcdt(0b11111);
    lcdt(0b11111);
    }

}


unsigned int read_mem_location (unsigned int num, unsigned char bank)
{
unsigned char lh,ll;
unsigned long addr;
unsigned int dat,offset;
offset = bank;
offset = offset*8192;
addr = num;
addr = (addr * 2) + offset;
lh = iic_mem_read(0xA0, addr);
ll = iic_mem_read(0xA0, addr+1);
dat = (((unsigned int)(lh))<<8) | ll;
return dat;
}

void write_mem_location(unsigned int data, unsigned int num, unsigned char bank)
{
unsigned long addr,offset;
offset = bank;
offset = offset*8192;
addr = num;
addr = (addr * 2) + offset;
iic_mem_write(0xA0,addr,data>>8);
__delay_ms(10);
iic_mem_write(0xA0,addr+1,data);
__delay_ms(10);
}

void iic_mem_write (unsigned int hw_addr, unsigned long addr, unsigned char data)
{
iic_start();
iic_write(hw_addr);
iic_write(addr>>8);
iic_write(addr);
iic_write(data);
iic_stop();
}

unsigned char iic_mem_read (unsigned int hw_addr, unsigned long addr)
{
unsigned char temp;
iic_start();
iic_write(hw_addr);
iic_write(addr>>8);
iic_write(addr);
iic_restart();
iic_write(hw_addr+1);
temp = iic_read(1);
iic_stop();
return temp;
}


unsigned int get_adc (unsigned char chnl)
{
ADCON1 = 0xB3;
ADCON0 = ((chnl<<2)&0x3C);
ADCON0bits.ADON = 1;
ADCON0bits.GO_DONE=1;
while (ADCON0bits.GO_DONE==1);
return ADRES;
}

unsigned int get_batt_volts (void)
{
unsigned long adc;
unsigned int ret;
adc = get_adc(ADC_CHNL_BMON);
adc = adc * 3300;
adc = adc / 1023;
adc = adc * 2;
ret = (unsigned int)(adc);
return ret;
}

unsigned char get_batt_level (unsigned int voltage)
{
unsigned char ret;
if (voltage<3500)
    ret = 0;
else if (voltage<3700)
    ret = 1;
else if (voltage<3850)
    ret = 2;
else if (voltage<4000)
    ret = 3;
else ret = 4;
return ret;
}

