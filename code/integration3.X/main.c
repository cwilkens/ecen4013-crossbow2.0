/* 
 * File:   main.c
 * Author: coltmw
 * AKA Colt Wilkens
 *
 * Created on March 5, 2014, 4:06 PM
 */

#include <libpic30.h> // hehehe. this is for flash.
#include <p24HJ64GP502.h>
#include <pps.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "logics.h"
#include "captouch.h"
#include "canbus.h"

#define MAXCMDLEN 40

extern char touchdata[TOUCHSIZE+1];

// game constants are in logics.h

// global state
typedef enum {
    EMPTY,
    LOADED,
    ARMED
} state_t;

// game constants!
int COOLDOWN_TIME = 4;
int MAXAMMO = 30;
int MAXHEALTH = 50;

state_t firing_state = EMPTY;
int ammo;
int health;
int cooldown_secs = 0;
int boltstrength = 0;
int MODE = 0;           // Easter Eggs. What else would it be?
struct SFLAGS {
    bool packet_received;  // IR packet received
    bool cooldown_display; // update cooldown display
    // update momentary messages on screen
    bool dmg_display;
    bool fire_display;
    int fire_anim;      // counter for firing (0: not firing, 1-4: frames)
    bool loading_display;
    int load_anim;
};
struct SFLAGS flags;


// IR packet info
enum MIRP packet;
// IR parsing variables
volatile unsigned int s_on, s_off, d_on1, d_off1, d_on2, d_off2, stop;
//volatile unsigned int s_on2, s_off2, s_on3, s_off3, s_on4, s_off4; // DEBUG
volatile char rx_state;     /*      0 : Scanning
                             *      1 : Data sampling
                             *      2 : Process
                             *      3 : Error
                             */

// rom number to increment through different startup bonus sounds
int __attribute__((space(prog),aligned(_FLASH_PAGE*2),address(0x6800))) dat[_FLASH_PAGE];

// other functions
void start_cooldown_timer();
void start_dmg_timer();
void start_fire_timer();
void start_load_timer();

void IO_init();

// FBS
#pragma config BWRP = WRPROTECT_ON      // Boot Segment Write Protect (Boot segment is write-protected)
#pragma config BSS = NO_FLASH           // Boot Segment Program Flash Code Protection (No Boot program Flash segment)
#pragma config RBS = NO_RAM             // Boot Segment RAM Protection (No Boot RAM)

// FSS
#pragma config SWRP = WRPROTECT_ON      // Secure Segment Program Write Protect (Secure segment is write-protected)
#pragma config SSS = NO_FLASH           // Secure Segment Program Flash Code Protection (No Secure Segment)
#pragma config RSS = NO_RAM             // Secure Segment Data RAM Protection (No Secure RAM)

// FGS
#pragma config GWRP = OFF               // General Code Segment Write Protect (User program memory is not write-protected)
#pragma config GSS = OFF                // General Segment Code Protection (User program memory is not code-protected)

// FOSCSEL
#pragma config FNOSC = FRC  // Oscillator Mode (Internal Fast RC (FRC) with divide by N)
//#pragma config FNOSC = LPRCDIVN  // Oscillator Mode (Internal Fast RC (FRC) with divide by N)
#pragma config IESO = OFF        // Internal External Switch Over Mode
// (Start-up device with FRC, then automatically switch to user-selected oscillator source when ready)
#pragma config FWDTEN = OFF     // turn off watchdog timer because oops.

#pragma config ALTI2C = OFF
#pragma config JTAGEN = OFF

#pragma config OSCIOFNC = ON            // OSC2 Pin Function (OSC2 pin has digital I/O function)

// FOSC= FIN*M/(N1*N2), FCY=FOSC/2
// FOSC= 8M*40(2*2)=80MHz for 8M input clock

/*
 * The prototype integration code for MAGE Crossbow 2.0
 * It uses skeleton functions in logics.h (implemented in logics.c)
 * And a UART connection from a computer that emulates inputs.
 *
 * This is where the magic happens.
 */
int main(int argc, char** argv) {

    // first up: clock timing.
    // Internal Fast RC oscillator (default) is FOSC = 7.37 MHz nominal.
    // (see pg 120 of datasheet)
    // DOZE is set to FCY/8 on default, but DOZEN sets it to off.
    // FCY (processor clock) is FOSC/2, so FCY = 3.685 (4 MHz)

    //PLLFBD = 50;
    // wait for PLL to start up
    // while (OSCCONbits.LOCK != 1) {};

    // note OSC2 pin was janky IO because of OSCIOFNC bit
    // which might be a config bit
    // pragma config OSCIOFNC = ON // to set OSC2 (RA3) to I/O pin3

    // next up: UART initialization
    // set all that analog shit to 0
    AD1PCFGL = 0xFFFF;
    unsigned int i, j, k; //??
#ifdef DEBUG
    RPOR0bits.RP0R = 0b00011 /* U1TX */;    // output pin RP0 for transmit
    RPINR18bits.U1RXR = 1;                  // input pin RP1 for receive

    // config BRG scaler
    // U1BRG = FCY/(16*baud) - 1
    // U1BRG = 3.685 MHz/(16*9600) - 1 = 23.99 - 1 = 23
    U1BRG = 23;

    U1MODEbits.STSEL = 0; // 1 stop bit
    U1MODEbits.PDSEL = 0; // 8 bit, no parity
    U1MODEbits.ABAUD = 0; // auto-baud off
    U1MODEbits.BRGH = 0;  // not high speed
    U1STAbits.URXISEL = 0; // disable interrupt?
    // enable UART
    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;  // enable transmit

    /* wait at least 104 ?s (1/9600) before sending first char */
    for(i = 0; i < 4160; i++)
    {
     Nop();  // DONT TAKE THIS OUT OR IT FREAKS OUT
    }
#endif

    _prog_addressT p;
    _init_prog_address(p, dat);
    int data1[_FLASH_ROW];
    // read first row out of flash
    _memcpy_p2d16(data1, p, _FLASH_ROW);
    //data1[0] = dat[0]; // read first value out of flash (sound count)

#ifdef DEBUG
        uart_chrstr("\r\nBooting...");
        uart_newline();
#endif
    // magical peripheral inits!
    IO_init();
    LED_init();
#ifdef DEBUG
        uart_chrstr("initializing OLED display...\r\n");
#endif
    OLED_init();
    screen_display(SPLASH);
    audio_init();
    touch_init();
    CAN_init();
    unsigned char dice = 0b10000000;
    for(i = 0; i < 7; i++)
    {
        update_led_bar(dice);
        for(j = 0; j < 10000; j++) {
            for(k = 0; k < 7; k++)
                Nop();
        }
        dice >>= 1;
    }
    // this is exactly what you think it is
    unsigned int ele = captouch_raw();
    if (ele) // & 0x0F)
        MODE = 1;
    play_sound(AUD_START); // STARTUP SOUND
    for(i = 0; i < 7; i++)
    {
        update_led_bar(dice);
        for(j = 0; j < 10000; j++) {
            for(k = 0; k < 7; k++)
                Nop();
        }
        dice <<= 1;
    }
    for(i = 0; i < 7; i++)
    {
        update_led_bar(dice);
        for(j = 0; j < 10000; j++)
        {
            for(k = 0; k < 7; k++)
                Nop();
        }
        dice >>= 1;
    }
    for(i = 0; i < 8; i++)
    {
        update_led_bar(dice);
        for(j = 0; j < 10000; j++) {
            for(k = 0; k < 7; k++)
                Nop();
        }
        dice <<= 1;
    }
    update_led_bar(0);
    // bonus_sound is data1[0] (one value, stored in flash)
    data1[0] += 1;
    if (data1[0] >= AUD_BONUS_SIZE)
        data1[0] = 0;
    //int startup =
    //__EEPROM_DATA(0, 0, 0, 0, 0, 0, 0, 0);
    //int startup = captouch_rand() % AUD_BONUS_SIZE;
    if (MODE == 1)
    {
        play_sound(AUD_START);
        MAXAMMO = 50; // yes.
    }
    else
        play_sound(AUD_BONUS_START + data1[0]);
    
    // write new value back to flash
    _erase_flash(p);
    _write_flash16(p, data1);

    //FLASH_Write();
    //bonus_sound += 1;
    //__prog__ char *target = 0x6800;
    //*target = bonus_sound +1;
    ammo = MAXAMMO;
    health = MAXHEALTH;
    screen_display(MAIN);
    //im_dead(); // for testing CANbus ------------
    

#ifdef DEBUG
        uart_chrstr("Mk 4 booted.\r\n\r\n");
#endif

    // UI display inits
    update_ammo(ammo);
    update_health(health);
    // initialize state variables
    flags.packet_received = false;
    flags.cooldown_display = false;
    flags.dmg_display = false;
    flags.fire_display = false;
    flags.fire_anim = 0;

    // UART variables
#ifdef DEBUG
    char value;
    char command[MAXCMDLEN] = "";
    int charcount = 0;
#endif
    
    while (1)
    {
        /*int val = CANrx();
        if (val != -1) {
            uart_chrstr("CANrx!\r\n");
            char num = ecan1MsgBuf[val][4]; // third data byte
            char number[ITOASIZE] = "";
            itoa(number, num, 10);
            uart_chrstr("data byte 3: ");
            uart_chrstr(number);
            uart_newline();
        }*/
        // check all flags from interrupts
        if (flags.packet_received)
        {
            flags.packet_received = false;
            // parse packet results
            if (packet == HIT)
            {
                if (T2CONbits.TON == 0) // if dmg timer is off, we haven't played a sound recently
                    play_sound(AUD_HIT);
                else if (PR2/TMR2 <= 2) // ex: 8686 / 5000 -> play sound if halfway through and hit again
                    play_sound(AUD_HIT);
                // make sure we're not drawing over the fire animation or cooldown
                if (flags.fire_anim == 0 && cooldown_secs == 0)
                    update_hit(true);       // turn on hit icon
                start_dmg_timer();      // start timer to turn off hit icon
                health -= 1;
                if (health < 0)
                    health = 0;
                update_health(health);  // update health display
            }
            if (packet == HEAL)
            {
                health += 1;
                if (health > MAXHEALTH)
                    health = MAXHEALTH;
                update_health(health);
            }
            if (health == 0) // soooo dead
            {
                screen_display(DEAD);
                play_sound(AUD_DEAD);
                bool sent_dead = false;
                while (!sent_dead)
                    sent_dead = im_dead();
                // dead, so eat cycles
                while (1) {};
            }
        }
        if (flags.cooldown_display)
        {
            flags.cooldown_display = false;
            cooldown_display(cooldown_secs);
        }
        if (flags.dmg_display)
        {
            flags.dmg_display = false;
            // make sure we're not drawing over the fire animation or cooldown
            if (flags.fire_anim == 0 && cooldown_secs == 0)
                update_hit(false);
        }
        if (flags.fire_display)
        {
            if (flags.fire_anim < 4) {
                flags.fire_anim += 1;
                start_fire_timer();
                fire_display(flags.fire_anim);
            } else {
                flags.fire_anim = 0;
                //fire_display(0); // maybe unneeded
                start_cooldown_timer();
                play_sound(AUD_COOLDOWN);
                cooldown_display(cooldown_secs);
            }
            flags.fire_display = false;
            //fire_display(false);
        }
        if (flags.loading_display)
        {
            update_load(2);
            //if (flags.load_anim < 2) {
            //    flags.load_anim += 1;
            //    start_load_timer();
            //    update_load(flags.load_anim);
            //} else {
            //    flags.load_anim = 0;
                //fire_display(0); // maybe unneeded
                //start_cooldown_timer();
                //play_sound(AUD_COOLDOWN);
                //cooldown_display(cooldown_secs);
            //}
            flags.loading_display = false;
            //fire_display(false);
        }
        
        touchtype event = check_touch();
        if (event.istap) {
#ifdef DEBUG
                uart_chrstr("TAP EVENT");
                //im_dead(); // testing CAN bus
                uart_newline();
#endif
            // do tap stuff
            if (firing_state == EMPTY && cooldown_secs == 0) {
                if (ammo > 0) {
                    start_load_timer();
                    flags.load_anim = 1;
                    update_load(1);
                    play_sound(AUD_LOAD);
                    firing_state = LOADED;
                }
                else
                {
                    // out of ammo stuff
                }
            }
            else if (firing_state == ARMED) {
                //update_load(false);
                play_sound(AUD_FIRE);
                start_fire_timer();
                flags.fire_anim = 1;
                fire_display(flags.fire_anim);
                shoot_packet(boltstrength);
                update_led_bar(0);
                firing_state = EMPTY;
                ammo -= 1;
                update_ammo(ammo);
                // cooldown TIMER is now started after the firing anim is done.
                // but still set counter, to prevent instant loading.
                cooldown_secs = COOLDOWN_TIME;
#ifdef DEBUG
                    uart_chrstr("after firing complete\r\n");
#endif
            }
        }
        if (event.isswipe) {
#ifdef DEBUG
                uart_chrstr("SWIPE EVENT, str = ");
                char number[ITOASIZE] = "";
                uart_chrstr(itoa(number, event.swipestrength, 10));
                uart_newline();
#endif
            // do swipe stuff
            if (firing_state == LOADED || firing_state == ARMED) { // ARMED for re-set
                boltstrength = event.swipestrength;
                int bar_bits = 0;
                int b = 0;
                for(b = 0; b < boltstrength; b++) {
                    // bitshift to form output
                    bar_bits = (bar_bits << 1) + 1;
                }
                update_led_bar(bar_bits);
                firing_state = ARMED;
            }
        }


        // ------------------ UART HANDLING -----------------
#ifdef DEBUG
            // what does this do
            if (U1STAbits.FERR == 1) {
                continue;
            }
            // clear overrun bit
            if (U1STAbits.OERR == 1) {
                U1STAbits.OERR = 0;
                continue;
            }
            if (U1STAbits.URXDA == 1) {
                // grab input, process if enter was pressed
                value = U1RXREG;
                if (value == '\r')
                {
                    if (charcount < MAXCMDLEN)
                        command[charcount] = 0; // try to null-term for safety
                    else
                        command[MAXCMDLEN] = 0; // chop off last char for null.
                    charcount = 0;
                    uart_newline();
                    // check command
                    /*if (strcmp(command, "toggle") == 0) {
                        // toggle LED
                        uart_chrstr("toggling LED");
                        uart_newline();
                        if (LATAbits.LATA4== 0)
                            LATAbits.LATA4 = 1;
                        else
                            LATAbits.LATA4 = 0;
                    }
                    if (strncmp(command, "touch ", sizeof("touch ")-1) == 0) {
                        // strip rest and put into touchdata
                        strcpy(touchdata, command+sizeof("touch ")-1);
                        uart_chrstr("grabbed: '");
                        uart_chrstr(touchdata);
                        uart_chrstr("'.");
                        uart_newline();
                    }*/
                } else {
                    // throw into command buffer, echo output
                    if (charcount < MAXCMDLEN) {
                        if (value == 127) { // ascii backspace (technically DEL)
                            charcount -= 1;
                        } else {
                            command[charcount] = value;
                            charcount += 1;
                        }
                    }
                    U1TXREG = value;
                }
            }
#endif // end UART handling
    }

    return (EXIT_SUCCESS);
}

void start_cooldown_timer()
{
    TMR1 = 0;               // reset timer count
    IEC0bits.T1IE = 1;      // enable timer1 interrupt
    T1CONbits.TON = 1;      // start timer
}

void start_dmg_timer()
{
    TMR2 = 0;               // reset timer count
    IEC0bits.T2IE = 1;      // enable timer2 interrupt
    T2CONbits.TON = 1;      // start timer
}

void start_fire_timer()
{
    TMR3 = 0;               // reset timer count
    IEC0bits.T3IE = 1;      // enable timer3 interrupt
    T3CONbits.TON = 1;      // start timer
}

void start_load_timer()
{
    TMR4 = 0;               // reset timer count
    IEC1bits.T4IE = 1;      // enable timer4 interrupt
    T4CONbits.TON = 1;      // start timer
}

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void)
{
    // ISR goes here
    if (cooldown_secs > 0)
    {
        cooldown_secs -= 1;
        flags.cooldown_display = true;
    } else {
        T1CONbits.TON = 0;      // turn off timer
        IEC0bits.T1IE = 0;      // disable timer1 interrupt
    }

    IFS0bits.T1IF = 0; // clear Timer1 interrupt flag
}

// these two are one-time interrupts, so turn off timer right after
void __attribute__((__interrupt__, no_auto_psv)) _T2Interrupt(void)
{
    flags.dmg_display = true;
    T2CONbits.TON = 0;      // turn off timer
    IEC0bits.T2IE = 0;      // disable timer interrupt
    IFS0bits.T2IF = 0;      // clear Timer2 interrupt flag
}

void __attribute__((__interrupt__, no_auto_psv)) _T3Interrupt(void)
{
    flags.fire_display = true;
    T3CONbits.TON = 0;      // turn off timer
    IEC0bits.T3IE = 0;      // disable timer interrupt
    IFS0bits.T3IF = 0;      // clear Timer3 interrupt flag
}

void __attribute__((__interrupt__, no_auto_psv)) _T4Interrupt(void)
{
    flags.loading_display = true;
    T4CONbits.TON = 0;      // turn off timer
    IEC1bits.T4IE = 0;      // disable timer interrupt
    IFS1bits.T4IF = 0;      // clear Timer4 interrupt flag
}

enum MIRP packet;
volatile unsigned int s_on, s_off, d_on1, d_off1, d_on2, d_off2, stop;
//volatile unsigned int s_on2, s_off2, s_on3, s_off3, s_on4, s_off4; // DEBUG
volatile char rx_state;     /*      0 : Scanning
                             *      1 : Data sampling
                             *      2 : Process
                             *      3 : Error
                             */

void __attribute__((__interrupt__, no_auto_psv)) _INT1Interrupt(void)
{
    // ISR goes here
    packet = ERROR;
    rx_state = 0;
    s_on = 0;
    s_off = 0;
    d_on1 = 0;
    d_off1 = 0;
    d_on2 = 0;
    d_off2 = 0;
    stop = 0;

    // DEBUG CODE (like, to count cycles for IR, not just UART stuff)
    /*s_on2 = 0;
    s_off2 = 0;
    s_on3 = 0;
    s_off3 = 0;
    s_on4 = 0;
    s_off4 = 0;
    d_on1 = 0;
    d_off1 = 0;
    d_on2 = 0;
    d_off2 = 0;
    stop = 0;

    while (ir_rec==0) {
        s_on = s_on + 1;
    }
    while (ir_rec==1) {
        s_off = s_off + 1;
    }

    while (ir_rec==0) {
        s_on2 = s_on2 + 1;
    }
    while (ir_rec==1) {
        s_off2 = s_off2 + 1;
    }

    while (ir_rec==0) {
        s_on3 = s_on3 + 1;
    }
    while (ir_rec==1) {
        s_off3 = s_off3 + 1;
    }

    while (ir_rec==0) {
        s_on4 = s_on4 + 1;
    }
    while (ir_rec==1) {
        s_off4 = s_off4 + 1;
    }

    char number[ITOASIZE] = "";
    sprintf(number,"%d",s_on);
    uart_chrstr("s_on: ");
    uart_chrstr(number);
    uart_newline();
    char number2[ITOASIZE] = "";
    sprintf(number2,"%d",s_off);
    uart_chrstr("s_off: ");
    uart_chrstr(number2);
    uart_newline();

    number[ITOASIZE] = "";
    sprintf(number,"%d",s_on2);
    uart_chrstr("s_on2: ");
    uart_chrstr(number);
    uart_newline();
    number2[ITOASIZE] = "";
    sprintf(number2,"%d",s_off2);
    uart_chrstr("s_off2: ");
    uart_chrstr(number2);
    uart_newline();

    number[ITOASIZE] = "";
    sprintf(number,"%d",s_on3);
    uart_chrstr("s_on3: ");
    uart_chrstr(number);
    uart_newline();
    number2[ITOASIZE] = "";
    sprintf(number2,"%d",s_off3);
    uart_chrstr("s_off3: ");
    uart_chrstr(number2);
    uart_newline();

    number[ITOASIZE] = "";
    sprintf(number,"%d",s_on4);
    //itoa(number, s_off, 10);
    uart_chrstr("s_on4: ");
    uart_chrstr(number);
    uart_newline();
    number2[ITOASIZE] = "";
    sprintf(number2,"%d",s_off4);
    //itoa(number, s_off, 10);
    uart_chrstr("s_off4: ");
    uart_chrstr(number2);
    uart_newline();*/

    /*** SCANNING ***/
    if (rx_state == 0) {
        while (ir_rx==0) {
            s_on = s_on + 1;
        }
        while (ir_rx==1) {
            s_off = s_off + 1;
            if (s_off > 1500) break;
        }
        if(s_off > MIRP_START_OFF_MAX || s_off < MIRP_START_OFF_MIN)
        {
            rx_state = 3;
            /*char number[ITOASIZE] = "";
            sprintf(number,"%d",s_on);
            uart_chrstr("s_on: ");
            uart_chrstr(number);
            uart_newline();
            char number2[ITOASIZE] = "";
            sprintf(number2,"%d",s_off);
            uart_chrstr("s_off: ");
            uart_chrstr(number2);
            uart_newline();*/
        }
        else
            rx_state = 1;
    }

    /*** FRAME ERROR ***/
    if(rx_state == 3) {
        rx_state = 0;

        s_on = 0;
        s_off = 0;
    }

    /*** DATA SAMPLING ***/
    if(rx_state == 1) {
        // Data
        while (ir_rx==0) {
            d_on1 = d_on1 + 1;
        }
        while (ir_rx==1) {
            d_off1 = d_off1 + 1;
            if (d_off1 > 1500) break;
        }

        // Redundant Data
        while (ir_rx==0) {
            d_on2 = d_on2 + 1;
        }
        while (ir_rx==1) {
            d_off2 = d_off2 + 1;
            if (d_off2 > 1500) break;
        }
        // Stop signal
        while (ir_rx==0) {
            stop = stop + 1;
        }
        rx_state = 2;
    }


    if(rx_state == 2)
    {
#ifdef DEBUG
        char number[ITOASIZE] = "";
        sprintf(number,"%d",d_on1);
        uart_chrstr("d_on1: ");
        uart_chrstr(number);
        uart_newline();
        sprintf(number,"%d",d_on2);
        uart_chrstr("d_on2: ");
        uart_chrstr(number);
        uart_newline();
#endif
        if(s_on > MIRP_START + MIRP_TOLERANCE || s_on < MIRP_START - MIRP_TOLERANCE)
        {
            rx_state = 0;
        }
        if(rx_state != 0)
        {
            if(d_on1 > MIRP_D1 - MIRP_TOLERANCE && d_on1 < MIRP_D1 + MIRP_TOLERANCE &&
               d_on2 > MIRP_D1 - MIRP_TOLERANCE && d_on2 < MIRP_D1 + MIRP_TOLERANCE)
            {
                //return HIT;
                packet = HIT;
            }
            else if(d_on1 > MIRP_D2 - MIRP_TOLERANCE && d_on1 < MIRP_D2 + MIRP_TOLERANCE &&
               d_on2 > MIRP_D2 - MIRP_TOLERANCE && d_on2 < MIRP_D2 + MIRP_TOLERANCE)
            {
                //return HEAL;
                packet = HEAL;
            }
            else if(d_on1 > MIRP_D3 - MIRP_TOLERANCE && d_on1 < MIRP_D3 + MIRP_TOLERANCE &&
               d_on2 > MIRP_D3 - MIRP_TOLERANCE && d_on2 < MIRP_D3 + MIRP_TOLERANCE)
            {
                //Third Packet
                packet = THIRD;
            }
            else if(d_on1 > MIRP_D4 - MIRP_TOLERANCE && d_on1 < MIRP_D4 + MIRP_TOLERANCE &&
               d_on2 > MIRP_D4 - MIRP_TOLERANCE && d_on2 < MIRP_D4 + MIRP_TOLERANCE)
            {
                //Fourth Packet
                packet = FOURTH;
            }
            else if(d_on1 > MIRP_D5 - MIRP_TOLERANCE && d_on1 < MIRP_D5 + MIRP_TOLERANCE &&
               d_on2 > MIRP_D5 - MIRP_TOLERANCE && d_on2 < MIRP_D5 + MIRP_TOLERANCE)
            {
                //Fifth Packet
                packet = FIFTH;
            }
            else if(d_on1 > MIRP_D6 - MIRP_TOLERANCE && d_on1 < MIRP_D6 + MIRP_TOLERANCE &&
               d_on2 > MIRP_D6 - MIRP_TOLERANCE && d_on2 < MIRP_D6 + MIRP_TOLERANCE)
            {
                //Sixth Packet
                packet = SIXTH;
            }
            else
            {
#ifdef DEBUG
                uart_chrstr("error in packet\r\n");
#endif
                //Error Packet
                packet = ERROR;
            }
        }

        s_on = 0;
        s_off = 0;
        s_on = 0;
        s_off = 0;
        d_on1 = 0;
        d_off1 = 0;
        d_on2 = 0;
        d_off2 = 0;
        stop = 0;

        rx_state = 0;
    }

    // raise packet flag to parse in main loop
    // should not be done in interrupt, because if the interrupt was called
    // in a draw function, the health bar's draw function could change
    // drawing addresses... causing stuff to be drawn in the wrong spot
    // when the interrupt returns
    flags.packet_received = true;

    IFS1bits.INT1IF = 0; // clear external interrupt flag
}

void __attribute__((__interrupt__, no_auto_psv)) _INT0Interrupt(void)
{
    // ISR goes here!
    // huh, we must've smoked something funky
    // somewhere back there when we all got the munchies
    // ate hershey's cookies and cream and got clumsy
    // cause there's white rappers everywhere and nobody's hungry
    
    // IRQ from cap touch chip
    // this is technically better than the polling up above, but seemed
    // inconsistent last time I tested it (in integration1)
    // sooo nbd, just ignore it.
    //check_touch();

    IFS0bits.INT0IF = 0; // clear external interrupt flag
}

void __attribute__((__interrupt__, no_auto_psv)) _DMA0Interrupt(void)
{
    IFS0bits.DMA0IF = 0;  //Clear the DMA0 Interrupt Flag
}

void IO_init()
{
#ifdef DEBUG
    uart_chrstr("initializing all I/O pins...\r\n");
#endif
    // init all TRIS bits and remappable bits
    tris_led_data = 0; // bit-banged SPI
    tris_led_clk  = 0; // ^
    //tris_led_ss   = 0; // now tied to 3.3V
    tris_ir_rx    = 1; // probably overridden by INT1, but oh well.
    tris_ir_tx    = 0; // bit-banged 56kHz MIRP packets
    //tris_ir_data  = 0; // bit-banged SPI for IR tx power levels
    //tris_ir_clk   = 0; // ^
    tris_oled_d_c = 0; // data/command toggle for OLED SPI
    tris_oled_res = 0; // reset line for OLED
    tris_aud_clk  = 0;
    tris_aud_data = 0;
    tris_aud_res  = 0;


    // one line, map pin for external interrupt
    RPINR0bits.INT1R = 5;     // input pin RP5 for IR input
    // INT0 is for captouch IRQ, static at pin RP7

    // remappable pins for SPI (to OLED)
    RPOR6bits.RP12R = 0x0007; //Set pin RP12 to SDO1
    RPOR6bits.RP13R = 0x0008; //Set pin RP13 to SCK1

    // remappable pins for CANbus
    RPOR5bits.RP11R = 0b10000 /* C1TX */;   // output pin RP11 for transmit
    RPINR26bits.C1RXR = 10;                 // input pin RP10 for receive
    
    // set timer interrupt for cooldown
    // approx 1 second
    T1CONbits.TON = 0;      // turn off during setup
    T1CONbits.TCS = 0;      // internal instruction clock source
    T1CONbits.TGATE = 0;    // disable gated timer
    T1CONbits.TCKPS = 0b11; // select 1:256 prescaler
    PR1 = 15625;            // period value (256*15625 = 4 MHz)
    IPC0bits.T1IP = 0x02;   // timer 1 interrupt priority level
    IFS0bits.T1IF = 0;      // clear timer1 interrupt flag
    // call start_cooldown_timer() to start timer
    // automatically turns off when cooldown count is 0
    //IEC0bits.T1IE = 1;      // enable timer1 interrupt
    //T1CONbits.TON = 1;      // start timer

    // set timer interrupt for "damage" flashing on screen
    T2CONbits.TON = 0;      // turn off during setup
    T2CONbits.T32 = 0;      // turn off 32 bit timer mode
    T2CONbits.TCS = 0;      // internal instruction clock source
    T2CONbits.TGATE = 0;    // disable gated timer
    T2CONbits.TCKPS = 0b11; // select 1:256 prescaler
    PR2 = 8636; //4318;             // period value (256*4318 = 1105405 -> ~300 ms)
    IPC1bits.T2IP = 0x01;   // timer 2 interrupt priority level
    IFS0bits.T2IF = 0;      // clear timer2 interrupt flag

    // set timer interrupt for a single frame of (4 frame) fire animation
    T3CONbits.TON = 0;      // turn off during setup
    T3CONbits.TCS = 0;      // internal instruction clock source
    T3CONbits.TGATE = 0;    // disable gated timer
    T3CONbits.TCKPS = 0b11; // select 1:256 prescaler
    PR3 = 2318; //4318;             // period value (256*4318 = 1105405 -> ~300 ms)
    IPC2bits.T3IP = 0x01;   // timer 3 interrupt priority level
    IFS0bits.T3IF = 0;      // clear timer3 interrupt flag

    // set timer interrupt for a single frame of (2 frame) loading animation
    T4CONbits.TON = 0;      // turn off during setup
    T4CONbits.TCS = 0;      // internal instruction clock source
    T4CONbits.TGATE = 0;    // disable gated timer
    T4CONbits.TCKPS = 0b11; // select 1:256 prescaler
    PR4 = 4318; //4318;             // period value (256*4318 = 1105405 -> ~300 ms)
    IPC6bits.T4IP = 0x01;   // timer 3 interrupt priority level
    IFS1bits.T4IF = 0;      // clear timer3 interrupt flag


    // set external interrupt for IR input (use ext1 because ext0 is not remappable)
    INTCON2bits.INT1EP = 1; // edge polarity, interrupt on negative edge
    IPC5bits.INT1IP = 0x04; // IR input interrupt priority level
    IFS1bits.INT1IF = 0;    // clear interrupt flag
    IEC1bits.INT1IE = 1;    // enable external interrupt

    // external interrupt for captouch IRQ
    INTCON2bits.INT0EP = 0; // edge polarity, interrupt on positive edge
    IPC0bits.INT0IP = 0x03; // cap touch input interrupt priority level
    IFS0bits.INT0IF = 0;    // clear interrupt flag
    IEC0bits.INT0IE = 1;    // enable external interrupt

}