/* 
 * File:   spi_12832.h
 * Author: coltmw
 *
 * Created on April 4, 2014, 7:04 PM
 */

#ifndef SPI_12832_H
#define	SPI_12832_H

#ifdef	__cplusplus
extern "C" {
#endif

    //--------------------------------------------------------------------------
    void oled_command(unsigned char Data);
    void oled_data(unsigned char Data);

    void Set_Start_Column_12832(unsigned char d);
    void Set_Addressing_Mode_12832(unsigned char d);
    void Set_Column_Address_12832(unsigned char a, unsigned char b);
    void Set_Page_Address_12832(unsigned char a, unsigned char b);
    void Set_Start_Line_12832(unsigned char d);
    void Set_Contrast_Control_12832(unsigned char d);
    void Set_Area_Brightness_12832(unsigned char d);
    void Set_Segment_Remap_12832(unsigned char d);
    void Set_Entire_Display_12832(unsigned char d);
    void Set_Inverse_Display_12832(unsigned char d);
    void Set_Multiplex_Ratio_12832(unsigned char d);
    void Set_Dim_Mode_12832(unsigned char a, unsigned char b);
    void Set_Master_Config_12832(unsigned char d);
    void Set_Display_On_Off_12832(unsigned char d);
    void Set_Start_Page_12832(unsigned char d);
    void Set_Common_Remap_12832(unsigned char d);
    void Set_Display_Offset_12832(unsigned char d);
    void Set_Display_Clock_12832(unsigned char d);
    void Set_Area_Color_12832(unsigned char d);
    void Set_Precharge_Period_12832(unsigned char d);
    void Set_Common_Config_12832(unsigned char d);
    void Set_VCOMH_12832(unsigned char d);
    void Set_Read_Modify_Write_12832(unsigned char d);

    void Set_NOP_12832();

    void Set_LUT_12832(unsigned char a, unsigned char b, unsigned char c, unsigned char d);

    void Set_Bank_Color_12832();


#ifdef	__cplusplus
}
#endif

#endif	/* SPI_12832_H */

