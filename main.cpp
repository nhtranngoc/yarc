#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "clock.h"
#include "console.h"
#include "lcd-spi.h"
#include "sdram.h"
#include "lcd-dma.h"

typedef char map_t;
#define MAP_WIDTH 	16
#define MAP_HEIGHT 	16

const uint16_t rect_w = LCD_WIDTH / MAP_WIDTH;
const uint16_t rect_h = LCD_WIDTH / MAP_HEIGHT;

layer1_pixel *const raycaster_canvas = (uint32_t *)SDRAM_BASE_ADDRESS;
const map_t map[] =    "0000222222220000"\
                       "1              0"\
                       "1      11111   0"\
                       "1     0        0"\
                       "0     0  1110000"\
                       "0     3        0"\
                       "0   10000      0"\
                       "0   0   11100  0"\
                       "0   0   0      0"\
                       "0   0   1  00000"\
                       "0       1      0"\
                       "2       1      0"\
                       "0       0      0"\
                       "0 0000000      0"\
                       "0              0"\
                       "0002222222200000";

void write_pixel(uint32_t *buffer, uint16_t x, uint16_t y, uint32_t c);
void draw_rectangle(uint32_t *buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t c);
void draw();

int main(void) {
	/* init timers. */
	clock_setup();

	/* set up USART 1. */
	console_setup(115200);
	console_stdio_setup();

	/* set up SDRAM. */
	sdram_init();

	/* set up LCD. */
	lcd_dma_init(raycaster_canvas);
	lcd_spi_init();

	draw(); 

	while (1) {
		
	}

	return 0;
}

void draw() {
	for(uint16_t j = 0; j < MAP_HEIGHT; j++) {
		for(uint16_t i = 0; i < MAP_WIDTH; i++) {
			if(map[i+j*MAP_WIDTH] != ' ') {
				uint16_t rect_x  = i * rect_w;
				uint16_t rect_y = j * rect_h;

				draw_rectangle(raycaster_canvas, rect_x, rect_y, rect_w, rect_h, GREEN);
			}
		}
	}
}

void write_pixel(uint32_t *buffer, uint16_t x, uint16_t y, uint32_t c) {
    // Due to how the on-board LCD is mapped, we want to flip the coordinates   
    auto pixel = x * LCD_WIDTH + y;

    buffer[pixel] = c;
}

void draw_rectangle(uint32_t *buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t c) {
	for(uint16_t i = 0; i < w; i++) {
		for(uint16_t j = 0; j < h; j++) {
			write_pixel(buffer, x + i, y + j, c);
		}
	}
}