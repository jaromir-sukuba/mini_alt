#include <p18cxxx.h>
#include <delays.h>
#include <stdio.h>
#include <math.h>
#include "main_tasks.h"
#include "hw.h"


unsigned char tbuff[20];
unsigned char lcd_buff[25];
unsigned long pres_arr[PRES_ARR_SIZE];
unsigned long pres_arr_t[PRES_ARR_SIZE];
unsigned char pres_arr_pointer;
unsigned int timer_main, time,timer_log,timer_usb;

unsigned char main_state, main_substate, meas_state, ndata,batt_level, memory_bank, logging;
unsigned long pres,rpres;
float alt,alt_temp;
unsigned int alti,alti_r,memory_pointer,batt_volt;
int ralt;
char temper;

volatile key_var keys, keys_old, keys_new;


void main_init (void)
{
init_hw_slow();
initLCD();
main_state  = STATE_PRESS;
pres = 101300;
batt_level = 3;
ralt = 250;
logging = 0;
memory_pointer = 0;
pres = 0;
time = get_timer();
timer_log = time;
memory_bank = 0;
}

unsigned char main_loop_nonusb (unsigned char state)
{
    refresh_disp(lcd_buff);
    __delay_ms(50);
    time = get_timer();
    batt_volt = get_batt_volts();
    batt_level = get_batt_level(batt_volt);
    set_battery_char(batt_level);
    ndata = get_vars(&meas_state,&alt,&pres,&temper,rpres);
    if (ndata!=0) LED = ~ LED;

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
            keys.CHAR = 0;
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
            keys.CHAR = 0;
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
            keys.CHAR = 0;
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
                main_substate=SSTATE_SET_MEM_NONE;
                main_state = STATE_MEM;
                keys.CHAR = 0;
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
                keys.CHAR = 0;
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
        if (main_substate==SSTATE_SET_MEM_NONE)
            {
            clear_disp_buffer();
            if (logging==0 ) sprintf (lcd_buff,"MEM   !~ ");
            else sprintf (lcd_buff,"MEML  !~ ");
            sprintf (lcd_buff+8,"%d/%d",memory_bank,memory_pointer);
            if (keys.k3)
                {
                keys.k3 = 0;
                main_state = STATE_OFF;
                }
            if (keys.k2)
                {
                keys.k2 = 0;
                main_substate=SSTATE_SET_MEM_ASK;
                keys.CHAR = 0;
                }
            }
        if (main_substate==SSTATE_SET_MEM_ASK)
            {
            clear_disp_buffer();
            sprintf (lcd_buff,"Select ~ ");
            sprintf (lcd_buff+8,"BNK  LOG");
            if (keys.k1)
                {
                keys.k1 = 0;
                main_substate=SSTATE_SET_MEM_BANK;
                keys.CHAR = 0;
                }
            if (keys.k3)
                {
                keys.k3 = 0;
                main_substate=SSTATE_SET_MEM_FIN;
                keys.CHAR = 0;
                }
            }
        if (main_substate==SSTATE_SET_MEM_BANK)
            {
            clear_disp_buffer();
            sprintf (lcd_buff,"Memory ~ ");
            sprintf (lcd_buff+8,"Bank %d",memory_bank);
            if (keys.k3)
                {
                keys.k3 = 0;
                memory_bank++;
                if (memory_bank>=4) memory_bank=0;
                }
            if (keys.k2)
                {
                keys.k2 = 0;
                main_substate=SSTATE_SET_MEM_NONE;
                }
            if (keys.k1)
                {
                if (memory_bank<1) memory_bank=4;
                memory_bank--;
                keys.k1 = 0;
                }
            }
        if (main_substate==SSTATE_SET_MEM_FIN)
            {
            clear_disp_buffer();
            if (logging==0) sprintf (lcd_buff,"Start? ~ ");
            else sprintf(lcd_buff,"Stop?  ~ ");
            sprintf (lcd_buff+8,"NO   YES");
            if (keys.k1)
                {
                keys.k1 = 0;
                main_substate=SSTATE_SET_MEM_NONE;
                }
            if (keys.k2)
                {
                keys.k2 = 0;
                }
            if (keys.k3)
                {
                keys.k3 = 0;
                main_substate=SSTATE_SET_MEM_NONE;
                memory_pointer = 0;
                if (logging==0) logging = 1;
                else logging = 0;
                }
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



return state;
}

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

void main_loop_usb_init (void)
{
timer_usb = get_timer();
init_hw_fast ();
}

unsigned char main_loop_usb(unsigned char state)
{
unsigned int adc;
time = get_timer();
if (time>=(timer_usb + 1000))
    {
    LED = ~ LED;
    adc = get_adc(ADC_CHNL_CHMON);
    timer_usb = time;
    sprintf (lcd_buff,"USB MODE");
    sprintf (lcd_buff+8,"%d  ",adc);
    refresh_disp(lcd_buff);
    }
return state;
}

