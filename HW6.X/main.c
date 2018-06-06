#include <xc.h>                         // processor SFR definitions
#include <sys/attribs.h>                // __ISR macro
#include <math.h>
#include <stdio.h>
#include "ST7735.h"                     // ST7735 LCD header


// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable secondary osc
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1 // use slowest wdt
#pragma config WINDIS = OFF // wdt no window mode
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiplied by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 1 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module

void drawChar(unsigned short x, unsigned short y, unsigned short color1, unsigned short color2, unsigned char character)
{
    unsigned char row = 0;
    unsigned char col = 0;
    unsigned char index = character - 0x20;
    unsigned char pixels = 0;
    
    for (col = 0; col < 5; col++)
    {
        pixels = ASCII[index][col];
        
        for (row = 0; row < 8; row++)
        {
            if (x + col < 128)
            {
                if (pixels >> row & 1 == 1)
                {
                    LCD_drawPixel(x+col, y+row, color1);
                }
                else
                {
                    LCD_drawPixel(x+col, y+row, color2);
                }
            }
        }
    }
}

void LCD_drawString(unsigned short x, unsigned short y, unsigned char* message, unsigned short color1, unsigned short color2)
{
    unsigned short count = 0;
    unsigned short offset = 0;
    
    while(message[count])
    {
        drawChar(x+offset, y, color1, color2, message[count]);
        
        count++;
        offset = offset + 5;
    }
    
}

void LCD_progressBar(unsigned short x, unsigned short y, unsigned short h, unsigned short value)
{
    unsigned short height = 0;
    unsigned short length = 0;
    
    for(length = 0; length < 100; length++)
    {
        for(height = 0; height < h; height++)
        {
            if (length < value)
            {
                LCD_drawPixel(x+length, y+height, GREEN);
            }
            else
            {
                LCD_drawPixel(x+length, y+height, BLACK);
            }
        }
    }
}

void Calculate_FPS(unsigned short x, unsigned short y)
{
    unsigned char message[30];
    int time_elapsed = _CP0_GET_COUNT();
    float fps = (24000000.0/(float) time_elapsed);
    
    sprintf(message, "fps: %f ", fps);
    LCD_drawString(x, y, message, GREEN, BLACK);
    
}

int main() {
    
    // variable initializations
    
   
    
    unsigned char message[30];
    unsigned short percentage = 0;

    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISBbits.TRISB4 = 1;
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 1;
    
    
    
    

    __builtin_enable_interrupts();
    
    
     LCD_init();
     LCD_clearScreen(BLACK);

    while(1) {
        
            _CP0_SET_COUNT(0); // This will set the core timer to zero.
            
            sprintf(message, "Hello World: %d ", percentage);
            LCD_drawString(10, 32, message, GREEN, BLACK);
            LCD_progressBar(10, 44, 10, percentage);
            
            Calculate_FPS(10, 60);
            
         // Heartbeat code
            while(_CP0_GET_COUNT() < 2400000)
            {
                
                while(!PORTBbits.RB4)
                {
                    LATAbits.LATA4 = 0;
                } // this will wait for 1ms to pass (in this case, we want to update the values 1000/second)
                
            }
            LATAINV = 0x10;
            
            percentage++;
            
            if (percentage > 100)
            {
                percentage = 0;
            }
        }
	// use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
    
	// remember the core timer runs at half the sysclk
}