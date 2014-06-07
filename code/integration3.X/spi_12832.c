/*
 * header for SPI to oled screen... driver chip 12832 (pulled from internet)
 *
 */

#include <p24HJ64GP502.h>

#include "logics.h"


//--------------------------------------------------------------------------
void oled_command(unsigned char Data)
{
    SPI1BUF = Data;
    oled_d_c = 0;   //D/C
    int i;
    // Nops because SPI TBF bit is broken, see silicon errata
    for(i = 0; i < 40; i++) {
        Nop();
    }
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void oled_data(unsigned char Data)
{
    SPI1BUF = Data;
    oled_d_c = 1;   //D/C
    int i;
    // Nops because SPI TBF bit is broken, see silicon errata
    for(i = 0; i < 40; i++) {
        Nop();
    }
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void Set_Start_Column_12832(unsigned char d)
{
	oled_command(0x00+d%16);		// Set Lower Column Start Address for Page Addressing Mode
						//   Default => 0x00
	oled_command(0x10+d/16);		// Set Higher Column Start Address for Page Addressing Mode
						//   Default => 0x10
}
//--------------------------------------------------------------------------

void Set_Addressing_Mode_12832(unsigned char d)
{
	oled_command(0x20);			// Set Memory Addressing Mode
	oled_command(d);			//   Default => 0x02
						//     0x00 => Horizontal Addressing Mode
						//     0x01 => Vertical Addressing Mode
						//     0x02 => Page Addressing Mode
}
//--------------------------------------------------------------------------

void Set_Column_Address_12832(unsigned char a, unsigned char b)
{
	oled_command(0x21);			// Set Column Address
	oled_command(a);			//   Default => 0x00 (Column Start Address)
	oled_command(b);			//   Default => 0x83 (Column End Address)
}
//--------------------------------------------------------------------------

void Set_Page_Address_12832(unsigned char a, unsigned char b)
{
	oled_command(0x22);			// Set Page Address
	oled_command(a);			//   Default => 0x00 (Page Start Address)
	oled_command(b);			//   Default => 0x07 (Page End Address)
}
//--------------------------------------------------------------------------

void Set_Start_Line_12832(unsigned char d)
{
	oled_command(0x40|d);			// Set Display Start Line
						//   Default => 0x40 (0x00)
}
//--------------------------------------------------------------------------

void Set_Contrast_Control_12832(unsigned char d)
{
	oled_command(0x81);			// Set Contrast Control for Bank 0
	oled_command(d);			//   Default => 0x80
}
//--------------------------------------------------------------------------

void Set_Area_Brightness_12832(unsigned char d)
{
	oled_command(0x82);			// Set Brightness for Area Color Banks
	oled_command(d);			//   Default => 0x80
}
//--------------------------------------------------------------------------

void Set_Segment_Remap_12832(unsigned char d)
{
	oled_command(0xA0|d);			// Set Segment Re-Map
						//   Default => 0xA0
						//     0xA0 (0x00) => Column Address 0 Mapped to SEG0
						//     0xA1 (0x01) => Column Address 0 Mapped to SEG131
}
//--------------------------------------------------------------------------

void Set_Entire_Display_12832(unsigned char d)
{
	oled_command(0xA4|d);			// Set Entire Display On / Off
						//   Default => 0xA4
						//     0xA4 (0x00) => Normal Display
						//     0xA5 (0x01) => Entire Display On
}
//--------------------------------------------------------------------------

void Set_Inverse_Display_12832(unsigned char d)
{
	oled_command(0xA6|d);			// Set Inverse Display On/Off
						//   Default => 0xA6
						//     0xA6 (0x00) => Normal Display
						//     0xA7 (0x01) => Inverse Display On
}
//--------------------------------------------------------------------------

void Set_Multiplex_Ratio_12832(unsigned char d)
{
	oled_command(0xA8);			// Set Multiplex Ratio
	oled_command(d);			//   Default => 0x3F (1/64 Duty)
}
//--------------------------------------------------------------------------

void Set_Dim_Mode_12832(unsigned char a, unsigned char b)
{
	oled_command(0xAB);			// Set Dim Mode Configuration
	oled_command(0X00);			//           => (Dummy Write for First Parameter)
	oled_command(a);			//   Default => 0x80 (Contrast Control for Bank 0)
	oled_command(b);			//   Default => 0x80 (Brightness for Area Color Banks)
	oled_command(0xAC);			// Set Display On in Dim Mode
}
//--------------------------------------------------------------------------

void Set_Master_Config_12832(unsigned char d)
{
	oled_command(0xAD);			// Set Master Configuration
	oled_command(0x8E|d);			//   Default => 0x8E
						//     0x8E (0x00) => Select External VCC Supply
						//     0x8F (0x01) => Select Internal DC/DC Voltage Converter
}
//--------------------------------------------------------------------------

void Set_Display_On_Off_12832(unsigned char d)
{
	oled_command(0xAE|d);			// Set Display On/Off
						//   Default => 0xAE
						//     0xAE (0x00) => Display Off
						//     0xAF (0x01) => Display On
}
//--------------------------------------------------------------------------

void Set_Start_Page_12832(unsigned char d)
{
	oled_command(0xB0|d);			// Set Page Start Address for Page Addressing Mode
						//   Default => 0xB0 (0x00)
}
//--------------------------------------------------------------------------

void Set_Common_Remap_12832(unsigned char d)
{
	oled_command(0xC0|d);			// Set COM Output Scan Direction
						//   Default => 0xC0
						//     0xC0 (0x00) => Scan from COM0 to 63
						//     0xC8 (0x08) => Scan from COM63 to 0
}
//--------------------------------------------------------------------------

void Set_Display_Offset_12832(unsigned char d)
{
	oled_command(0xD3);			// Set Display Offset
	oled_command(d);			//   Default => 0x00
}
//--------------------------------------------------------------------------

void Set_Display_Clock_12832(unsigned char d)
{
	oled_command(0xD5);			// Set Display Clock Divide Ratio / Oscillator Frequency
	oled_command(d);			//   Default => 0x70
						//     D[3:0] => Display Clock Divider
						//     D[7:4] => Oscillator Frequency
}
//--------------------------------------------------------------------------

void Set_Area_Color_12832(unsigned char d)
{
	oled_command(0xD8);			// Set Area Color Mode On/Off & Low Power Display Mode
	oled_command(d);			//   Default => 0x00 (Monochrome Mode & Normal Power Display Mode)
}
//--------------------------------------------------------------------------

void Set_Precharge_Period_12832(unsigned char d)
{
	oled_command(0xD9);			// Set Pre-Charge Period
	oled_command(d);			//   Default => 0x22 (2 Display Clocks [Phase 2] / 2 Display Clocks [Phase 1])
						//     D[3:0] => Phase 1 Period in 1~15 Display Clocks
						//     D[7:4] => Phase 2 Period in 1~15 Display Clocks
}
//--------------------------------------------------------------------------

void Set_Common_Config_12832(unsigned char d)
{
	oled_command(0xDA);			// Set COM Pins Hardware Configuration
	oled_command(0x02|d);			//   Default => 0x12 (0x10)
						//     Alternative COM Pin Configuration
						//     Disable COM Left/Right Re-Map
}
//--------------------------------------------------------------------------

void Set_VCOMH_12832(unsigned char d)
{
	oled_command(0xDB);			// Set VCOMH Deselect Level
	oled_command(d);			//   Default => 0x34 (0.77*VCC)
}
//--------------------------------------------------------------------------

void Set_Read_Modify_Write_12832(unsigned char d)
{
	oled_command(0xE0|d);			// Set Read Modify Write Mode
						//   Default => 0xE0
						//     0xE0 (0x00) => Enter Read Modify Write
						//     0xEE (0x0E) => Exit Read Modify Write
}
//--------------------------------------------------------------------------

void Set_NOP_12832()
{
	oled_command(0xE3);			// Command for No Operation
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Bank Color & Look Up Table Setting (Partial Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Set_LUT_12832(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
	oled_command(0x91);			// Define Look Up Table of Area Color
	oled_command(a);			//   Define Bank 0 Pulse Width
	oled_command(b);			//   Define Color A Pulse Width
	oled_command(c);			//   Define Color B Pulse Width
	oled_command(d);			//   Define Color C Pulse Width
}

void Set_Bank_Color_12832()
{
	oled_command(0x92);			// Define Area Color for Bank 1~16 (Page 0)
	oled_command(0x00);			//   Define Bank 1~4 as Color A
	oled_command(0x55);			//   Define Bank 5~8 as Color B
	oled_command(0xAA);			//   Define Bank 9~12 as Color C
	oled_command(0xFF);			//   Define Bank 13~16 as Color D

	oled_command(0x93);			// Define Area Color for Bank 17~32 (Page 1)
	oled_command(0xFF);			//   Define Bank 17~32 as Color D
	oled_command(0xFF);
	oled_command(0xFF);
	oled_command(0xFF);
}