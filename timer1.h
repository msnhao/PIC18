void TIMER1_initializer(void);
void Interrupt_initializer(void);

void Eeprom_WriteByte(unsigned char address, unsigned char data);
unsigned char Eeprom_ReadByte(unsigned char address);
//newest
//0x00: Tcount 1
//0x01: AAcount 1
//0x02: Ccount 1
//0x03: 9VCount 1
//0x04: Dcount 1    drained
//0x05: durr[0] 1
//0x06: durr[1] 1
//0x10: Tcount 2
//0x11: AAcount 2
//0x12: Ccount 2
//0x13: 9VCount 2
//0x14: Dcount 2    drained
//0x15: durr[0] 2
//0x16: durr[1] 2
//0x20: Tcount 3
//0x21: AAcount 3
//0x22: Ccount 3
//0x23: 9VCount 3
//0x24: Dcount 3    drained
//0x25: durr[0] 3
//0x26: durr[1] 3
//0x30: Tcount 4
//0x31: AAcount 4
//0x32: Ccount 4
//0x33: 9VCount 4
//0x34: Dcount 4    drained
//0x35: durr[0] 4
//0x36: durr[1] 4
//oldest


void change_time(int ymdhms);
void print_setup(int ymdhms);

void readADC(char channel);

int PWM_Max_Duty();
void set_PWM_freq(long fre);
void set_PWM1_duty(unsigned int duty);
void set_PWM2_duty(unsigned int duty);
PWM1_Start();
PWM2_Start();
PWM1_Stop();
PWM2_Stop();