#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "clock.h"
#include "console.h"
#include "lcd-spi.h"
#include "sdram.h"
#include "lcd-dma.h"

#include <math.h>

typedef char map_t;
#define MAP_WIDTH 	16
#define MAP_HEIGHT 	16
#define PI 3.1415
#define FOV PI/3.f

typedef struct Player_ {
	float x;
	float y;
	float a;
} Player;

const uint16_t rect_w = LCD_WIDTH / (MAP_WIDTH*2);
const uint16_t rect_h = LCD_WIDTH / MAP_HEIGHT;

Player player;

layer1_pixel *const raycaster_canvas = (uint32_t *)SDRAM_BASE_ADDRESS;
layer1_pixel *const buffer = (uint32_t *)SDRAM_BASE_ADDRESS + (LCD_HEIGHT * LCD_WIDTH * sizeof(layer1_pixel));

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
void fill(uint32_t *buffer, uint32_t c);
uint32_t map_to_color(map_t pixel);

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

	player.a = 0;

	// buffer = (layer1_pixel *) malloc(sizeof(layer1_pixel) * LCD_WIDTH * LCD_HEIGHT);
	fill(buffer, WHITE);

	fill(raycaster_canvas, GREEN);

	while (1) {
		continue;
	}

	return 0;
}

/*
 * Here is where all the work is done.  We poke a total of 6 registers
 * for each frame.
 */

void lcd_tft_isr(void)
{
	LTDC_ICR |= LTDC_ICR_CRRIF;

	// mutate_background_color();
	// move_sprite();
	draw();
	memcpy(raycaster_canvas, buffer, LCD_WIDTH * LCD_HEIGHT * 4);
	player.a += 0.03;
	if(player.a > 2*PI) {
		player.a = 0;
	}

	LTDC_SRCR |= LTDC_SRCR_VBR;
}

void draw() {
	fill(buffer, WHITE);
	map_t pixel;
	// Draw map
	for(uint16_t j = 0; j < MAP_HEIGHT; j++) {
		for(uint16_t i = 0; i < MAP_WIDTH; i++) {
			pixel = map[i+j*MAP_WIDTH];
			if(pixel != ' ') {
				uint16_t rect_x  = i * rect_w;
				uint16_t rect_y = j * rect_h;

				draw_rectangle(buffer, rect_x, rect_y, rect_w, rect_h, map_to_color(pixel));
			}
		}
	}

	// Draw player
	player.x = 3.456f;
	player.y = 2.345f;

	// Draw cone of vision
	for(uint16_t i = 0; i < LCD_WIDTH/2; i++) {
		float angle = player.a - FOV/2 + FOV * i / float(LCD_WIDTH/2);
		for(float c = 0; c < 20; c+=.05) {
			float cx = player.x + c * cosf(angle);
			float cy = player.y + c * sinf(angle);

			uint16_t pix_x = cx * rect_w;
			uint16_t pix_y = cy * rect_h;
			write_pixel(buffer, pix_x, pix_y, SILVER);

			pixel = map[int(cx) + int(cy) * MAP_WIDTH];
			if(pixel != ' ') {
				// Our ray intersects a wall, so let's render it
				uint16_t column_height = LCD_HEIGHT/(c*cosf(angle-player.a));
				draw_rectangle(buffer, LCD_WIDTH/2+i, LCD_HEIGHT/2-column_height/2, 1, column_height, map_to_color(pixel));
				break;
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

void fill(uint32_t *buffer, uint32_t c) {
	for(uint16_t i = 0; i < LCD_WIDTH; i++) {
		for(uint16_t j = 0; j < LCD_HEIGHT; j++) {
			buffer[j+i*LCD_HEIGHT] = c;
		}
	}
}

uint32_t map_to_color(map_t pixel) {
	switch(pixel) {
		case '0': return GRAY;
		case '1': return MAROON;
		case '2': return NAVY;
		case '3': return PURPLE;
	}

	// default case
	return WHITE;
}