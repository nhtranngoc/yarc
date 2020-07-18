#ifndef LCD_DMA_H
#define LCD_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/ltdc.h>

#include "clock.h"
#include "console.h"
#include "lcd-spi.h"
#include "sdram.h"

#define LCD_WIDTH  240
#define LCD_HEIGHT 320
#define REFRESH_RATE 60 /* Hz */

#define HSYNC       10
#define HBP         20
#define HFP         10

#define VSYNC        2
#define VBP          2
#define VFP          4

// Colors
#define WHITE	0xffffffff
#define BLACK 	0xff000000
#define RED		0xffff0000
#define LIME 	0xff00ff00
#define BLUE	0xff0000ff
#define YELLOW  0xffffff00
#define CYAN    0xff00ffff
#define MAGENTA 0xffff00ff
#define SILVER  0xffc0c0c0
#define GRAY    0xff808080
#define MAROON  0xff800000
#define OLIVE   0xff808000
#define GREEN   0xff008000
#define PURPLE  0xff800080
#define TEAL    0xff008080
#define NAVY    0xff000080

/* Layer 1 (bottom layer) is ARGB8888 format, full screen. */

typedef uint32_t layer1_pixel;
#define LCD_LAYER1_PIXFORMAT LTDC_LxPFCR_ARGB8888

#define LCD_LAYER1_PIXEL_SIZE (sizeof(layer1_pixel))
#define LCD_LAYER1_WIDTH  LCD_WIDTH
#define LCD_LAYER1_HEIGHT LCD_HEIGHT
#define LCD_LAYER1_PIXELS (LCD_LAYER1_WIDTH * LCD_LAYER1_HEIGHT)
#define LCD_LAYER1_BYTES  (LCD_LAYER1_PIXELS * LCD_LAYER1_PIXEL_SIZE)

extern void lcd_dma_init(layer1_pixel *canvas);

void write_pixel(uint32_t *buffer, uint16_t x, uint16_t y, uint32_t c);
void draw_rectangle(uint32_t *buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t c);
// void draw_line(uint32_t *buffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t c);
void fill(uint32_t *buffer, uint32_t c);

#ifdef __cplusplus
}
#endif

#endif // LCD_DMA_H_