/* 
 * File:   macros.h
 * Author: Administrator
 *
 * Created on August 17, 2016, 2:42 PM
 */

#ifndef MACROS_H
#define	MACROS_H

#define __delay_1s() for(char i=0;i<20;i++){__delay_ms(48);}
#define __delay_5cs() for(char i=0;i<10;i++){__delay_ms(48);}
#define __lcd_newline() lcdInst(0b11000000);    //Bottom Line
#define __lcd_clear() lcdInst(0b00000001);
#define __lcd_home() lcdInst(0b10000000);   //Top Line



#define __bcd_to_num(num) (num & 0x0F) + ((num & 0xF0)>>4)*10


//void delay_10ms(unsigned char n) { 
//         while (n-- != 0) { 
//             __delay_ms(10); 
//         } 
//}
#endif	/* MACROS_H */

