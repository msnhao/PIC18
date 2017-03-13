/*
 * File:   timer1.c
 * Author: 1
 *
 * Created on February 5, 2017, 10:51 PM
 */


#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "lcd.h"
#include "I2C.h"
#include "macros.h"
#include "eeprom_routines.h"
#include "timer1.h"

long freq; // Selected PWM Frequency
unsigned int i = 0; // Keep track of current PWM duty cycle

void TIMER1_initializer(void)
{
    
	T1CONbits.TMR1ON = 0;           //make sure Timer1 is off
	
	T1CONbits.TMR1CS = 0;           //### Set clock source to Fosc/4 (TMR1CS)
	T1CKPS0 = 0;                    //### set Timer1 Pre-scale to 1 using
	T1CKPS1 = 0;                    //### T1CKPS0 and T1CKPS1
    

}

void Interrupt_initializer(void)
{
	PIR1bits.TMR1IF = 0;                 //clear TMR1IF
	PIE1bits.TMR1IE = 1;                 //### Enable Timer 1 Interrupts
    
    PIE1bits.CCP1IE = 0;                 //### Enable Timer 1 Interrupts
    PIE2bits.CCP2IE = 0;                 //### Enable Timer 1 Interrupts
    
    IPR1bits.TMR1IP = 0;
    
	INTCONbits.PEIE = 1;                 //### Enable Peripheral Interrupts
	INTCONbits.GIE = 1;                  //### Enable Global Interrupts
    
//    RCONbits.IPEN = 1;
//    INTCONbits.GIEL = 1;
//    INTCONbits.GIEH = 1;
    
    INTCON2bits.INTEDG0 = 1;    //Interrupt on rising edge RB0
                                //RB0 always high priority
    INTCON2bits.INTEDG2 = 1;    //Interrupt on rising edge RB2
//    INTCON3bits.INT2IP = 0;     //low priority
}

//! @brief      Reads a single byte of data from the EEPROM.
//! @param      address     The EEPROM address to write the data to (note that not all
//!                         16-bits of this variable may be supported).
//! @returns    The byte of data read from EEPROM.
//! @warning    This function does not return until read operation is complete.
unsigned char Eeprom_ReadByte(unsigned char address)
{

    // Set address registers
    EEADRH = (unsigned char)(address >> 8);
    EEADR = (unsigned char)address;

    EECON1bits.EEPGD = 0;       // Select EEPROM Data Memory
    EECON1bits.CFGS = 0;        // Access flash/EEPROM NOT config. registers
    EECON1bits.RD = 1;          // Start a read cycle

    // A read should only take one cycle, and then the hardware will clear
    // the RD bit
    while(EECON1bits.RD == 1);

    return EEDATA;              // Return data

}

//! @brief      Writes a single byte of data to the EEPROM.
//! @param      address     The EEPROM address to write the data to (note that not all
//!                         16-bits of this variable may be supported).
//! @param      data        The data to write to EEPROM.
//! @warning    This function does not return until write operation is complete.
void Eeprom_WriteByte(unsigned char address, unsigned char data)
{    
    // Set address registers
    EEADRH = (unsigned char)(address >> 8);
    EEADR = (unsigned char)address;

    EEDATA = data;          // Write data we want to write to SFR
    EECON1bits.EEPGD = 0;   // Select EEPROM data memory
    EECON1bits.CFGS = 0;    // Access flash/EEPROM NOT config. registers
    EECON1bits.WREN = 1;    // Enable writing of EEPROM (this is disabled again after the write completes)

    // The next three lines of code perform the required operations to
    // initiate a EEPROM write
    EECON2 = 0x55;          // Part of required sequence for write to internal EEPROM
    EECON2 = 0xAA;          // Part of required sequence for write to internal EEPROM
    EECON1bits.WR = 1;      // Part of required sequence for write to internal EEPROM

    // Loop until write operation is complete
    while(PIR2bits.EEIF == 0)
    {
        continue;   // Do nothing, are just waiting
    }

    PIR2bits.EEIF = 0;      //Clearing EEIF bit (this MUST be cleared in software after each write)
    EECON1bits.WREN = 0;    // Disable write (for safety, it is re-enabled next time a EEPROM write is performed)
}


void change_time(int ymdhms){
    unsigned char curr_time;
    int j;
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(ymdhms - 1); //Set memory pointer to seconds
    I2C_Master_Stop(); //Stop condition
    
    I2C_Master_Start();
    I2C_Master_Write(0b11010001); //7 bit RTC address + Read

    curr_time = I2C_Master_Read(0);
    I2C_Master_Stop();
    if(curr_time % 0x10 == 0x09){
        curr_time += 0x10;
        curr_time -= 0x09;
    }else{
        curr_time += 0x01;
    }
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(ymdhms - 1);
    I2C_Master_Write(curr_time);
    I2C_Master_Stop(); //Stop condition
}

void print_setup(int ymdhms){
    if(ymdhms < 4 ){
        __lcd_home();
        printf("        ");
        __lcd_home();
        
    }else{
        __lcd_newline();
        printf("        ");
        __lcd_newline();
    }
    
    switch(ymdhms){
        
        case 7:
            printf("Year");
            break;
        case 6:
            printf("Month");
            break;
        case 5:
            printf("Day");
            break;
        case 3:
            printf("Hour");
            break;
        case 2:
            printf("Min");
            break;
        case 1:
            printf("Sec");
            break;
        default:
            break;
        
    }
    
}

void readADC(char channel){
    // Select A2D channel to read
    ADCON0 = ((channel <<2));
    ADON = 1;
    ADCON0bits.GO = 1;
   while(ADCON0bits.GO_NOT_DONE){__delay_ms(5);}
    
    
}

// Returns the register values to be set for 100% duty cycle, this is dependent
// on the PWM frequency and oscillator frequency
// https://electrosome.com/pwm-pic-microcontroller-mplab-xc8/
int PWM_Max_Duty(){
  return(_XTAL_FREQ/(freq*TMR2PRESCALE);
}


// Sets PR2 register to match the frequency desired
// See datasheet pg 149, equation 16-1
// https://electrosome.com/pwm-pic-microcontroller-mplab-xc8/
void set_PWM_freq(long fre){
  PR2 = (_XTAL_FREQ/(fre*4*TMR2PRESCALE)) - 1;
  freq = fre;
}

// Sets the duty cycle of PWM1, from 1024 to 0
// See datasheet pg 150, equation 16-1
// https://electrosome.com/pwm-pic-microcontroller-mplab-xc8/
void set_PWM1_duty(unsigned int duty){
  if(duty<1024)
  {
    duty = ((float)duty/1023)*PWM_Max_Duty();
    CCP1X = duty & 2; // Set the 2 lest significant bit in CCP1CON register
    CCP1Y = duty & 1;
    CCPR1L = duty>>2; // Set rest of the duty cycle bits in CCPR1L
  }
    
}

// Sets the duty cycle of PWM2, from 1024 to 0
// See datasheet pg 150, equation 16-1
// https://electrosome.com/pwm-pic-microcontroller-mplab-xc8/
void set_PWM2_duty(unsigned int duty){
  if(duty<1024)
  {
    duty = ((float)duty/1023)*PWM_Max_Duty();
    CCP2X = duty & 2; // Set the 2 lest significant bit in CCP2CON register
    CCP2Y = duty & 1;
    CCPR2L = duty>>2; // Set rest of the duty cycle bits in CCPR2L
  }
}

PWM1_Start(){
  //Configure CCP1CON, single output mode, all active high
  P1M1 = 0;
  P1M0 = 0;
  CCP1M3 = 1;
  CCP1M2 = 1;
  CCP1M1 = 0;
  CCP1M0 = 0;
  
  //Configure prescale values for Timer2, according to TMR2PRESCALAR
  #if TMR2PRESCALAR == 1
    T2CKPS0 = 0;
    T2CKPS1 = 0;
  #elif TMR2PRESCALAR == 4
    T2CKPS0 = 1;
    T2CKPS1 = 0;
  #elif TMR2PRESCALAR == 16
    T2CKPS0 = 1;
    T2CKPS1 = 1;
  #endif

  // Enable timer 2
  TMR2ON = 1;
  
  // Enable PWM output pins
  TRISCbits.TRISC2 = 0;
}

PWM2_Start(){
  //Configure CCP2CON, enter PWM mode
  CCP2M3 = 1;
  CCP2M2 = 1;
  CCP2M1 = 0;
  CCP2M0 = 0;
  
  //Configure prescale values for Timer2, according to TMR2PRESCALAR
  #if TMR2PRESCALAR == 1
    T2CKPS0 = 0;
    T2CKPS1 = 0;
  #elif TMR2PRESCALAR == 4
    T2CKPS0 = 1;
    T2CKPS1 = 0;
  #elif TMR2PRESCALAR == 16
    T2CKPS0 = 1;
    T2CKPS1 = 1;
  #endif

  // Enable timer 2
  TMR2ON = 1;
  
  // Enable PWM output pins
  TRISCbits.TRISC1 = 0;
}

PWM1_Stop(){
  CCP1M3 = 0;
  CCP1M2 = 0;
  CCP1M1 = 0;
  CCP1M0 = 0;
}

PWM2_Stop(){
  CCP2M3 = 0;
  CCP2M2 = 0;
  CCP2M1 = 0;
  CCP2M0 = 0;
}