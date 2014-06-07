/* 
 * File:   logics.h
 * Author: coltmw
 *
 * Honestly, Colt wrote all of the files.
 *
 * All of them.
 *
 * Created on March 5, 2014, 5:24 PM
 */

#ifndef LOGICS_H
#define	LOGICS_H

#include <stdbool.h>

// yo yo yo, right at the top!
// uncomment this to turn on UART debugging
// (uses RP0 for tx, RP1 for rx, connected to programming header!)
// #define DEBUG 1

// game constants!
extern int COOLDOWN_TIME;
extern int MAXAMMO;
extern int MAXHEALTH;
extern int ammo;
extern int health;

#define OUT_FN_PPS_C1OUT			0x0001				/* RPn tied to Comparator1 Output */
#define OUT_FN_PPS_C2OUT			0x0002				/* RPn tied to Comparator2 Output */
#define OUT_FN_PPS_U1TX				0x0003				/* RPn tied to UART1 Transmit */
#define OUT_FN_PPS_U1RTS			0x0004				/* RPn tied to UART1 Ready To Send */
#define OUT_FN_PPS_U2TX				0x0005				/* RPn tied to UART2 Transmit */
#define OUT_FN_PPS_U2RTS			0x0006				/* RPn tied to UART2 Ready To Send */
#define OUT_FN_PPS_SDO1				0x0007				/* RPn tied to SPI1 Data Output */
#define OUT_FN_PPS_SCK1				0x0008				/* RPn tied to SPI1 Clock Output */
#define OUT_FN_PPS_SS1				0x0009				/* RPn tied to SPI1 Slave Select Output */
#define OUT_FN_PPS_SDO2				0x000A				/* RPn tied to SPI2 Data Output */
#define OUT_FN_PPS_SCK2				0x000B				/* RPn tied to SPI2 Clock Output */
#define OUT_FN_PPS_SS2				0x000C				/* RPn tied to SPI2 Slave Select Output */
#define OUT_FN_PPS_CSDO				0x000D				/* RPn tied to DCI Serial Data Output*/
#define OUT_FN_PPS_CSCKOUT			0x000E				/* RPn tied to DCI Serial Clock Output*/
#define OUT_FN_PPS_COFSOUT			0x000F				/* RPn tied to DCI Frame Sync Output*/
#define OUT_FN_PPS_C1TX				0x0010				/* RPn tied to ECAN1 Transmit */
#define OUT_FN_PPS_OC1				0x0012				/* RPn tied to Output Compare 1 */
#define OUT_FN_PPS_OC2				0x0013				/* RPn tied to Output Compare 2 */
#define OUT_FN_PPS_OC3				0x0014				/* RPn tied to Output Compare 3 */
#define OUT_FN_PPS_OC4				0x0015				/* RPn tied to Output Compare 4 */
#define OUT_FN_PPS_UPDN1			0x001A				/* RPn tied to QEI1 UPDN Output */
#define OUT_FN_PPS_UPDN2			0x001B				/* RPn tied to QEI2 UPDN Output */


// ---- MASTER PINOUT ----
#define led_clk         LATAbits.LATA0
#define led_data        LATAbits.LATA1
//#define led_ss          LATAbits.LATA2
#define tris_led_clk    TRISAbits.TRISA0
#define tris_led_data   TRISAbits.TRISA1
//#define tris_led_ss     TRISAbits.TRISA2

#define ir_rx           PORTBbits.RB5
#define tris_ir_rx      TRISBbits.TRISB5

#define ir_tx           PORTBbits.RB4
//#define ir_data         LATAbits.LATA2
//#define ir_clk          LATAbits.LATA3
#define tris_ir_tx      TRISBbits.TRISB4
//#define tris_ir_data    TRISAbits.TRISA2
//#define tris_ir_clk     TRISAbits.TRISA3

#define aud_clk         LATBbits.LATB2
#define aud_data        LATBbits.LATB3
#define aud_res         LATAbits.LATA2
#define tris_aud_clk    TRISBbits.TRISB2
#define tris_aud_data   TRISBbits.TRISB3
#define tris_aud_res    TRISAbits.TRISA2

#define oled_d_c        LATBbits.LATB14
#define oled_res        LATBbits.LATB15
#define tris_oled_d_c   TRISBbits.TRISB14
#define tris_oled_res   TRISBbits.TRISB15

// see IO_init() for remappable pin settings

// audio song numbers
#define AUD_FIRE        1
#define AUD_LOAD        2
#define AUD_HIT         3
#define AUD_COOLDOWN    4
#define AUD_DEAD        5
#define AUD_START       6

#define AUD_BONUS_START 13
#define AUD_BONUS_SIZE  6

// for IR receive
// MAGIC NUMBERS FOR 3.685Mhz//8MHz
#define MIRP_ENVELOPE       1050//560
#define MIRP_START          90//37
#define MIRP_START_OFF_MAX  730//1070//530
#define MIRP_START_OFF_MIN  700//1030//470
#define MIRP_D1             168//75
#define MIRP_D2             238//258//112
#define MIRP_D3             344//149
#define MIRP_D4             430//187
#define MIRP_D5             516//224
#define MIRP_D6             602//261
#define MIRP_TOLERANCE      15//15


#ifdef	__cplusplus
extern "C" {
#endif

    // worst case itoa string size
    // (sizeof(int)*8+1) for radix=2 (16 bit cpu=2 bytes, 2*8+1)
    #define ITOASIZE 17
    

    // --- general prototypes for prototyping (let's get meta!)
#ifdef DEBUG
    void uart_newline();
    void uart_chrstr(char* string);
    void uart_binary(char value);
    void uart_binary_16(int value);
#endif

    extern int MODE;
    
    // --- prototypes for other logic blocks
    // visual logic
    enum SCRN {
        SPLASH,
        MAIN,
        DEAD
    };
    void OLED_init();
    void screen_display(enum SCRN mode);
    void update_ammo(int amount);
    void update_health(int health);
    void update_load(int frame);
    void fire_display(int frame);
    void update_hit(bool hit);
    void cooldown_display(int count);

    // visual led bar
    void LED_init();
    void update_led_bar(int bar_bits);

    // CAN logic
    void CAN_init();
    bool im_dead();

    // sound logic
    void audio_init();
    void play_sound(int soundnumber);

    // IR logic
    enum MIRP {
        ERROR,
        HIT,
        HEAL,
        THIRD,
        FOURTH,
        FIFTH,
        SIXTH
    };
    //enum MIRP receive_packet();     // functionality now implemented in INT1
    void shoot_packet(int strength);

    void set_ir_tx(char bits);

    // in captouch.c, but putting prototype here for access
    unsigned int captouch_raw();
    

#ifdef	__cplusplus
}
#endif

#endif	/* LOGICS_H */

