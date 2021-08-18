#include "lcd_api.h"

void InitPin(uint pin)
{
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 0);
}

void LCD_Clear(struct lcdapi_data *data)
{
    LCD_HalWriteCommand(data, LCD_CLR);
    sleep_ms(1);
    LCD_HalWriteCommand(data, LCD_HOME);
    sleep_ms(1);
    
    data->cursor_x = 0;
    data->cursor_y = 0;
}

void LCD_ShowCursor(struct lcdapi_data *data)
{
    LCD_HalWriteCommand(data, LCD_ON_CTRL | LCD_ON_DISPLAY | LCD_ON_CURSOR);
}

void LCD_HideCursor(struct lcdapi_data *data)
{
    LCD_HalWriteCommand(data, LCD_ON_CTRL | LCD_ON_DISPLAY);
}

void LCD_BlinkCursorOff(struct lcdapi_data *data)
{
    LCD_HalWriteCommand(data, LCD_ON_CTRL | LCD_ON_DISPLAY | LCD_ON_CURSOR);
}

void LCD_BlinkCursorOn(struct lcdapi_data *data)
{
    LCD_HalWriteCommand(data, LCD_ON_CTRL | LCD_ON_DISPLAY | LCD_ON_CURSOR | LCD_ON_BLINK);
}

void LCD_DisplayOff(struct lcdapi_data *data)
{
    LCD_HalWriteCommand(data, LCD_ON_CTRL);
}

void LCD_DisplayOn(struct lcdapi_data *data)
{
    LCD_HalWriteCommand(data, LCD_ON_CTRL | LCD_ON_DISPLAY);
}

void LCD_MoveTo(struct lcdapi_data *data, int cursor_x, int cursor_y)
{
    data->cursor_x = cursor_x;
    data->cursor_y = cursor_y;

    uint addr = cursor_x & 0x3f;
    
    if (cursor_y & 1)
        addr += 0x40;
    
    if (cursor_y & 2)
        addr += data->columns;

    LCD_HalWriteCommand(data, LCD_DDRAM | addr);
}

void LCD_PutChar(struct lcdapi_data *data, char character)
{
    if (character == '\n' && !data->implied_newline)
        data->cursor_x = data->columns;
    else
    {
        LCD_HalWriteData(data, character);
        data->cursor_x += 1;
    }

    if (data->cursor_x >= data->columns)
    {
        data->cursor_x = 0;
        data->cursor_y += 1;
        data->implied_newline = (character != '\n');
    }

    if (data->cursor_y >= data->lines)
        data->cursor_y = 0;

    LCD_MoveTo(data, data->cursor_x, data->cursor_y);
}

void LCD_PutStr(struct lcdapi_data *data, const char* string)
{
    for (int i = 0; i < StringSize(string); i++)
    {
        LCD_PutChar(data, string[i]);
    }
}

void LCD_HalWriteCommand(struct lcdapi_data *data, uint cmd)
{
    gpio_put(data->rs, 0);

    LCD_HalWrite4Bits(data, cmd >> 4);
    LCD_HalWrite4Bits(data, cmd);
}

void LCD_HalWriteData(struct lcdapi_data *ldata, uint data)
{
    gpio_put(ldata->rs, 1);

    LCD_HalWrite4Bits(ldata, data >> 4);
    LCD_HalWrite4Bits(ldata, data);
}

void LCD_HalWrite4Bits(struct lcdapi_data *data, uint nibble)
{
    gpio_put(data->d7, nibble & 0x08);
    gpio_put(data->d6, nibble & 0x04);
    gpio_put(data->d5, nibble & 0x02);
    gpio_put(data->d4, nibble & 0x01);
    
    LCD_HalPulseEnable(data);
}

void LCD_HalPulseEnable(struct lcdapi_data *data)
{
    gpio_put(data->enable, 0);
    sleep_us(1);
    gpio_put(data->enable, 1);
    sleep_us(1);
    gpio_put(data->enable, 0);
    sleep_us(100);
}

void LCD_HalWriteInitNibble(struct lcdapi_data *data, uint nibble)
{
    LCD_HalWrite4Bits(data, nibble >> 4);
}

int StringSize(const char * str)
{
    int Size = 0;
    while (str[Size] != '\0') Size++;
    return Size;
}

void LCD_Init(struct lcdapi_data *data)
{
    InitPin(data->rs);
    InitPin(data->enable);

    InitPin(data->d4);
    InitPin(data->d5);
    InitPin(data->d6);
    InitPin(data->d7);

    sleep_ms(20);

    LCD_HalWriteInitNibble(data, LCD_FUNCTION_RESET);
    sleep_ms(5);
    LCD_HalWriteInitNibble(data, LCD_FUNCTION_RESET);
    sleep_ms(1);
    LCD_HalWriteInitNibble(data, LCD_FUNCTION_RESET);
    sleep_ms(1);

    LCD_HalWriteInitNibble(data, LCD_FUNCTION);
    sleep_ms(1);

    if (data->lines > 4)
        data->lines = 4;
    
    if (data->columns > 40)
        data->columns = 40;

    data->cursor_x = 0;
    data->cursor_y = 0;
    data->implied_newline = false;

    LCD_DisplayOff(data);
    LCD_Clear(data);

    sleep_ms(1);
    LCD_HalWriteCommand(data, LCD_ENTRY_MODE | LCD_ENTRY_INC);
    sleep_ms(1);
    
    LCD_BlinkCursorOn(data);
    sleep_ms(1);

    LCD_HalWriteCommand(data, LCD_FUNCTION | LCD_FUNCTION_2LINES);
}