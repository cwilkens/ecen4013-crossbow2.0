/* 
 * File:   canbus.h
 * Author: coltmw
 * Created on April 21, 2014, 6:06 PM
 *
 * WAIT... Colt wrote this one too?
 * Man, he must be crazy or something.
 *
 * Also, CANbus on the PIC24... just don't do it, guys.
 *
 * Really. Colt didn't even finish it.
 *
 * But! In retrospect, I woulda used a PIC18F as a coprocessor to
 * handle CANbus and IR tx/rx (since it's time-sensitive) and just
 * used SPI between the two.
 *
 * And I totally coulda put it on the ultiboard at the same size, too.
 *
 */

#ifndef CANBUS_H
#define	CANBUS_H

#ifdef	__cplusplus
extern "C" {
#endif

// let's get some defines for all the COOL CANBUS STUFF we're going to do.
// Addendum: no canbus stuff happened. Because PIC24 is ridank and graphics
// are cooler.
    
// --- basic config defines ---
// BRG prescaler. On PIC18F2580, in reference MCAN.c (mage website), BRG = 7
// this leads to: TQ = (2 x (1+BRG))/FOSC
// which becomes: TQ = (2 x 8)/(8 MHz)
// TQ = 0.000002 (2 us)
// on our PIC24HJ64GP502, BRP is configured as:
// TQ = (2 x (1+BRP))/FCAN
// according to DS70226, FCAN is FCY, our FCY is 3.685 MHz
// so, solve for BRP:
// ((TQ * FCAN)/2)-1 = BRP
// BRP = 2.685 -> let's go to 3 because I'm lazy.
#define CAN_BRP 3 // see above

// sweet, sweet protocol #defines
#define DEVICE_ID 0xA1 // because we can pick anything above 0x4 and below 0xFF

/* Assign 32x8 Word Message Buffers for ECAN1 in DMA RAM */
extern unsigned int ecan1MsgBuf[32][8] __attribute__((space(dma)));

    //CANtx(0x02, DEVICE_ID, 0xFE, 0x00, 0x00);
void CANtx(unsigned char SIDH,
           unsigned char d0, unsigned char d1,
           unsigned char d2, unsigned char d3);

int CANrx();

#ifdef	__cplusplus
}
#endif

#endif	/* CANBUS_H */

