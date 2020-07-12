#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "clock.h"
#include "console.h"
#include "lcd-spi.h"
#include "sdram.h"
#include "lcd-dma.h"

layer1_pixel *const raycaster_canvas = (uint32_t *)SDRAM_BASE_ADDRESS;

void write_pixel(uint32_t *buffer, uint32_t x, uint32_t y, uint32_t c);

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

	write_pixel(raycaster_canvas, 50, 50, TEAL);

	while (1) {
		
	}

	return 0;
}


void write_pixel(uint32_t *buffer, uint32_t x, uint32_t y, uint32_t c) {
    // Due to how the on-board LCD is mapped, we want to flip the coordinates   
    auto pixel = x * LCD_WIDTH + y;

    buffer[pixel] = c;
}