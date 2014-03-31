unsigned char main_loop_nonusb (unsigned char state);
void main_init (void);
void timer_task(void);
unsigned int get_timer(void);
float calc_alt (unsigned long pres, unsigned long rpres);
unsigned long get_avg_pres (unsigned long pres);
float get_alt (unsigned char * state);
unsigned char get_vars (unsigned char * state, float * altitude, unsigned long * pressure, char * temperature, unsigned long rpres);
unsigned long calc_pres (int alt, unsigned long pres);
unsigned char main_loop_usb(unsigned char state);
void main_loop_usb_init (void);


typedef union
{
struct
 {
    unsigned k1:1;
    unsigned k2:1;
    unsigned k3:1;
    unsigned k4:1;
    unsigned ok:1;
    unsigned z4:1;
    unsigned z5:1;
 	unsigned z6:1;
 };
unsigned char CHAR;
}key_var;

#define STATE_START 0
#define STATE_PRESS 1
#define STATE_ALT   2
#define STATE_TEMP  3
#define STATE_OFF   4
#define STATE_MEM   5
#define STATE_RPRES 6

#define SSTATE_SET_RPRES_NONE   3
#define SSTATE_SET_RPRES_ASK    0
#define SSTATE_SET_RPRES_DIR    1
#define SSTATE_SET_RPRES_ALT    2

#define SSTATE_SET_MEM_NONE     0
#define SSTATE_SET_MEM_BANK     1
#define SSTATE_SET_MEM_START    2
#define SSTATE_SET_MEM_FIN      3
#define SSTATE_SET_MEM_ASK      4


#define MEAS_STATE_START    0
#define MEAS_STATE_WAIT     1
#define MEAS_STATE_DONE     MEAS_STATE_START

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

#define ADC_CHNL_BMON   0
#define ADC_CHNL_CHMON  10


/*
RA1,2,3,5 - D4..7
RC0 - EN
RC1 - RS
RC7 - K1
RC2 - K2
RB0 - K3

*/

