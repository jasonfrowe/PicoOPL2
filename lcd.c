/**
 * lcd.c
 * 
 * HD44780 20x4 LCD with I2C Interface Implementation
 * Based on PCF8574 I2C backpack protocol
 */

#include "lcd.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>
#include <stdio.h>

// LCD Commands
#define LCD_CLEAR_DISPLAY 0x01
#define LCD_RETURN_HOME 0x02
#define LCD_ENTRY_MODE_SET 0x04
#define LCD_DISPLAY_CONTROL 0x08
#define LCD_CURSOR_SHIFT 0x10
#define LCD_FUNCTION_SET 0x20
#define LCD_SET_CGRAM_ADDR 0x40
#define LCD_SET_DDRAM_ADDR 0x80

// Display control flags
#define LCD_DISPLAY_ON 0x04
#define LCD_CURSOR_ON 0x02
#define LCD_BLINK_ON 0x01

// Function set flags
#define LCD_8BIT_MODE 0x10
#define LCD_4BIT_MODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// Entry mode flags
#define LCD_ENTRY_RIGHT 0x00
#define LCD_ENTRY_LEFT 0x02
#define LCD_ENTRY_SHIFT_INCREMENT 0x01
#define LCD_ENTRY_SHIFT_DECREMENT 0x00

// PCF8574 I2C LCD Backpack pin mapping
#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE 0x04  // Enable bit
#define LCD_RW 0x02      // Read/Write bit
#define LCD_RS 0x01      // Register select bit

// Row start addresses for 20x4 display
static const uint8_t row_offsets[4] = {0x00, 0x40, 0x14, 0x54};

// Current backlight state
static uint8_t backlight_state = LCD_BACKLIGHT;

// Write a byte to I2C
static void i2c_write_byte(uint8_t data) {
    i2c_write_blocking(LCD_I2C_PORT, LCD_I2C_ADDR, &data, 1, false);
}

// Send 4 bits to LCD via I2C
static void lcd_write_nibble(uint8_t nibble, uint8_t mode) {
    uint8_t data = (nibble & 0xF0) | mode | backlight_state;
    
    // Send data with enable high
    i2c_write_byte(data | LCD_ENABLE);
    sleep_us(1);
    
    // Send data with enable low (latch)
    i2c_write_byte(data & ~LCD_ENABLE);
    sleep_us(50);
}

// Send a byte to LCD (split into two 4-bit nibbles)
static void lcd_write_byte(uint8_t data, uint8_t mode) {
    lcd_write_nibble(data & 0xF0, mode);
    lcd_write_nibble((data << 4) & 0xF0, mode);
}

// Send command to LCD
static void lcd_command(uint8_t cmd) {
    lcd_write_byte(cmd, 0);  // mode = 0 for command
    if (cmd <= 0x02) {
        sleep_ms(2);  // Clear/home commands need more time
    }
}

// Send data (character) to LCD
static void lcd_data(uint8_t data) {
    lcd_write_byte(data, LCD_RS);  // mode = RS for data
}

bool lcd_init(void) {
    // Initialize I2C
    i2c_init(LCD_I2C_PORT, 100 * 1000);  // 100kHz
    gpio_set_function(LCD_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(LCD_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(LCD_SDA_PIN);
    gpio_pull_up(LCD_SCL_PIN);
    
    printf("LCD: Initializing I2C on SDA=%d, SCL=%d\n", LCD_SDA_PIN, LCD_SCL_PIN);
    
    // Wait for LCD to power up
    sleep_ms(50);
    
    // Initialize LCD in 4-bit mode (HD44780 initialization sequence)
    // Start in 8-bit mode
    lcd_write_nibble(0x30, 0);
    sleep_ms(5);
    lcd_write_nibble(0x30, 0);
    sleep_us(150);
    lcd_write_nibble(0x30, 0);
    sleep_us(150);
    
    // Switch to 4-bit mode
    lcd_write_nibble(0x20, 0);
    sleep_us(150);
    
    // Function set: 4-bit mode, 2 lines, 5x8 dots
    lcd_command(LCD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE | LCD_5x8DOTS);
    
    // Display control: display on, cursor off, blink off
    lcd_command(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON);
    
    // Clear display
    lcd_clear();
    
    // Entry mode: increment cursor, no shift
    lcd_command(LCD_ENTRY_MODE_SET | LCD_ENTRY_LEFT);
    
    printf("LCD: Initialization complete\n");
    return true;
}

void lcd_clear(void) {
    lcd_command(LCD_CLEAR_DISPLAY);
    sleep_ms(2);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    if (row >= LCD_ROWS) row = LCD_ROWS - 1;
    if (col >= LCD_COLS) col = LCD_COLS - 1;
    
    uint8_t address = col + row_offsets[row];
    lcd_command(LCD_SET_DDRAM_ADDR | address);
}

void lcd_print(const char *str) {
    while (*str) {
        lcd_data(*str++);
    }
}

void lcd_print_at(uint8_t col, uint8_t row, const char *str) {
    lcd_set_cursor(col, row);
    lcd_print(str);
}

void lcd_backlight(bool on) {
    backlight_state = on ? LCD_BACKLIGHT : 0x00;
    i2c_write_byte(backlight_state);
}
