#include "lcd_api.h"

LCDApi::LCDApi(LCDAPI_CONFIG config)
    : m_data(config)
{
    init_pin(m_data.m_rs);
    init_pin(m_data.m_enable);

    init_pin(m_data.m_d4);
    init_pin(m_data.m_d5);
    init_pin(m_data.m_d6);
    init_pin(m_data.m_d7);

    sleep_ms(20);

    hal_write_init_nibble(LCD_FUNCTION_RESET);
    sleep_ms(5);
    hal_write_init_nibble(LCD_FUNCTION_RESET);
    sleep_ms(1);
    hal_write_init_nibble(LCD_FUNCTION_RESET);
    sleep_ms(1);

    hal_write_init_nibble(LCD_FUNCTION);
    sleep_ms(1);

    if (m_data.m_lines > 4)
        m_data.m_lines = 4;
    
    if (m_data.m_columns > 40)
        m_data.m_columns = 40;

    m_data.m_cursor_x = 0;
    m_data.m_cursor_y = 0;
    implied_newline = false;

    display_off();
    clear(); 

    sleep_ms(1);
    hal_write_command(LCD_ENTRY_MODE | LCD_ENTRY_INC);
    sleep_ms(1);
    
    blink_cursor_on();
    sleep_ms(1);

    hal_write_command(LCD_FUNCTION | LCD_FUNCTION_2LINES);
}

void LCDApi::init_pin(uint pin)
{
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 0);
}

void LCDApi::clear()
{
    hal_write_command(LCD_CLR);
    sleep_ms(1);
    hal_write_command(LCD_HOME);
    sleep_ms(1);
    
    m_data.m_cursor_x = 0;
    m_data.m_cursor_y = 0;
}

void LCDApi::show_cursor()
{
    hal_write_command(LCD_ON_CTRL | LCD_ON_DISPLAY | LCD_ON_CURSOR);
}

void LCDApi::hide_cursor()
{
    hal_write_command(LCD_ON_CTRL | LCD_ON_DISPLAY);
}

void LCDApi::blink_cursor_off()
{
    hal_write_command(LCD_ON_CTRL | LCD_ON_DISPLAY | LCD_ON_CURSOR);
}

void LCDApi::blink_cursor_on()
{
    hal_write_command(LCD_ON_CTRL | LCD_ON_DISPLAY | LCD_ON_CURSOR | LCD_ON_BLINK);
}

void LCDApi::display_off()
{
    hal_write_command(LCD_ON_CTRL);
}

void LCDApi::display_on()
{
    hal_write_command(LCD_ON_CTRL | LCD_ON_DISPLAY);
}

void LCDApi::move_to(int cursor_x, int cursor_y)
{
    m_data.m_cursor_x = cursor_x;
    m_data.m_cursor_y = cursor_y;

    uint addr = cursor_x & 0x3f;
    
    if (cursor_y & 1)
        addr += 0x40;
    
    if (cursor_y & 2)
        addr += m_data.m_columns;

    hal_write_command(LCD_DDRAM | addr);
}

void LCDApi::putchar(char character)
{
    if (character == '\n' && !implied_newline)
        m_data.m_cursor_x = m_data.m_columns;
    else
    {
        hal_write_data(int(character));
        m_data.m_cursor_x += 1;
    }

    if (m_data.m_cursor_x >= m_data.m_columns)
    {
        m_data.m_cursor_x = 0;
        m_data.m_cursor_y += 1;
        implied_newline = (character != '\n');
    }

    if (m_data.m_cursor_y >= m_data.m_lines)
        m_data.m_cursor_y = 0;

    move_to(m_data.m_cursor_x, m_data.m_cursor_y);
}

void LCDApi::putstr(const char* string)
{
    for (int i = 0; i < string_size(string); i++)
    {
        putchar(string[i]);
    }
}

void LCDApi::hal_write_command(uint cmd)
{
    gpio_put(m_data.m_rs, 0);

    hal_write_4bits(cmd >> 4);
    hal_write_4bits(cmd);
}

void LCDApi::hal_write_data(uint data)
{
    gpio_put(m_data.m_rs, 1);

    hal_write_4bits(data >> 4);
    hal_write_4bits(data);
}

void LCDApi::hal_write_4bits(uint nibble)
{
    gpio_put(m_data.m_d7, nibble & 0x08);
    gpio_put(m_data.m_d6, nibble & 0x04);
    gpio_put(m_data.m_d5, nibble & 0x02);
    gpio_put(m_data.m_d4, nibble & 0x01);
    
    hal_pulse_enable();
}

void LCDApi::hal_pulse_enable()
{
    gpio_put(m_data.m_enable, 0);
    sleep_us(1);
    gpio_put(m_data.m_enable, 1);
    sleep_us(1);
    gpio_put(m_data.m_enable, 0);
    sleep_us(100);
}

void LCDApi::hal_write_init_nibble(uint nibble)
{
    hal_write_4bits(nibble >> 4);
}

int LCDApi::string_size(const char * str)
{
    int Size = 0;
    while (str[Size] != '\0') Size++;
    return Size;
}