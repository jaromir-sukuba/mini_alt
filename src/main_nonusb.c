#include <p18cxxx.h>
#include <delays.h>
#include <stdio.h>
#include <math.h>
#include "main_nonusb.h"

#define	LED	LATBbits.LATB3

#define	DISP_DB4	LATAbits.LATA1
#define	DISP_DB5	LATAbits.LATA2
#define	DISP_DB6	LATAbits.LATA3
#define	DISP_DB7	LATAbits.LATA5
#define	DISP_EN		LATCbits.LATC0
#define	DISP_RS		LATCbits.LATC1

#define K1      PORTCbits.RC7
#define K2      PORTCbits.RC2
#define K3      PORTBbits.RB0

#define PRES_ARR_SIZE   15

unsigned char tbuff[20];
unsigned char lcd_buff[25];
unsigned long pres_arr[PRES_ARR_SIZE];
unsigned long pres_arr_t[PRES_ARR_SIZE];
unsigned char pres_arr_pointer;
unsigned int timer_main, time;


unsigned char main_state, main_substate, meas_state, ndata,batt_level;
unsigned long pres,rpres;
float alt,alt_temp;
unsigned int alti,alti_r;
int ralt;
char temper;

volatile key_var keys, keys_old, keys_new;


unsigned char main_loop_nonusb (unsigned char state)
{
iic_init ();
initLCD();
main_state  = STATE_PRESS;
rpres = 101300;
batt_level = 3;
ralt = 250;
while (1)
    {
    time = get_timer();
    set_battery_char(batt_level);
    refresh_disp(lcd_buff);
    ndata = get_vars(&meas_state,&alt,&pres,&temper,rpres);
    if (ndata!=0) LED = ~ LED;
/*    if (meas_state==MEAS_STATE_DONE)
        {
        alti = alt;
        alti_r = ((alt-((float)(alti)))*10);
        sprintf (lcd_buff,"P:%ld  ",pres);
        sprintf (lcd_buff+8,"A:%d,%1.1d",alti,alti_r);
        refresh_disp(lcd_buff);
        } */
    if (main_state == STATE_START)
        {
        sprintf (lcd_buff,"START         ");
        }
    if (main_state == STATE_PRESS)
        {
        clear_disp_buffer();
        sprintf (lcd_buff,"PRESS  ~  ");
        sprintf (lcd_buff+8,"%ldPa",pres);
        if (keys.k3)
            {
            keys.k3 = 0;
            main_state = STATE_ALT;
            }
        }
    if (main_state == STATE_ALT)
        {
        clear_disp_buffer();
        sprintf (lcd_buff,"ALT    ~   ");
        alti = alt;
        alti_r = ((alt-((float)(alti)))*10);
        sprintf (lcd_buff+8,"%d,%1.1dm",alti,alti_r);
        if (keys.k3)
            {
            keys.k3 = 0;
            main_state = STATE_TEMP;
            }
        }
    if (main_state == STATE_TEMP)
        {
        clear_disp_buffer();
        sprintf (lcd_buff,"TEMP   ~   ");
        sprintf (lcd_buff+8,"%d dC",temper);
        if (keys.k3)
            {
            keys.k3 = 0;
            main_state = STATE_RPRES;
            main_substate  = SSTATE_SET_RPRES_NONE;
            }
        }
    if (main_state == STATE_RPRES)
        {
        if (main_substate==SSTATE_SET_RPRES_NONE)
            {
            clear_disp_buffer();
            sprintf (lcd_buff,"RPRES @~  ");
            sprintf (lcd_buff+8,"%ldhPa",rpres/100);
            if (keys.k3)
                {
                keys.k3 = 0;
                main_state = STATE_MEM;
                }
            if (keys.k2)
                {
                keys.k2=0;
                main_substate  = SSTATE_SET_RPRES_ASK;
                }
            }
        if (main_substate==SSTATE_SET_RPRES_ASK)
            {
            clear_disp_buffer();
            sprintf (lcd_buff,"SET RPRES");
            sprintf (lcd_buff+8,"ALT  DIR");
            if (keys.k1)
                {
                keys.k1 = 0;
                main_substate  = SSTATE_SET_RPRES_ALT;
                }
            if (keys.k3)
                {
                keys.k3=0;
                main_substate  = SSTATE_SET_RPRES_DIR;
                }
            }
        if (main_substate==SSTATE_SET_RPRES_DIR)
            {
            clear_disp_buffer();
            sprintf (lcd_buff,"SET RPRES");
            sprintf (lcd_buff+8,"%ldhPa",rpres/100);
            if (keys.k1)
                {
                keys.k1 = 0;
                rpres = rpres - 100;
                }
            if (keys.k2)
                {
                keys.k2=0;
                main_substate  = SSTATE_SET_RPRES_NONE;
                }
            if (keys.k3)
                {
                keys.k3=0;
                rpres = rpres + 100;
                }
            }
        if (main_substate==SSTATE_SET_RPRES_ALT)
            {
            clear_disp_buffer();
            sprintf (lcd_buff,"SET RALT");
            sprintf (lcd_buff+8,"%d m",ralt);
            if (keys.k1)
                {
                keys.k1 = 0;
                ralt--;
                }
            if (keys.k2)
                {
                keys.k2=0;
                rpres = calc_pres(ralt,pres);
                main_substate  = SSTATE_SET_RPRES_NONE;
                }
            if (keys.k3)
                {
                keys.k3=0;
                ralt++;
                }
            }

        }

    if (main_state == STATE_MEM)
        {
        clear_disp_buffer();
        sprintf (lcd_buff,"MEM   !~ ");
        if (keys.k3)
            {
            keys.k3 = 0;
            main_state = STATE_OFF;
            }
        }
    if (main_state == STATE_OFF)
        {
        clear_disp_buffer();
        sprintf (lcd_buff,"OFF    ~  ");
        if (keys.k3)
            {
            keys.k3 = 0;
            main_state = STATE_PRESS;
            }
        }

    }

return state;
}
//
//float get_alt (unsigned char * state)
//{
//float alt;
//unsigned char stat;
//alt = 0;
//stat = *state;
//if (stat==MEAS_STATE_START)
//    {
//    iic_write_reg(0xC0, 0x26, 0x3A);
//    stat=MEAS_STATE_WAIT;
//    }
//else if (stat==MEAS_STATE_WAIT)
//{
//pl = iic_read_reg(0xC0,0x26);
//if ((pl&0x02)==0)
//    {
//    th = iic_read_reg(0xC0,0x04);
//    tl = iic_read_reg(0xC0,0x05);
//    pu = iic_read_reg(0xC0,0x01);
//    ph = iic_read_reg(0xC0,0x02);
//    pl = iic_read_reg(0xC0,0x03);
//    LED = 1;
//    pres = (((unsigned long)(pl))>>4)|(((unsigned long)(ph))<<4)|(((unsigned long)(pu))<<12);
//    pres = pres / 4;
//    pres = get_avg_pres(pres);
//    alt = calc_alt (pres,101325);
//    stat=MEAS_STATE_START;
//    }
//}
//*state= stat;
//return alt;
//}

unsigned char get_vars (unsigned char * state, float * altitude, unsigned long * pressure, char * temperature, unsigned long rpres)
{
float alt;
unsigned char stat, ndat,th,tl,pu,ph,pl;;
alt = 0;
ndat = 0;
stat = *state;
if (stat==MEAS_STATE_START)
    {
    iic_write_reg(0xC0, 0x26, 0x3A);
    stat=MEAS_STATE_WAIT;
    }
else if (stat==MEAS_STATE_WAIT)
{
pl = iic_read_reg(0xC0,0x26);
if ((pl&0x02)==0)
    {
    th = iic_read_reg(0xC0,0x04);
    tl = iic_read_reg(0xC0,0x05);
    pu = iic_read_reg(0xC0,0x01);
    ph = iic_read_reg(0xC0,0x02);
    pl = iic_read_reg(0xC0,0x03);
    pres = (((unsigned long)(pl))>>4)|(((unsigned long)(ph))<<4)|(((unsigned long)(pu))<<12);
    pres = pres / 4;
    pres = get_avg_pres(pres);
    alt = calc_alt (pres,rpres);
    stat=MEAS_STATE_START;
    *altitude = alt;
    *pressure = pres;
    *temperature = th;
    ndat = 1;
    }
}
else stat=MEAS_STATE_START;
*state= stat;
return ndat;
}


void timer_task(void)
{
if (PIR3bits.TMR4IF)
{
PIR3bits.TMR4IF=0;
timer_main = timer_main + 20;
keys_new.CHAR = 0;
if (K1==0) keys_new.k1 = 1;
if (K2==0) keys_new.k2 = 1;
if (K3==0) keys_new.k3 = 1;
keys.CHAR = keys.CHAR | ((keys_new.CHAR^keys_old.CHAR)&keys_new.CHAR);
keys_old.CHAR = keys_new.CHAR;
//LED = ~ LED;
}
}

unsigned int get_timer(void)
{
unsigned int temp;
PIE3bits.TMR4IE=0;
temp = timer_main;
PIE3bits.TMR4IE=1;
return temp;
}

unsigned long get_avg_pres (unsigned long pres)
{
unsigned char i,c,d;
unsigned long acc,swap;

pres_arr[pres_arr_pointer++] = pres;
if (pres_arr_pointer >= PRES_ARR_SIZE)
    {
    pres_arr_pointer = 0;
    }
acc = 0;
for (i=0;i<PRES_ARR_SIZE;i++)
    pres_arr_t[i] = pres_arr[i];

for (c = 0 ; c < ( PRES_ARR_SIZE - 1 ); c++)
  {
    for (d = 0 ; d < PRES_ARR_SIZE - c - 1; d++)
    {
      if (pres_arr_t[d] > pres_arr_t[d+1]) /* For decreasing order use < */
      {
        swap       = pres_arr_t[d];
        pres_arr_t[d]   = pres_arr_t[d+1];
        pres_arr_t[d+1] = swap;
      }
    }
  }
c = 0;
for (i=PRES_ARR_SIZE/3;i<((PRES_ARR_SIZE/3)*2);i++)
    {
    acc = acc + pres_arr_t[i];
    c++;
    }
acc = acc / c;
return acc;
}

unsigned long calc_pres (int alt, unsigned long pres)
{
float rpres,pressure;
pressure = pres;
rpres= pressure/(exp(-((alt*9.81)/(286*(273.15+15)))));
return ((unsigned long)(rpres));
}

float calc_alt (unsigned long pres, unsigned long rpres)
{
float p,p0,alt;
int temper;
p = pres;
p0 = rpres;
temper = 15;
alt = (-log(p/p0))*((286.0*(temper+273.15))/9.81);
return alt;
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
SSP1CON1 = 0x28;
SSP1CON2 = 0x00;
SSP1STAT = 0x00;
SSP1ADD = 10;
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

/*
 * http://omerk.github.io/lcdchargen/
 * http://www.imagesco.com/articles/lcd/06.html
 *
*/
