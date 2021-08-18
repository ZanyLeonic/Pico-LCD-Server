#ifndef __LCD_API_H__
#define __LCD_API_H__

#include <stdio.h>
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

struct lcdapi_data
{
    int lines;
    int columns;
    
    int rs;
    int enable;

    int d4;
    int d5;
    int d6;
    int d7;

    int cursor_x;
    int cursor_y;

    bool implied_newline;
};

void InitPin(uint pin);

void LCD_Init(struct lcdapi_data *data);
void LCD_Clear(struct lcdapi_data *data);

void LCD_ShowCursor(struct lcdapi_data *data);
void LCD_HideCursor(struct lcdapi_data *data);

void LCD_BlinkCursorOn(struct lcdapi_data *data);
void LCD_BlinkCursorOff(struct lcdapi_data *data);

void LCD_DisplayOn(struct lcdapi_data *data);
void LCD_DisplayOff(struct lcdapi_data *data);

void LCD_MoveTo(struct lcdapi_data *data, int cursor_x, int cursor_y);
void LCD_PutChar(struct lcdapi_data *data, char character);
void LCD_PutStr(struct lcdapi_data *data, const char* string);

void LCD_HalWriteCommand(struct lcdapi_data *data, uint cmd);
void LCD_HalWriteData(struct lcdapi_data *ldata, uint data);

void LCD_HalWrite4Bits(struct lcdapi_data *data, uint nibble);
void LCD_HalPulseEnable(struct lcdapi_data *data);
void LCD_HalWriteInitNibble(struct lcdapi_data *data, uint nibble);

int StringSize(const char * str);

#endif