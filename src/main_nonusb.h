unsigned char main_loop_nonusb (unsigned char state);
void timer_task(void);
void lcd_ll_set_dl (unsigned char data);
void initLCD(void);
void lcdc (unsigned char data);
void lcdt (unsigned char data);
void set_battery_char (unsigned char level);
void clear_disp_buffer (void);


void __delay_ms(unsigned int val);
void refresh_disp(unsigned char * data);
unsigned int get_timer(void);
void iic_init (void);
void iic_start (void);
void iic_restart (void);
void iic_stop (void);
void iic_write (unsigned char data);
unsigned char iic_read (unsigned char ack);
unsigned char iic_read_reg (unsigned char hw_addr, unsigned char addr);
void iic_write_reg (unsigned char hw_addr, unsigned char addr, unsigned char data);
float calc_alt (unsigned long pres, unsigned long rpres);
unsigned long get_avg_pres (unsigned long pres);
float get_alt (unsigned char * state);
unsigned char get_vars (unsigned char * state, float * altitude, unsigned long * pressure, char * temperature, unsigned long rpres);
unsigned long calc_pres (int alt, unsigned long pres);

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
#define STATE_RPRES   6

#define SSTATE_SET_RPRES_NONE    3
#define SSTATE_SET_RPRES_ASK    0
#define SSTATE_SET_RPRES_DIR    1
#define SSTATE_SET_RPRES_ALT    2


#define MEAS_STATE_START    0
#define MEAS_STATE_WAIT     1
#define MEAS_STATE_DONE     MEAS_STATE_START

/*
RA1,2,3,5 - D4..7
RC0 - EN
RC1 - RS
RC7 - K1
RC2 - K2
RB0 - K3

*/

