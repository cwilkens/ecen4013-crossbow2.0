/*
 *
 * Hey, did you hear about how Colt Wilkens wrote this one too?
 *
 */

#include <p24HJ64GP502.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "captouch.h"

//#include <i2c.h>

// I2C stuff
// FCY = 3.685 MHz
// FSCL = 100 kHz
// I2CBRG = (FCY/FSCL - FCY/10,000,000) - 1 = 35.4815
//I2C1BRG = 35;
#define I2C_BRG 35
#define lenArray(arr)  (sizeof(arr)/sizeof(arr[0]))

char touchdata[TOUCHSIZE+1];
unsigned char slaveaddr;

#ifdef DEBUG
void uart_binary(char value)
{
    int i;
    char temp = value;
    for(i = 0; i < 8; i++) {
        if (temp & 0x80)
            uart_chrstr("1");
        else
            uart_chrstr("0");
        temp = temp << 1;
    }
}

void uart_binary_16(int value)
{
    int i;
    int temp = value;
    for(i = 0; i < 16; i++) {
        if ( (temp >> (15-i)) & 0x0001)
            uart_chrstr("1");
        else
            uart_chrstr("0");
        //temp = temp << 1;
    }
}
#endif

//loop nops for delay
void DelayuSec(unsigned int N)
{
   unsigned int j;
   while(N > 0)
   {
      for(j=0;j < 10; j++);
      N--;
   }
}

/*
 *   i2c_* functions taken from:
 *   http://www.engscope.com/pic24-tutorial/10-2-i2c-basic-functions/
 *
 */

//Resets the I2C bus to Idle
void reset_i2c_bus(void)
{
   int x = 0;

   //initiate stop bit
   I2C1CONbits.PEN = 1;

   //wait for hardware clear of stop bit
   while (I2C1CONbits.PEN)
   {
      DelayuSec(1);
      x ++;
      if (x > 20) break;
   }
   I2C1CONbits.RCEN = 0;
   IFS1bits.MI2C1IF = 0; // Clear Interrupt
   I2C1STATbits.IWCOL = 0;
   I2C1STAT = I2C1STAT & 0xFBFF;
   //I2C1STATbits.BCL = 0; // see above (and silicon errata)
   DelayuSec(10);
}

//function initiates I2C1 module to baud rate BRG
void i2c_init()
{
   int temp;

   I2C1BRG = I2C_BRG;
   I2C1CONbits.I2CEN = 0;	// Disable I2C Mode
   I2C1CONbits.DISSLW = 1;	// Disable slew rate control
   IFS1bits.MI2C1IF = 0;	 // Clear Interrupt
   I2C1CONbits.I2CEN = 1;	// Enable I2C Mode
   temp = I2CRCV;	 // read buffer to clear buffer full
   reset_i2c_bus();	 // set bus to idle
}

//function iniates a start condition on bus
void i2c_start(void)
{
   int x = 0;
   I2C1CONbits.ACKDT = 0;	//Reset any previous Ack
   DelayuSec(10);
   I2C1CONbits.SEN = 1;	//Initiate Start condition
   Nop();

   //the hardware will automatically clear Start Bit
   //wait for automatic clear before proceding
   while (I2C1CONbits.SEN)
   {
      DelayuSec(1);
      x++;
      if (x > 20)
      break;
   }
   DelayuSec(2);
}

void i2c_restart(void)
{
   int x = 0;

   I2C1CONbits.RSEN = 1;	//Initiate restart condition
   Nop();

   //the hardware will automatically clear restart bit
   //wait for automatic clear before proceding
   while (I2C1CONbits.RSEN)
   {
      DelayuSec(1);
      x++;
      if (x > 20)	break;
   }
   DelayuSec(2);
}

//basic I2C byte send
char send_i2c_byte(int data)
{
    int i;

    while (I2C1STATbits.TBF) { }
    IFS1bits.MI2C1IF = 0; // Clear Interrupt
    I2CTRN = data; // load the outgoing data byte

    // wait for transmission
    for (i=0; i<500; i++)
    {
        if (!I2C1STATbits.TRSTAT) break;
        DelayuSec(1);
    }
    if (i == 500) {
#ifdef DEBUG
        uart_chrstr("failed to send?\r\n");
#endif
        return(1);
    }

    // Check for NO_ACK from slave, abort if not found
    if (I2C1STATbits.ACKSTAT == 1)
    {
#ifdef DEBUG
        uart_chrstr("no ack\r\n");
#endif
        reset_i2c_bus();
        return(1);
    }
    DelayuSec(2);
    return(0);
}

//function reads data, returns the read data, no ack
char i2c_read(void)
{
    int i = 0;
    char data = 0;

    //set I2C module to receive
    I2C1CONbits.RCEN = 1;

    //if no response, break
    while (!I2C1STATbits.RBF)
    {
        i ++;
        if (i > 2000) {
#ifdef DEBUG
            uart_chrstr("Dread:");
            uart_binary_16(I2C1STAT);
            uart_newline();
#endif
            break;
        }
    }
    //get data from I2CRCV register
    data = I2CRCV;

    //return data
    return data;
}

//function reads data, returns the read data, with ack
char i2c_read_ack(void)	//does not reset bus!!!
{
    int i = 0;
    char data = 0;

    //set I2C module to receive
    I2C1CONbits.RCEN = 1;

    //if no response, break
    while (!I2C1STATbits.RBF)
    {
        i++;
        if (i > 2000) {
#ifdef DEBUG
            uart_chrstr("\r\n^delayed ack read.^\r\n");
#endif
            break;
        }
    }
    //get data from I2CRCV register
    data = I2CRCV;

    //set ACK to high
    I2C1CONbits.ACKEN = 1;

    //wait before exiting
    DelayuSec(10);

    //return data
    return data;
}

void I2Cwrite(char addr, char subaddr, char value)
{
    i2c_start();
    send_i2c_byte(addr);
    send_i2c_byte(subaddr);
    send_i2c_byte(value);
    reset_i2c_bus();
}

char I2Cread(char addr, char subaddr)
{
    char temp;

    i2c_start();
    send_i2c_byte(addr);
    send_i2c_byte(subaddr);
    DelayuSec(10);

    i2c_restart();
    send_i2c_byte(addr | 0x01);
    temp = i2c_read();

    reset_i2c_bus();
    return temp;
}

void i2c_wait(unsigned int count)
{
    while (count--)
    {
        Nop();
        Nop();
    }
}


// touch logic
void touch_init()
{
    i2c_init();

    unsigned int i; //??
    for(i = 0; i < 4160; i++)
    {
        Nop();  // DONT TAKE THIS OUT OR IT FREAKS OUT
    }
    slaveaddr = 0x5A << 1;

    // Section A
    // This group controls filtering when data is > baseline.
    I2Cwrite(slaveaddr, MHD_R, 0x01);
    I2Cwrite(slaveaddr, NHD_R, 0x01);
    I2Cwrite(slaveaddr, NCL_R, 0x00);
    I2Cwrite(slaveaddr, FDL_R, 0x00);

    // Section B
    // This group controls filtering when data is < baseline.
    I2Cwrite(slaveaddr, MHD_F, 0x01);
    I2Cwrite(slaveaddr, NHD_F, 0x01);
    I2Cwrite(slaveaddr, NCL_F, 0xFF);
    I2Cwrite(slaveaddr, FDL_F, 0x02);

    // Section C
    // This group sets touch and release thresholds for each electrode
    I2Cwrite(slaveaddr, ELE0_T, TOU_THRESH);
    I2Cwrite(slaveaddr, ELE0_R, REL_THRESH);
    I2Cwrite(slaveaddr, ELE1_T, TOU_THRESH);
    I2Cwrite(slaveaddr, ELE1_R, REL_THRESH);
    I2Cwrite(slaveaddr, ELE2_T, TOU_THRESH);
    I2Cwrite(slaveaddr, ELE2_R, REL_THRESH);
    I2Cwrite(slaveaddr, ELE3_T, TOU_THRESH);
    I2Cwrite(slaveaddr, ELE3_R, REL_THRESH);
    I2Cwrite(slaveaddr, ELE4_T, TOU_THRESH);
    I2Cwrite(slaveaddr, ELE4_R, REL_THRESH);
    I2Cwrite(slaveaddr, ELE5_T, TOU_THRESH);
    I2Cwrite(slaveaddr, ELE5_R, REL_THRESH);
    I2Cwrite(slaveaddr, ELE6_T, TOU_THRESH);
    I2Cwrite(slaveaddr, ELE6_R, REL_THRESH);
    I2Cwrite(slaveaddr, ELE7_T, TOU_THRESH);
    I2Cwrite(slaveaddr, ELE7_R, REL_THRESH);
    I2Cwrite(slaveaddr, ELE8_T, TOU_THRESH);
    I2Cwrite(slaveaddr, ELE8_R, REL_THRESH);
    I2Cwrite(slaveaddr, ELE9_T, TOU_THRESH);
    I2Cwrite(slaveaddr, ELE9_R, REL_THRESH);
    I2Cwrite(slaveaddr, ELE10_T, TOU_THRESH);
    I2Cwrite(slaveaddr, ELE10_R, REL_THRESH);
    I2Cwrite(slaveaddr, ELE11_T, TOU_THRESH);
    I2Cwrite(slaveaddr, ELE11_R, REL_THRESH);

    // Section D
    // Set the Filter Configuration
    // Set ESI2
    I2Cwrite(slaveaddr, FIL_CFG, 0x04);

    // Section E
    // Electrode Configuration
    // Enable 6 Electrodes and set to run mode
    // Set ELE_CFG to 0x00 to return to standby mode
    // I2Cwrite(slaveaddr, ELE_CFG, 0x0C);	// Enables all 12 Electrodes
    // Enable first 9 electrodes (0 for tap, 1-9 for swipe)
    I2Cwrite(slaveaddr, ELE_CFG, 0x09);
    
    // Section F
    // Enable Auto Config and auto Reconfig
    I2Cwrite(slaveaddr, ATO_CFG0, 0x0B);
    I2Cwrite(slaveaddr, ATO_CFGU, 0xC9);	// USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V
    I2Cwrite(slaveaddr, ATO_CFGL, 0x82);	// LSL = 0.65*USL = 0x82 @3.3V
    I2Cwrite(slaveaddr, ATO_CFGT, 0xB5);	// Target = 0.9*USL = 0xB5 @3.3V

    // nullterm touchdata for safety
    touchdata[TOUCHSIZE] = '\0';
#ifdef DEBUG
    // config I2C
    unsigned char i2cbyte;
    uart_chrstr("initializing touch...");
    uart_newline();
    
    i2cbyte = I2Cread(slaveaddr, 0x5C);
    if (i2cbyte == 16) // default value for ELE_CFG
        uart_chrstr("captouch working!\r\n");
    else
        uart_chrstr("captouch failed!\r\n");
#endif
};

// eight bits of previous touch status
char swipe_ele = 0x00;

// previous frame touches
unsigned int frames[5] = {0, 0, 0, 0, 0};

touchtype check_touch() {
    touchtype returnval;
    // init structure
    returnval.istap = false;
    returnval.isswipe = false;

    unsigned char i2cbyte, i2cbyte2;
    /*i2cbyte =  I2Cread(slaveaddr, 0x00);

    // ----------  DEBUG CODE  ---------------
    uart_newline();
    uart_binary(i2cbyte);
    uart_chrstr(" - b");
    i2cbyte =  I2Cread(slaveaddr, 0x01);
    uart_binary(i2cbyte);
    uart_chrstr(" - c");
    i2cbyte =  I2Cread(slaveaddr, 0x02);
    uart_binary(i2cbyte);
    uart_chrstr(" - d");
    i2cbyte =  I2Cread(slaveaddr, 0x03);
    uart_binary(i2cbyte);
    uart_chrstr(" - e");
    i2cbyte =  I2Cread(slaveaddr, ELE_CFG);
    uart_binary(i2cbyte);
    uart_chrstr("\r");*/
    // --- END DEBUG CODE ---

    i2cbyte =  I2Cread(slaveaddr, 0x00);
    i2cbyte2 =  I2Cread(slaveaddr, 0x01);
    /*i2c_start();
    send_i2c_byte(slaveaddr);
    send_i2c_byte(0x00);
    DelayuSec(10);

    i2c_restart();
    send_i2c_byte(slaveaddr | 0x01);
    i2cbyte = i2c_read_ack();
    i2cbyte2 = i2c_read();

    reset_i2c_bus();*/

    // combine electrodes
    unsigned int ele = ((i2cbyte2 & 0x0F) << 8) | i2cbyte;

    if (ele != frames[0] && ele != 0x0000) // add to frames
    {
        frames[5] = frames[4];
        frames[4] = frames[3];
        frames[3] = frames[2];
        frames[2] = frames[1];
        frames[1] = frames[0];
        frames[0] = ele;
        if (frames[0] == 0x0001 &&
            frames[1] == 0x0080 &&
            frames[2] == 0x0008) {
            if (MODE == 0) { // go into yoshi mode
                MODE = 1;
                COOLDOWN_TIME = 2;
                MAXAMMO = 50; // yes.
                ammo = MAXAMMO;
                play_sound(AUD_START);
                screen_display(MAIN);
                // UI display inits
                update_ammo(ammo);
                update_health(health);
                PR3 = 4318;
            }
        }
    }
    


    if (ele & 0x0100) // ninth bit, ele 8: tap event.
    {
        swipe_ele = 0x00;
        returnval.istap = true;
    }
    else
    if (ele & 0x00FF) // 8 electrodes (ele 0-7)
    {
        // check if ele is the first one, or one next to a previous one
        // pin 0 (bit 0) is actually the highest swipe strength
        // pin 7 (bit 7) is lowest swipe strength
        // loops upwards in swipe strength so always returns higher value
        if (ele & 0x80) {
            returnval.isswipe = true;    // if first electrode is touched
            returnval.swipestrength = 1;
            swipe_ele = ele; // set for next swipe check
        }
        int i;
        for(i = 0; i < 8; i++) {
            if ((ele & (0x80 >> i)) && // check if this ele is touched AND
                  ( (swipe_ele & (0x80 >> (i+1))) ||  // (ele before was touched OR
                    (swipe_ele & (0x80 >> (i-1)))     //  ele after was touched)
                  )
               )
            {
                returnval.isswipe = true;
                returnval.swipestrength = i+1;
                swipe_ele = ele; // set for next swipe check
            }
        }
    } else {
        // reset swipe status
        swipe_ele = 0x00;
    }



        /*
        char number[ITOASIZE] = "";
        char number2[ITOASIZE] = "";
        unsigned int combined = i2cbyte + ((i2cbyte2 & 0b00011111) << 8);
        itoa(number, combined, 10);
        bool ovcf = false;
        if (i2cbyte2 & 0x80) ovcf = true;

        // check OOR registers
        i2cbyte =  I2Cread(slaveaddr, 0x02);
        i2cbyte2 = I2Cread(slaveaddr, 0x03);
        unsigned int combined2 = i2cbyte + ((i2cbyte2 & 0b00011111) << 8);
        itoa(number2, combined2, 10);*/

        

    /*i2cbyte = I2Cread(slaveaddr, 0x01);

    if (i2cbyte & 0x80) {
        I2Cwrite(slaveaddr, 0x01, 0x00);    // reset OVCF
        I2Cwrite(slaveaddr, ELE_CFG, 0x0C); // Enable first 12 electrodes
    }*/
    /*if (strcmp(touchdata, "tap") == 0)
    {
        returnval.istap = true;
    }
    else if (strncmp(touchdata, "swipe ", sizeof("swipe ")-1) == 0)
    {
        // get swipe strength
        int strength = atoi(touchdata+sizeof("swipe"));
        // make sure we're sitting on valid data
        // don't declare a swipe unless there's a valid strength
        if (strength > 0) {
            returnval.isswipe = true;
            if (strength > 9) strength = 9; // 1-9 levels
            returnval.swipestrength = strength;
        }
    }*/
    return returnval;
};

unsigned int captouch_raw() {
    unsigned char i2cbyte, i2cbyte2;
    i2cbyte =  I2Cread(slaveaddr, 0x00);
    i2cbyte2 =  I2Cread(slaveaddr, 0x01);
    // combine electrodes
    unsigned int ele = ((i2cbyte2 & 0x0F) << 8) | i2cbyte;
    return ele;
}

#ifdef DEBUG
void dbg_touch() // ----- just reads a bunch of captouch status regs -> to UART
{
    char i2cbyte;
    i2cbyte =  I2Cread(slaveaddr, 0x00);

    uart_binary(i2cbyte);
    uart_chrstr(" - b");
    i2cbyte =  I2Cread(slaveaddr, 0x01);
    uart_binary(i2cbyte);
    uart_chrstr(" - c");
    i2cbyte =  I2Cread(slaveaddr, 0x02);
    uart_binary(i2cbyte);
    uart_chrstr(" - d");
    i2cbyte =  I2Cread(slaveaddr, 0x03);
    uart_binary(i2cbyte);
    uart_chrstr(" - e");
    i2cbyte =  I2Cread(slaveaddr, ELE_CFG);
    uart_binary(i2cbyte);
    uart_chrstr("\r");
}
#endif