// written by Colt Wilkens
// April 3, 2014
// part of Crossbow 2.0 for the MAGE system

#include <p24HJ64GP502.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "logics.h"

// see logics.h for pinout #defines

void LED_init()
{
    // init zeros to LED bars
    update_led_bar(0);
}


void update_led_bar(int bar_bits)
{
#ifdef DEBUG
    char number[ITOASIZE] = "";
    itoa(number, bar_bits, 2);
    uart_chrstr("led bar: ");
    uart_chrstr(number);
    uart_newline();
#endif
    // shift bits out to LED bar
    int i;
    for(i = 0; i < 8; i++)
    {
        led_clk = 0;
        led_data = (bar_bits >> i) & 0x1;
        led_clk = 1;
    }
    //led_ss = 1;
}