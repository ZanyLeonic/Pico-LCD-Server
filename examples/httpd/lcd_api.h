#ifndef __LCD_API_H__
#define __LCD_API_H__

#include "pico/stdlib.h"
#include "pico/multicore.h"

#define LCD_CLR             0x01      // DB0: clear display
#define LCD_HOME            0x02      // DB1: return to home position

#define LCD_ENTRY_MODE      0x04      // DB2: set entry mode
#define LCD_ENTRY_INC       0x02      // --DB1: increment
#define LCD_ENTRY_SHIFT     0x01      // --DB0: shift

#define LCD_ON_CTRL         0x08      // DB3: turn lcd/cursor on
#define LCD_ON_DISPLAY      0x04      // --DB2: turn display on
#define LCD_ON_CURSOR       0x02      // --DB1: turn cursor on
#define LCD_ON_BLINK        0x01      // --DB0: blinking cursor

#define LCD_MOVE            0x10      // DB4: move cursor/display
#define LCD_MOVE_DISP       0x08      // --DB3: move display (0-> move cursor)
#define LCD_MOVE_RIGHT      0x04      // --DB2: move right (0-> left)

#define LCD_FUNCTION        0x20      // DB5: function set
#define LCD_FUNCTION_8BIT   0x10      // --DB4: set 8BIT mode (0->4BIT mode)
#define LCD_FUNCTION_2LINES 0x08      // --DB3: two lines (0->one line)
#define LCD_FUNCTION_10DOTS 0x04      // --DB2: 5x10 font (0->5x7 font)
#define LCD_FUNCTION_RESET  0x30      // See "Initializing by Instruction" section

#define LCD_CGRAM           0x40      // DB6: set CG RAM address
#define LCD_DDRAM           0x80      // DB7: set DD RAM address

#define LCD_RS_CMD          0
#define LCD_RS_DATA         1

#define LCD_RW_WRITE        0
#define LCD_RW_READ         1


struct LCDAPI_CONFIG
{
    int m_lines;
    int m_columns;
    
    int m_rs;
    int m_enable;

    int m_d4;
    int m_d5;
    int m_d6;
    int m_d7;

    int m_cursor_x;
    int m_cursor_y;
};

class LCDApi
{
    LCDAPI_CONFIG m_data;

    bool implied_newline;

public:
    LCDApi(LCDAPI_CONFIG config);

    void clear();

    void show_cursor();
    void hide_cursor();

    void blink_cursor_on();
    void blink_cursor_off();

    void display_on();
    void display_off();

    void move_to(int cursor_x, int cursor_y);
    void putchar(char character);
    void putstr(const char* string);

    void hal_write_command(uint cmd);
    void hal_write_data(uint data);
private:
    void init_pin(uint pin);

    void hal_write_4bits(uint nibble);
    void hal_pulse_enable();
    void hal_write_init_nibble(uint nibble);

    int string_size(const char * str);
};

#endif