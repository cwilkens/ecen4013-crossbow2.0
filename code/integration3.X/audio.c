
/*
 * File:   audio.c
 * Author: Who do you think?
 *
 * We switched audio boards after integration 1.
 *
 * Like, passed int1 with a 100%, and used the mic audio board,
 * then switched to the sd card audio board.
 *
 * Because it's better. 
 *
 * Created on March 23, 2014, 3:40 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <p24HJ64GP502.h>

#include "logics.h"

void audio_init()
{
    int i;
    aud_clk = 1;
    aud_data = 1;
    aud_res = 1;
    for(i = 0; i < 18425; i++)  // 5 ms?
        Nop();
    aud_res = 0;
    for(i = 0; i < 18425; i++)  // 5 ms?
        Nop();
    aud_res = 1;
    //sdi_out(0xFFF1); // second lowest volume level? (FFF0 min, FFF7 max)
    // FFFE play/pause
    // FFFF stop
    // 0000 to 00FF play file (0000.ad4 to 0511.ad4)

}
void sdi_out(int value)
{
    int i, j;

    aud_clk = 1;
    aud_clk = 0;
    // wait 2 ms? (to pull out of sleep mode)
    for(i = 0; i < 16370; i++)
    {
        Nop();
    }
    for(i = 0; i < 16; i++)
    {
        //
        aud_clk = 0;
        // msb first
        aud_data = (value >> (15-i)) & 0x01;
        for(j = 0; j < 368; j++) // 200 us? // 737
            Nop();
        aud_clk = 1;
        for(j = 0; j < 368; j++) // 200 us? // 1474
            Nop();
    }
    aud_data = 1;
}

// sound logic
void play_sound(int soundnumber)
{
#ifdef DEBUG
        char number[ITOASIZE] = "";
        itoa(number, soundnumber, 10);
        uart_chrstr("playing sound ");
        uart_chrstr(number);
        uart_newline();
#endif

    //sdi_out(0xFFF7);
    sdi_out(soundnumber + MODE*6);
};