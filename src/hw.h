void lcd_ll_set_dl (unsigned char data);
void initLCD(void);
void lcdc (unsigned char data);
void lcdt (unsigned char data);
void set_battery_char (unsigned char level);
void clear_disp_buffer (void);
void iic_mem_write (unsigned int hw_addr, unsigned long addr, unsigned char data);
unsigned char iic_mem_read (unsigned int hw_addr, unsigned long addr);
unsigned int read_mem_location (unsigned int num, unsigned char bank);
void write_mem_location(unsigned int data, unsigned int num, unsigned char bank);

void __delay_ms(unsigned int val);
void refresh_disp(unsigned char * data);
void iic_init (void);
void iic_start (void);
void iic_restart (void);
void iic_stop (void);
void iic_write (unsigned char data);
unsigned char iic_read (unsigned char ack);
unsigned char iic_read_reg (unsigned char hw_addr, unsigned char addr);
void iic_write_reg (unsigned char hw_addr, unsigned char addr, unsigned char data);
unsigned int get_adc (unsigned char chnl);
unsigned int get_batt_volts (void);
unsigned char get_batt_level (unsigned int voltage);

void init_hw_fast (void);
void init_hw_slow (void);

