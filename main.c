/*
 * File:   main.c
 * Author: True Administrator
 *
 * Created on July 18, 2016, 12:11 PM
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

//<editor-fold defaultstate="collapsed" desc="Variable Defs">
int state;  //0: standby, 1:run, 2:Mem, 3: Complete
int count[5];       //0: Total Count, 1: AACount, 2: Ccount, 3: 9VCount, 4: DCount
int timer_count;
int durr[2];
int pos_switch;
int test_dir;
int terminate;

//</editor-fold>
const char happynewyear[7] = {  0x45, //45 Seconds 
                        0x59, //59 Minutes
                        0x17, //24 hour mode, set to 23:00
                        0x02, //Saturday 
                        0x23, //31st
                        0x01, //December
                        0x17};//2016

void interrupt Interrupthandler (void){

    if(TMR1IF){

        TMR1IF = 0;                //clear TMR1IF to enable further interrupts
        TMR1H = 0X3C;                       //re-load the Timer1 register pair (TMR1H:TMR1L)
                                                //with 15536 (3CB0 hex)
        TMR1L = 0XB0;
        timer_count += 1;
        if(timer_count == 50){
            timer_count = 0;
            durr[0] += 1;
        }
        if(durr[0] == 60){
            durr[0] = 0;
            durr[1] += 1;
        }
        if(durr[1] == 3){
            //state = termination;

        }
    }
    if(INT2IF){
        INT2IF = 0;
        
        if(test_dir == 1){
            pos_switch += 1;
        }else{
            pos_switch -= 1;
        }

        printf("S");    
    }
}

//void interrupt high_priority HIGH_Interrupthandler (void){
//    if(INTCONbits.INT0IF == 1){
//        INTCONbits.INT0IF = 0;
//        state = 100;
//        //shut off conveyor and H-bridge
//        PWM1_Stop();
//        PWM2_Stop();
//        __lcd_home();
//        printf("                ");
//        __lcd_newline();
//        printf("                ");
//
//        __lcd_home();
//        printf("EMERGENCY STOP  ");
//        __lcd_newline();
//        printf("Press Reset");
//
//        while(state == 100){
//
//            if (PORTBbits.RB1 != 0){
//            unsigned char keypress = (PORTB & 0xF0)>>4;
//            while(PORTBbits.RB1 == 1){
//
//            switch(keypress){
//                case 7: 
//                        state = 0;
//                        break;
//
//
//
//            }
//            }
//
//        }
//        }
//        __lcd_home();
//        printf("                ");
//        __lcd_newline();
//        printf("                ");
//        __lcd_home();
//        printf("Running...");
//    }
//}

void memory(){
    di();
    int run = 1;
    unsigned char address = 0x00;
    count[0] = Eeprom_ReadByte(address);
    address += 1;
    count[1] = Eeprom_ReadByte(address);
    address += 1;
    count[2] = Eeprom_ReadByte(address);
    address += 1;
    count[3] = Eeprom_ReadByte(address);
    address += 1;
    count[4] = Eeprom_ReadByte(address);
    address += 1;
    durr[0] = Eeprom_ReadByte(address);
    address += 1;
    durr[1] = Eeprom_ReadByte(address);
    
    __lcd_home();
    printf("                ");
    __lcd_newline();
    printf("                ");
    int scroll = 0;
    while(state == 2){
        
        if(scroll == 0){
            __lcd_home();
            printf("Older:4  Newer:5");
            
            __lcd_newline();
            printf("Run:%d Data   2->", run);
        }else if(scroll == 1){
            __lcd_home();
            printf("Upload Data:*   ");
            __lcd_newline();
            printf("Exit:B   <-1 2->");
        }else if(scroll == 2){
            __lcd_home();
            printf("Durr: %02d:%02d     ", durr[1], durr[0]);
            __lcd_newline();
            printf("Total:%02d", count[0]);
            printf(" <-1 2-> ");
            
        }else{
            __lcd_home();
            printf("AA:%02d C:%02d 9V:%02d", count[1], count[2], count[3]);
            __lcd_newline();
            printf("Dead:%02d", count[4]);
            printf("   <-1   ");
        }
        
        if (PORTBbits.RB1 != 0){
            unsigned char keypress = (PORTB & 0xF0)>>4;
            while(PORTBbits.RB1 == 1){
            }
            Nop();  //Apply breakpoint here because of compiler optimizations
            Nop();

            switch(keypress){
                case 0:
                    if(scroll > 0)
                        scroll -= 1;
                    break;
                case 1: 
                    if(scroll < 3)
                        scroll += 1;
                    break;
                case 7:
                    state = 0;
                    break;
                case 4:     //older run
                    if(address < 0x36){
                        run++;
                        address += 0x0A;
                        count[0] = Eeprom_ReadByte(address);
                        address += 1;
                        count[1] = Eeprom_ReadByte(address);
                        address += 1;
                        count[2] = Eeprom_ReadByte(address);
                        address += 1;
                        count[3] = Eeprom_ReadByte(address);
                        address += 1;
                        count[4] = Eeprom_ReadByte(address);
                        address += 1;
                        durr[0] = Eeprom_ReadByte(address);
                        address += 1;
                        durr[1] = Eeprom_ReadByte(address);
                    }
                    break;
                case 5:     //newer run
                    if(address > 0x06){
                        run--;
                        address -= 0x16;
                        count[0] = Eeprom_ReadByte(address);
                        address += 1;
                        count[1] = Eeprom_ReadByte(address);
                        address += 1;
                        count[2] = Eeprom_ReadByte(address);
                        address += 1;
                        count[3] = Eeprom_ReadByte(address);
                        address += 1;
                        count[4] = Eeprom_ReadByte(address);
                        address += 1;
                        durr[0] = Eeprom_ReadByte(address);
                        address += 1;
                        durr[1] = Eeprom_ReadByte(address);
                    }
                    break;
                case 12:
                    I2C_Master_Start(); //Start condition
                    I2C_Master_Write(0b00010000); //7 bit RTC address + Write
                    I2C_Master_Write(run);
                    I2C_Master_Write(count[0]);
                    I2C_Master_Write(count[1]);
                    I2C_Master_Write(count[2]);
                    I2C_Master_Write(count[3]);
                    I2C_Master_Write(count[4]);
                    I2C_Master_Write(durr[0]);
                    I2C_Master_Write(durr[1]);
                    I2C_Master_Stop();
                    break;

            }
        
        
        }
    }
    ei();
}

void set_time(void){
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(0x00); //Set memory pointer to seconds
    for(char i=0; i<7; i++){
        I2C_Master_Write(happynewyear[i]);
    }    
    I2C_Master_Stop(); //Stop condition
}

void standby(unsigned char time[]){
    int scroll = 0;
    int ymdhms = 0;    //0 nothing, 1 sec, 2 min, 3 hour, 5 day, 6 month, 7 year
    
    while(state == 0){
        // <editor-fold defaultstate="collapsed" desc="Display 1">
        if(scroll == 0){
            //Reset RTC memory pointer 
            I2C_Master_Start(); //Start condition
            I2C_Master_Write(0b11010000); //7 bit RTC address + Write
            I2C_Master_Write(0x00); //Set memory pointer to seconds
            I2C_Master_Stop(); //Stop condition

            //Read Current Time
            I2C_Master_Start();
            I2C_Master_Write(0b11010001); //7 bit RTC address + Read
            for(unsigned char i=0;i<0x06;i++){
                time[i] = I2C_Master_Read(1);
            }
            time[6] = I2C_Master_Read(0);       //Final Read without ack
            I2C_Master_Stop();
            if(ymdhms > 4 || ymdhms == 0){
                __lcd_home();
                printf("%02x/%02x/%02x        ", time[6],time[5],time[4]);    //Print date in YY/MM/DD
            }
            if(ymdhms < 4){
                __lcd_newline();
                printf("%02x:%02x:%02x", time[2],time[1],time[0]);    //HH:MM:SS
                printf("     2->");
            }

        // </editor-fold>    
        }else{
            __lcd_home();
            printf("Mem: A          ");
            __lcd_newline();
            printf("Start: C <-1    ");
        }
        
        if (PORTBbits.RB1 != 0){
        unsigned char keypress = (PORTB & 0xF0)>>4;
        while(PORTBbits.RB1 == 1){
        }
        Nop();  //Apply breakpoint here because of compiler optimizations
        Nop();
        
        switch(keypress){
            case 0:
                scroll = 0;
                break;
            case 1: 
                scroll = 1;
                break;
            case 4:
                ymdhms +=1;
                if(ymdhms == 8)
                    ymdhms = 0;
                break;
            case 5:
                if(ymdhms !=0 )
                    change_time(ymdhms);
                break;
            case 3:
                state = 2;
                break;
            case 11:
                state = 1;
                break;
                
            
        }

        print_setup(ymdhms);
        }
        __lcd_home();
        __lcd_newline();
        delay_10ms(3);
        
    }
}

void run(){
    timer_count = 0;
    for(int i = 0; i < 5; i++){
        count[i] = 0;
    }
    durr[0] = 0;        //seconds
    durr[1] = 0;        //minutes
    int type = 0;           //battery type 0 = nothing
    int angle = 0;
    terminate = 0;
    
    TMR1H = 0x3C;                   //### Set TMR1H
	TMR1L = 0xB0;                   //### Set TMR1L
    T1CONbits.TMR1ON = 1;           //enable timer 1
    
    INTCONbits.INT0IF = 0;
    INTCONbits.INT0IE = 1;     //enable emergency button interrupt RB0
    
    pos_switch = 0;             //initial position of tester arm should be 0
    test_dir = 0;               
    
    int step = 50;              //50: TEST CASE! 0: RUN MODE
    int conveyor_runs = 0;
    __lcd_home();
    printf("                ");
    __lcd_newline();
    printf("                ");
    __lcd_home();
    printf("Running...");
    while(state == 1){
        
        switch(step){
            case 0:     //check chute sensor
                readADC(3);     //RA2
                
                if(ADRESH > 0x00 || (ADRESH == 0x00 && ADRESL >0x00)){  //Place holder for sensor threshold. Replace during integration
                    step = 2;   //rotate X
                    
                }else{
                    step = 1;   // goto run conveyor cuz nothing detected
                    
                }
                break;
            case 1:     //Move conveyor.
                if(conveyor_runs == 5){     //keep track of the number of times the conveyor has run
                    state = 3;          //exit run mode if conveyor has already completed half turn
                }else{
                    PWM1_Start();       //Run Conveyor with RC2
                    set_PWM1_duty(512);
                    __delay_5cs();
                    PWM1_Stop();
                    conveyor_runs += 1;
                    step = 0;
                }
                break;
                
            case 2:     //MOVE X stepper with demuxxer
                //RC E:6, A2: 2, A1: 0, A0: RE2,    turn 90 degrees
                LATC = LATC & 0b10111110;
                LATE = LATE & 0b11111001;
                for(int j = 0; j < 2; j++){
                    for(int i = 0; i < 64; i++){
                        LATC = LATC ^ 0b01000000;
                        __delay_us(2000);
                        __delay_us(2000);
                        LATC = LATC ^ 0b01000000;
                        LATE = LATE ^ 0b00000010;
                        LATC = LATC ^ 0b01000000;
                        __delay_us(2000);
                        __delay_us(2000);
                        LATC = LATC ^ 0b01000000;
                        LATE = LATE ^ 0b00000110;
                        LATC = LATC ^ 0b01000000;
                        __delay_us(2000);
                        __delay_us(2000);
                        LATC = LATC ^ 0b01000000;
                        LATE = LATE ^ 0b00000010;
                        LATC = LATC ^ 0b01000000;
                        __delay_us(2000);
                        __delay_us(2000);
                        LATC = LATC ^ 0b01000000;
                        LATE = LATE ^ 0b00000110;
                        
                    }
                }
                
                step = 3;
                break;
                
            case 3:     //RB2 interrupt switch for DC tester arm
                INTCON3bits.INT2IF = 0;     //clear flag for RB2 on INTCON3
                INTCON3bits.INT2IE = 1;     //enable interrupts on RB2
                
                test_dir = 1;               //Test arm goes forward
                PWM2_Start();
                set_PWM2_duty(512);         //change duty cycle during integration for best speed
                                          //use H-Bridge, RC1 PWM ENABLE RE0, RA4
                LATEbits.LATE0 = 0;
                LATAbits.LATA4 = 1;
                while(pos_switch != 1){     //continue tester arm until a switch(interrupt) is flipped
                    
                }
                //turn off H-Bridge
                LATEbits.LATE0 = 0;     //break motor
                LATAbits.LATA4 = 0;
                __delay_5cs();
                PWM2_Stop();   
                
                if(pos_switch == 1){    //tester logic
                    readADC(1);     //read voltage on 4 contacts
                    if(ADRESH == 0x02 && (ADRESL < 0x1a||ADRESL < 0x0a)){       //20a to 21a
                        readADC(3);
                        if(ADRESH == 0x02 && (ADRESL < 0x1a||ADRESL < 0x0a)){       //20a to 21a
                            readADC(4);
                            if(ADRESH == 0x02 && (ADRESL < 0x1a||ADRESL < 0x0a)){       //20a to 21a
                                readADC(0);
                                if(ADRESH == 0x02 && (ADRESL < 0x1a||ADRESL < 0x0a)){       //20a to 21a
                                    count[4] += 1;  //Dead battery
                                    type = 4;
                                }else{  //AD Channel, 189 to 28e = dead
                                    if((ADRESH == 0x01 && ADRESL > 0x89)||(ADRESH == 0x02 && ADRESL < 0x8e)){
                                        count[4] += 1;  //Dead battery
                                        type = 4;
                                    }else{ 
                                        count[1] += 1;  //AA battery
                                        type = 1;
                                    }
                                }
                            }else{  //AD Channel 3
                                if((ADRESH <= 0x01 && ADRESL < 0x0c)||(ADRESH == 0x03 && ADRESL > 0x11)){   //FF to 311 = dead
                                    count[3] += 1;  //9V battery
                                    type = 3;
                                }else{ 
                                    count[4] += 1;  //Dead battery
                                    type = 4;
                                }
                            }
                            
                        }else{  //AD Channel 1
                           if((ADRESH <= 0x01 && ADRESL < 0x0c)||(ADRESH == 0x03 && ADRESL > 0x11)){   //FB to 305 = dead
                                count[3] += 1;  //9V battery
                                type = 3;
                            }else{ 
                                count[4] += 1;  //Dead battery
                                type = 4;
                            }
                        }              
                    }else{  //AD Channel 0
                            //17D to 283 = dead
                        if((ADRESH == 0x01 && ADRESL > 0x89)||(ADRESH == 0x02 && ADRESL < 0x8e)){
                            count[4] += 1;  //Dead battery
                            type = 4;
                        }else{ 
                            count[2] += 1;  //C battery
                            type = 2;
                        }
                    }
                    
                }
                step = 4;
                INTCON3bits.INT2IE = 0;     //disable interrupts on RB2 (tester arm switches)
                break;
            
            case 4: //Move ejection chute stepper with demuxxer
                
                switch(type){   //choose angle of ejection chute depending on battery type
                    case 1:
                        angle = 2;  //AA
                        break;
                    case 2:
                        angle = 2;  //C
                        break;
                    case 3:
                        angle = 2;  //9V
                        break;
                    case 4:
                        angle = 2;  //Dead
                        break;
                }   //Move Ejection Chute
                
                LATC = LATC & 0b10111110;
                LATE = LATE & 0b11111001;
                LATC = LATC ^ 0b01000001;   //fine tune best angle during integration
                for(int j = 0; j < angle; j++){
                    for(int i = 0; i < 64; i++){
                        __delay_us(2000);
                        __delay_us(2000);
                        LATE = LATE ^ 0b00000010;
                        __delay_us(2000);
                        __delay_us(2000);
                        LATC = LATC ^ 0b01000000;
                        LATE = LATE ^ 0b00000110;
                        LATC = LATC ^ 0b01000000;
                        __delay_us(2000);
                        __delay_us(2000);
                        LATE = LATE ^ 0b00000010;
                        __delay_us(2000);
                        __delay_us(2000);
                        LATC = LATC ^ 0b01000000;
                        LATE = LATE ^ 0b00000110;
                        LATC = LATC ^ 0b01000000;
                        
                    }
                }
                
                step = 5;
                break;
                
            case 5: //move DC motor to 3rd switch to eject batteries
                INTCON3bits.INT2IF = 0;     //clear flag for RB2 on INTCON3
                INTCON3bits.INT2IE = 1;     //enable interrupts on RB2
                
                test_dir = 1;
                
                PWM2_Start();
                set_PWM2_duty(512);                            //use H-Bridge, RC1PWM ENABLE RE0, RA4
                LATEbits.LATE0 = 0;                             //forward?
                LATAbits.LATA4 = 1;
                while(pos_switch != 2){
                    
                }                       //Change direction of H-bridge
                LATEbits.LATE0 = 1;
                LATAbits.LATA4 = 0;
                test_dir = 0;
                //can also use this time to move other steppers
                int j;
                while(pos_switch != 0){
                    //move ejection chute back to original position
                    //RC E:RC6, A2: RC0, A1: RE2, A0: RE1
                    LATC = LATC & 0b10111110;
                    LATE = LATE & 0b11111001;
                    LATC = LATC ^ 0b01000001;
                    LATE = LATE ^ 0b00000110;
                    for(j = 0; j < angle; j++){
                        for(int i = 0; i < 64; i++){
                            __delay_us(2000);
                            LATE = LATE ^ 0b0000010;
                            __delay_us(2000);
                            LATC = LATC ^ 0b01000000;
                            LATE = LATE ^ 0b00000110;
                            LATC = LATC ^ 0b01000000;
                            __delay_us(2000);
                            LATC = LATC ^ 0b01000000;
                            LATE = LATE ^ 0b00000010;
                            LATC = LATC ^ 0b01000000;
                            __delay_us(2000);
                            LATC = LATC ^ 0b01000000;
                            LATE = LATE ^ 0b00000110;
                            LATC = LATC ^ 0b01000000;
                            if(pos_switch == 0){
                                LATEbits.LATE0 = 0;     //break motor
                                LATAbits.LATA4 = 0;
                            }
                        }
                    }
                }
                step = 0;
                if(angle < 2){              //change smallest angle during integration
                    LATEbits.LATE0 = 0;     //break motor if angle is too small
                    LATAbits.LATA4 = 0;
                    __delay_us(2000);
                }
                
                PWM2_Stop(); 
                INTCON3bits.INT2IE = 0;     //disable interrupts on RB2
                break;
                
            case 50:    //testing
                //RC E:RC6, A2: RC0, A1: RE2, A0: RE1

                LATC = LATC & 0b10111110;
                LATE = LATE & 0b11111001;
                for(int j = 0; j < 2; j++){
                    for(int i = 0; i < 64; i++){
                        LATC = LATC ^ 0b01000000;
                        __delay_ms(2);
                        __delay_ms(2);

                        LATC = LATC ^ 0b01000000;
                        LATE = LATE ^ 0b00000010;
                        LATC = LATC ^ 0b01000000;
                        __delay_ms(2);
                        __delay_ms(2);

                       LATC = LATC ^ 0b01000000;
                       LATE = LATE ^ 0b00000110;
                        LATC = LATC ^ 0b01000000;
                        __delay_ms(2);
                        __delay_ms(2);

                        LATC = LATC ^ 0b01000000;
                        LATE = LATE ^ 0b00000010;
                        LATC = LATC ^ 0b01000000;
                        __delay_ms(2);
                        __delay_ms(2);

                        LATC = LATC ^ 0b01000000;
                        LATE = LATE ^ 0b00000110;
                        
                    }
                }
                
                
                    //PWM1_Start();       //Run Conveyor with RC2
                    //set_PWM1_duty(1023);
                    //LATCbits.LATC2 = 1;
                    //__delay_5cs();
                    //__delay_5cs();
                    //__delay_5cs();
                    //__delay_5cs();
                    //__delay_5cs();
                   //LATCbits.LATC2 = 0;
                    //PWM1_Stop();
                INT2IF = 0;     //clear flag for RB2 on INTCON3
                INT2IE = 1;     //enable interrupts on RB2
                
                test_dir = 1;
                ADCON1 = 0xFF;
                di();
                PWM2_Start();         //arm test
                set_PWM2_duty(1023);                            //use H-Bridge, RC1PWM ENABLE RE0, RA4
                
                LATEbits.LATE0 = 0;
                LATAbits.LATA4 = 1;
                __delay_ms(8);
                __delay_ms(8);
                __delay_ms(8);
                ei();
                __lcd_newline();
                while(pos_switch != 2){
 
                }
                ADCON1 = 0x0A;
                LATEbits.LATE0 = 0;
                LATAbits.LATA4 = 0;
                PWM2_Stop();         
                
                LATC = LATC & 0b10111110;
                LATE = LATE & 0b11111001;
                count[1] += 1;
                count[2] += 2;
                count[3] += 1;
                count[4] += 2;

                LATC = LATC ^ 0b01000001;
                for(int j = 0; j < 2; j++){
                    for(int i = 0; i < 64; i++){
                        __delay_ms(7);
                        LATE = LATE ^ 0b00000010;
                        __delay_ms(7);
                        LATC = LATC ^ 0b01000000;
                        LATE = LATE ^ 0b00000110;
                        LATC = LATC ^ 0b01000000;
                        __delay_ms(7);
                        LATE = LATE ^ 0b00000010;
                        __delay_ms(7);
                        LATC = LATC ^ 0b01000000;
                        LATE = LATE ^ 0b00000110;
                        LATC = LATC ^ 0b01000000;
                        
                    }
                }
                LATC = LATC & 0b10111110;
                LATE = LATE & 0b11111001;
                
                LATC = LATC ^ 0b01000001;
                LATE = LATE ^ 0b00000110;
                for(int j = 0; j < 2; j++){
                    for(int i = 0; i < 64; i++){
                        __delay_ms(2);
                        __delay_ms(2);
                        LATE = LATE ^ 0b0000010;
                        __delay_ms(2);
                        __delay_ms(2);
                        LATC = LATC ^ 0b01000000;
                        LATE = LATE ^ 0b00000110;
                        LATC = LATC ^ 0b01000000;
                        __delay_ms(2);
                        __delay_ms(2);
                        LATC = LATC ^ 0b01000000;
                        LATE = LATE ^ 0b00000010;
                        LATC = LATC ^ 0b01000000;
                        __delay_ms(2);
                        __delay_ms(2);
                        LATC = LATC ^ 0b01000000;
                        LATE = LATE ^ 0b00000110;
                        LATC = LATC ^ 0b01000000;
                    }
                }
                state = 3;
                
                break;
                

        }
        
        
    }
    count[0] = count[1] + count[2] + count[3] + count[4];   //get total count
    
    T1CONbits.TMR1ON = 0;       //disable timer because we done.
    TMR1H = 0X3C;
	TMR1L = 0XB0;
    unsigned char address = 0x36;
    int temp;
    for(int j = 0; j < 3; j++){     //Move data stored in EEPROM
        
        for(int i = 0; i < 7; i++){
            
            temp = Eeprom_ReadByte(address - 0x10);
            Eeprom_WriteByte(address, temp);
            address -= 0x01;
        }
        address -= 0x09;
    }
    Eeprom_WriteByte(address, durr[1]);     //add new data to EEPROM
    address -= 0x01;
    Eeprom_WriteByte(address, durr[0]);
    address -= 0x01;
    Eeprom_WriteByte(address, count[4]);
    address -= 0x01;
    Eeprom_WriteByte(address, count[3]);
    address -= 0x01;
    Eeprom_WriteByte(address, count[2]);
    address -= 0x01;
    Eeprom_WriteByte(address, count[1]);
    address -= 0x01;
    Eeprom_WriteByte(address, count[0]);
    INTCONbits.INT0IE = 0;     //disable emergency button interrupt RB0
}

void completion(){
    __lcd_home();
    printf("                ");
    __lcd_newline();
    printf("                ");
    int scroll = 0;
    while(state == 3){
        if(scroll == 0){
            __lcd_home();
            if(terminate == 0)
                printf("Run Complete    ");
            else
                printf("Runtime Error %d", terminate);
            __lcd_newline();
            printf("Exit: B      2->");
        }else if(scroll == 1){
            __lcd_home();
            printf("Durr: %02d:%02d     ", durr[1], durr[0]);
            __lcd_newline();
            printf("Total:%02d", count[0]);
            printf(" <-1 2->");
            
        }else{
            __lcd_home();
            printf("AA:%02d C:%02d 9V:%02d", count[1], count[2], count[3]);
            __lcd_newline();
            printf("Dead:%02d", count[4]);
            printf("   <-1   ");
        }
        
        if (PORTBbits.RB1 != 0){
            unsigned char keypress = (PORTB & 0xF0)>>4;
            while(PORTBbits.RB1 == 1){
            }
            Nop();  //Apply breakpoint here because of compiler optimizations
            Nop();

            switch(keypress){
                case 0:
                    if(scroll > 0)
                        scroll -= 1;
                    break;
                case 1: 
                    if(scroll < 2)
                        scroll += 1;
                    break;
                case 7:
                    state = 0;
                    break;



            }
        
        
        }
    }
    
}

void main(void) {
    // <editor-fold defaultstate="collapsed" desc="Initialize PIC">
    OSCCON = 0xF0;  //8Mhz
    
    // RTC uses RC3 and RC4
    // LCD uses RD2 - RD7
    // KEYPAD uses RB4 - RB7 and RB1
    
    TRISA = 0b00101111; // Set Port A analog inputs
    TRISB = 0b11110111; // Set Keypad Pins as input, interrupt inputs
    TRISC = 0x00011000; // Set I2C pins as input, rest are output
    TRISD = 0x00;   //All output mode
    TRISE = 0x00;   //RE2, RE0 and RE1 output

    LATA = 0x00;
    LATB = 0x00; 
    LATC = 0x00;
    LATD = 0x00;
    LATE = 0x00;
    
    nRBPU = 0;
    initLCD(); // Initialize LCD
    I2C_Master_Init(10000); //Initialize I2C Master with 100KHz clock
//    
    set_PWM_freq (2000);
//
    di(); // Disable all interrupts
    unsigned char time[7];
    set_time();
    ei();
   
    state = 0;
    TIMER1_initializer();
    Interrupt_initializer();
    
    ADCON0 = 0x00;  //Disable ADC
    ADCON1 = 0x0A;  //AN0 to AN3 used as analog input
    CVRCON = 0x00; // Disable CCP reference voltage output
    CMCONbits.CIS = 0;
    ADFM = 1;

    // </editor-fold>
    while(1){
        switch(state){
            case 0:
                standby(time);
                break;
            case 1:
                run();
                break;
            case 2:
                memory();
                break;
            case 3:
                completion();
                break;
            
        }
        
       
        
        
    }
    
    // This statement is here incase the main loop breaks, the return
    //statement will initiate a soft reset
    return; 
}

    