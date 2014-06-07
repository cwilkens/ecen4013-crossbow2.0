/*
 * YEAAAHAHAHAHAHAHAH
 *  
 * Colt Wilkens wrote this one too.
 *
 * Also packet shooting could've been made better by using some sort of PWM
 * module. And DMA'd the period or on/off signal to it. BAM, non-blocking.
 *
 * But:
 * Ain't nobody got time fo' dat.
 */

#include <p24HJ64GP502.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "logics.h"

// PROTOTYPING STUFF (ie uart output)
#ifdef DEBUG
void uart_newline() {
    U1TXREG = '\r';
    while (U1STAbits.TRMT == 0) {}; // wait while transmitting
    U1TXREG = '\n';
    while (U1STAbits.TRMT == 0) {}; // wait while transmitting
}

void uart_chrstr(char* string) {
    int i = 0;
    while (string[i] != '\0') {
        U1TXREG = string[i];
        i++;
        while (U1STAbits.TRMT == 0) {}; // wait while transmitting
    }
}
#endif

// EVERYBODYS LOGIC FUNCTIONS STUFF (for prototyping stage of main loop)

/*void update_ammo(int amount) {
    char number[ITOASIZE] = "";
    itoa(number, amount, 10);
    uart_chrstr("ammo level at ");
    uart_chrstr(number);
    uart_newline();
};*/

/*void update_health(int health) {
    char number[ITOASIZE] = "";
    itoa(number, health, 10);
    uart_chrstr("health at ");
    uart_chrstr(number);
    uart_newline();
};*/

/*void update_load(bool loaded) {
    if (loaded)
        uart_chrstr("bolt loaded!");
    else
        uart_chrstr("no bolt loaded...");
    uart_newline();
};*/

/*void update_led_bar(int bar_bits) {
    char number[ITOASIZE] = "";
    itoa(number, bar_bits, 2);
    uart_chrstr("led bar: ");
    uart_chrstr(number);
    uart_newline();
};*/

/*void fire_display(bool firing) {
    if (firing)
        uart_chrstr("firing bolt!");
    else
        uart_chrstr("not firing...");
    uart_newline();
};*/

/*void cooldown_display(int count) {
    char number[ITOASIZE] = "";
    itoa(number, count, 10);
    uart_chrstr("cooldown at ");
    uart_chrstr(number);
    uart_newline();
};*/


// CAN logic
/*bool im_dead() {
    uart_chrstr("you're totally dead, dude.");
    uart_newline();
    return true;
};*/

// sound logic
/*void play_sound(int soundnumber) {
    char number[ITOASIZE] = "";
    itoa(number, soundnumber, 10);
    uart_chrstr("playing sound ");
    uart_chrstr(number);
    uart_newline();
};*/

void signal_on(unsigned char a) {
    int i;
    // having an inner for loop with Nops was too inconsistent on the cycle count.
    for(i = 0; i <= a; i++) {
        ir_tx = 1;
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        ir_tx = 0;
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
    }
}

void signal_off(unsigned char b) {
    int i;
    for(i = 0; i <= b; i++) {
        ir_tx = 1;
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        ir_tx = 1;
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
    }
}

void MIRPHit() {
    signal_on(10);
    signal_off(140);

    signal_on(20);
    signal_off(130);
    signal_on(20);
    signal_off(130);

    signal_on(100);
}
void shoot_packet(int strength) {
#ifdef DEBUG
    char number[ITOASIZE] = "";
    itoa(number, strength, 10);
    uart_chrstr("shooting MIRP packet with strength of ");
    uart_chrstr(number);
    uart_newline();
#endif
    // shoot packet
    IEC1bits.INT1IE = 0;    // DISABLE IR INTERRUPT BECAUSE HOLY JESUS
    int i, j;
    for(i = 0; i < 3*strength; i++)
    {
        //signal_on(200);
        for(j = 0; j < 1800; j++)
            Nop();
            //signal_on(10);
        MIRPHit();
    }
    IEC1bits.INT1IE = 1; // re-enable because FML
};

