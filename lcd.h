/**
 * lcd.h
 * 
 * HD44780 20x4 LCD with I2C Interface
 * Driver for character LCD display via I2C backpack
 */

#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include <stdbool.h>

// I2C Configuration
#define LCD_I2C_PORT i2c0
#define LCD_SDA_PIN 12
#define LCD_SCL_PIN 13
#define LCD_I2C_ADDR 0x27  // Common I2C address, may need to be 0x3F

// Display dimensions
#define LCD_COLS 20
#define LCD_ROWS 4

/**
 * Initialize the LCD display
 * Sets up I2C communication and LCD configuration
 * 
 * @return true if initialization successful, false otherwise
 */
bool lcd_init(void);

/**
 * Clear the entire LCD display
 */
void lcd_clear(void);

/**
 * Set cursor position
 * 
 * @param col Column (0-19 for 20x4 display)
 * @param row Row (0-3 for 20x4 display)
 */
void lcd_set_cursor(uint8_t col, uint8_t row);

/**
 * Print a string at current cursor position
 * 
 * @param str Null-terminated string to print
 */
void lcd_print(const char *str);

/**
 * Print a formatted string at specific position
 * 
 * @param col Column (0-19)
 * @param row Row (0-3)
 * @param str Null-terminated string to print
 */
void lcd_print_at(uint8_t col, uint8_t row, const char *str);

/**
 * Turn backlight on/off
 * 
 * @param on true to turn backlight on, false to turn off
 */
void lcd_backlight(bool on);

#endif // LCD_H
