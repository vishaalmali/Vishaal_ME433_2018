#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro

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
#pragma config WDTPS = PS1048576 // use slowest wdt
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
#pragma config USERID = 0 //xFFFF // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = ON //OFF // allow multiple reconfigurations
#pragma config IOL1WAY = ON //OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module

#define SLAVE_ADDR 0b0100000

unsigned char getExpander() {
    unsigned char b=0;
    i2c_master_start(); // Begin the start sequence
    i2c_master_send(SLAVE_ADDR<<1);// send the slave address, left shifted by 1, 
                                  // which clears bit 0, indicating a write
    i2c_master_send(0x09);  // send a byte to the slave 
    i2c_master_restart();   // send a RESTART so we can begin reading
    i2c_master_send((SLAVE_ADDR<<1)|1); // send another byte to the slave
    b=i2c_master_recv(); //set b to receiving byte from slave
    i2c_master_ack(1);  // send NACK (1):  master needs no more bytes
    i2c_master_stop();  // send STOP:  end transmission, give up bus
    return b;
}

int main() {

    //  Some initialization function to set the right speed setting
    initExpander();
    __builtin_disable_interrupts(); // set the CP0 config register 
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583); // 0 data RAM 
    BMXCONbits.BMXWSDRM = 0x0;  // interrupts
    INTCONbits.MVEC = 0x1;  // disable JTAG
    DDPCONbits.JTAGEN = 0;  // TRIS and LAT
    TRISBbits.TRISB4=1; //RB4 as input
    TRISAbits.TRISA4=0; //RA4 as output
    LATAbits.LATA4=1;
    __builtin_enable_interrupts();

    while(1) {
        //set beat
        // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
            _CP0_SET_COUNT(0);
            TRISAbits.TRISA4=0;
            while(_CP0_GET_COUNT() < 1200000) {
               ; // LATAbits.LATA4=1;
            }

            _CP0_SET_COUNT(0);
            TRISAbits.TRISA4=1;
            while(_CP0_GET_COUNT() < 1200000) {
                ; // LATAbits.LATA4=0;
            }
            //read from GP7, if low, set GP0=1; if high, set GP0=0

            unsigned char v=0;
            if(getExpander()>>7==1)
                setExpander(0x0A,0);
            else
                setExpander(0x0A,1); 
    }
}

void initExpander(void) {
    //Turn off analog input
    ANSELBbits.ANSB2=0;
    ANSELBbits.ANSB3=0;
    
    i2c_master_setup();
    setExpander(0x0,0xF0);//11110000
    setExpander(0x0A,0x1);//GP0 light on
}

//
void setExpander(unsigned char pin,unsigned char level){
    i2c_master_start(); // Begin the start sequence
    i2c_master_send(SLAVE_ADDR<<1); //for write
    i2c_master_send(pin);   // send a byte to the slave
    i2c_master_send(level); // send another byte to the slave
    i2c_master_stop();  // send STOP:  end transmission, give up bus
}