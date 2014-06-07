/*
 *
 * Hey, did you hear?
 *
 * A guy (or possibly girl) named Colt wrote this.
 *
 */

#include <p24HJ64GP502.h>

#include "canbus.h"
#include "logics.h"

/* Assign 32x8 Word Message Buffers for ECAN1 in DMA RAM */
unsigned int ecan1MsgBuf[32][8] __attribute__((space(dma)));


void CAN_init()
{
    // request config mode
    C1CTRL1bits.REQOP=4;
    while (C1CTRL1bits.OPMODE != 4) {};
    // (remappable pins set in IO_init())

    // ----- set clock timing -----
    C1CFG1bits.BRP = CAN_BRP;   // see header
    C1CFG1bits.SJW = 0x0;       // set synch jump to 1xTQ
    C1CFG2bits.WAKFIL = 0;
    C1CFG2bits.SAM = 0x1;       // bus line sampled 3 times at sample point
    C1CFG2bits.SEG2PHTS = 0x1;  // phase2 time select bit
    C1CFG2bits.SEG1PH = 0x0;    // phase segment 1 is 1xTQ
    C1CFG2bits.PRSEG = 0x3;     // propagation time to 4xTQ
    C1CFG2bits.SEG2PH = 0x1;    // phase segment 2 is 2xTQ

    // buffer size
    // C1FCTRLbits.DMABS

    // ----- set receive filters -----
    C1CTRL1bits.WIN = 1; // set window to receive filters
    // create filter
    C1RXF0SIDbits.SID = 0x000;  // 11-bit SID to match
    C1RXF0SIDbits.EXIDE = 0b0;  // match only standard identifier messages
    C1BUFPNT1bits.F0BP = 0x1;   // put filter 0 hits into rx buffer 1
    C1FMSKSEL1bits.F0MSK = 0x0; // select mask 0 for filter 0
    // create mask
    C1RXM0SIDbits.SID = 0x700;  // mask top three bits to make sure they are zero (aka 8-bit SID) // mask NO bits (aka accept all)
    C1RXM0SIDbits.MIDE = 0b1;   // match message type of EXIDE bit (standard)
    // enable filter
    C1FEN1bits.FLTEN = 0x0001;  // disable all filters except filter 0

    // done with receive filters, set window back
    C1CTRL1bits.WIN = 0;

    // done with config, set to normal mode
    C1CTRL1bits.REQOP = 0b000;
    while (C1CTRL1bits.OPMODE != 0) {}; // wait to settle down

    // go into loopback mode for testing
    //C1CTRL1bits.REQOP = 0b010;
    //while (C1CTRL1bits.OPMODE != 0b010) {};

    // ----- initialize DMA for transmit -----
    DMA0CONbits.SIZE = 0x0;  // Transfer Size: Word Transfer Mode
    DMA0CONbits.DIR = 0x1;   // Transfer Direction: DMA RAM to Peripheral
    DMA0CONbits.AMODE = 0x2; // Addressing Mode: Peripheral Indirect
    DMA0CONbits.MODE = 0x0;  // Operating Mode: Continuous, Ping-Pong modes disabled
    DMA0REQ = 70;            // Assign ECAN1 Transmit event for DMA Channel 0
    DMA0CNT = 7;             // Set Number of DMA Transfer per ECAN message to 8 words
    DMA0PAD = 0x0442; //&C1TXD;         // Peripheral Address: ECAN1 Transmit Register
    /* Start Address Offset for ECAN1 Message Buffer 0x0000 */
    DMA0STA = __builtin_dmaoffset(ecan1MsgBuf);
    DMA0CONbits.CHEN = 0x1;  // Channel Enable: Enable DMA Channel 0
    // ? IEC0bits.DMA0IE = 1;     // Interrupt Enable: Enable DMA Channel 0 Interrupt

    // ----- initialize DMA for receive -----
    DMA1CONbits.SIZE = 0x0;  // Transfer Size: Word Transfer Mode
    DMA1CONbits.DIR = 0x0;   // Transfer Direction: Peripheral to DMA RAM
    DMA1CONbits.AMODE = 0x2; // Addressing Mode: Peripheral Indirect
    DMA1CONbits.MODE = 0x0;  // Operating Mode: Continuous, Ping-Pong modes disabled
    DMA1REQ = 34;            // Assign ECAN1 Receive event for DMA Channel 1
    DMA1CNT = 7;             // Set Number of DMA Transfer per ECAN message to 8 words
    DMA1PAD = 0x0440; //&C1RXD;        // Peripheral Address: ECAN1 Receive Register
    /* Start Address Offset for ECAN1 Message Buffer 0x0000 */
    DMA1STA = __builtin_dmaoffset(ecan1MsgBuf);
    DMA1CONbits.CHEN = 0x1;  // Channel Enable: Enable DMA Channel 1
    //IEC0bits.DMA1IE = 1;     //Channel Interrupt Enable: Enable DMA Channel 1 Interrupt


    // also this came from example 21-1 "standard data frame transmission"
    // see figure 21-8

    /* Assign 32x8word Message Buffers for ECAN1 in DMA RAM */
    //unsigned int ecan1MsgBuf[32][8] __attribute__((space(dma)));
    // ? DMA1STA = __builtin_dmaoffset(ecan1MsgBuf);
    /* Configure Message Buffer 2 for Transmission and assign priority */
    C1TR01CONbits.TXEN0 = 0x1;
    C1TR01CONbits.TXEN1 = 0x0; // buff 1 is receive
    C1TR01CONbits.TX0PRI = 0x3;
    C1TR01CONbits.TX1PRI = 0x3;

    

    // whelp let's send a connection request to the HIU
    //CANtx(0x02, DEVICE_ID, 0x01, 0x00, 0x00);

}


bool im_dead() {
#ifdef DEBUG
    uart_chrstr("you're totally dead, dude.");
    uart_newline();
#endif
    // whelp let's send a connection request to the HIU
    //CANtx(0x02, DEVICE_ID, 0x01, 0x00, 0x00); // for testing
    CANtx(0x00, 0x00, 0x00, 0x00, 0x00); // for testing

    // this is the im dead packet
    //CANtx(0x03, DEVICE_ID, 0xFE, 0x00, 0x00);

    return true; // hahaha, punk
}



void CANtx(unsigned char SIDH, // SIDH is the top 8 bits of 11-bit SID.
                               // aka "MSG ID" in MCAN payload specs.
           unsigned char d0, unsigned char d1,
           unsigned char d2, unsigned char d3)
{
    // see the MCAN payload specs on mage.okstate.edu

    /* can transmit code from DS70185 */

    
    /* WRITE TO MESSAGE BUFFER 0 */
    // cmw; see buffers in section 21.4 of DS70185

    /* CiTRBnSID = 0bxxx1 0010 0011 1100
    IDE = 0b0
    SRR = 0b0
    SID<10:0>= 0b100 1000 1111 */
    //ecan1MsgBuf[0][0] = 0x123C;
    ecan1MsgBuf[0][0] = 0x0000 | (SIDH << 5); // top 8 bits of SID, bitshift.
    /* CiTRBnEID = 0bxxxx 0000 0000 0000
    EID<17:6> = 0b0000 0000 0000 */
    ecan1MsgBuf[0][1] = 0x0000; // extended identifier, set to 0.
    /* CiTRBnDLC = 0b0000 0000 xxx0 1111
    EID<5:0> = 0b000000
    RTR = 0b0
    RB1 = 0b0
    RB0 = 0b0
    DLC = 0b1000 */
    ecan1MsgBuf[0][2] = 0x4; // sending four bytes (8 for testing)
    /* WRITE MESSAGE DATA BYTES */
    ecan1MsgBuf[0][3] = (d1 << 8) | d0;
    ecan1MsgBuf[0][4] = (d3 << 8) | d2;
    ecan1MsgBuf[0][5] = 0xabcd; // shouldn't care about setting these two.
    ecan1MsgBuf[0][6] = 0xabcd;
    /* REQUEST MESSAGE BUFFER 0 TRANSMISSION */
    C1TR01CONbits.TXREQ0 = 0x1;

};


int CANrx()
{
    int val = -1;
    // -1 if no packet
    // # if packet received, # is index of ecan1MsgBuf[n][0-7]
    if (C1RXFUL1bits.RXFUL0) // only filter enabled is filter 0
    {
        C1CTRL1bits.WIN = 1; // set window to access
        val = C1BUFPNT1bits.F0BP; // buffer set for filter 0
        C1CTRL1bits.WIN = 0;
    }
    return val;
}