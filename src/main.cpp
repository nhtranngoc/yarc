#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "clock.h"
#include "console.h"
#include "lcd-spi.h"
#include "sdram.h"
#include "lcd-dma.h"
#include "vector"
#include "walltext.h"

#include <math.h>

typedef char map_t;
#define MAP_WIDTH 	16
#define MAP_HEIGHT 	16
#define MAX_TEXTURE_SIZE 20480
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
                       "0   3   11100  0"\
                       "5   4   0      0"\
                       "5   4   1  00000"\
                       "0       1      0"\
                       "2       1      0"\
                       "0       0      0"\
                       "0 0000000      0"\
                       "0              0"\
                       "0002222222200000";

void draw();
uint32_t map_to_color(map_t pixel);
std::vector<uint32_t> texture_column(tImage *texture, const size_t texid, const size_t texcoord, const size_t column_height);

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

	player.x = 3.456f;
	player.y = 2.345f;
	player.a = PI/2;

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

void lcd_tft_isr(void) {
	LTDC_ICR |= LTDC_ICR_CRRIF;

	draw();
	memcpy(raycaster_canvas, buffer, LCD_WIDTH * LCD_HEIGHT * 4);
	// player.y += 0.03;
	player.a += 0.05;
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

	// Draw cone of vision
	for(uint16_t i = 0; i < LCD_WIDTH/2; i++) {
		float angle = player.a - FOV/2 + FOV * i / float(LCD_WIDTH/2);
		for(float c = 0; c < 20; c+=.05) {
			float cx = player.x + c * cosf(angle);
			float cy = player.y + c * sinf(angle);

			uint16_t pix_x = (uint16_t) cx * rect_w;
			uint16_t pix_y = (uint16_t) cy * rect_h;
			write_pixel(buffer, pix_x, pix_y, SILVER);

			pixel = map[int(cx) + int(cy) * MAP_WIDTH];
			if(pixel != ' ') {
				// Our ray intersects a wall, so let's render it
				uint16_t column_height = (uint16_t) (LCD_HEIGHT/(c*cosf(angle-player.a)));
				size_t texid = pixel - '0';

				float hitx = cx - floor(cx + .5f);
				float hity = cy - floor(cy + .5f);
				uint16_t x_texcoord = hitx * walltext.height;

				if(std::abs(hity) > std::abs(hitx)) {
					x_texcoord = hity * walltext.height;
				}

				if(x_texcoord < 0) {
					x_texcoord += walltext.height;
				}

				std::vector<uint32_t> column = texture_column(&walltext, texid, x_texcoord, column_height);
				pix_x = LCD_WIDTH/2+i;
				
				for(size_t j = 0; j < column_height; j++) {
					pix_y = j + LCD_HEIGHT/2 - column_height/2;
					write_pixel(buffer, pix_x, pix_y, column[j]);
				}

				break;
			}
		}
	}

	const size_t texid = 4; // draw the 4th texture on the screen
	const size_t walltext_cnt = walltext.width/walltext.height;	
	for(size_t i = 0; i < walltext.height; i++) {
		for(size_t j = 0; j < walltext.height; j++) {
			write_pixel(buffer, i, j, walltext.data[i+texid*walltext.height + j*walltext.height*walltext_cnt]);
		}
	}

}

uint32_t map_to_color(map_t square) {
	size_t texid = square - '0'; // who needs parseInt()
	return walltext.data[walltext.height*texid];
}

std::vector<uint32_t> texture_column(tImage *texture, const size_t texid, const size_t texcoord, const size_t column_height) {
	std::vector<uint32_t> column(column_height);
	for(size_t y = 0; y < column_height; y++) {
		size_t pix_x = texid * texture->height + texcoord;
		size_t pix_y = (y * texture->height)/ column_height;

		column[y] = texture->data[pix_x + pix_y * texture->width];
	}

	return column;
}