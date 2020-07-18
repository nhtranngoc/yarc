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

// #define STB_IMAGE_IMPLEMENTATION
// #define STBI_FAILURE_USERMSG
// #include "stb_image.h"

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

// std::vector<uint32_t> walltext;
// size_t walltext_size;
// size_t walltext_cnt;

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

void draw();
uint32_t map_to_color(map_t pixel);
// int load_texture(const char *filename, std::vector<uint32_t> &texture, size_t &text_size, size_t &text_cnt);

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

	// mutate_background_color();
	// move_sprite();
	draw();
	memcpy(raycaster_canvas, buffer, LCD_WIDTH * LCD_HEIGHT * 4);
	player.y += 0.03;
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
				draw_rectangle(buffer, LCD_WIDTH/2+i, LCD_HEIGHT/2-column_height/2, 1, column_height, map_to_color(pixel));
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

// int load_texture(const char *filename, std::vector<uint32_t> &texture, size_t &text_size, size_t &text_cnt) {
// 	int nchannels = -1, w, h;
// 	uint8_t *pixmap = stbi_load(filename, &w, &h, &nchannels, 0);

// 	if(!pixmap) {
// 		stbi_image_free(pixmap);
// 		return -1;
// 	}
// 	if(nchannels != 4) {
// 		stbi_image_free(pixmap);
// 		return -1;
// 	} 

// 	text_cnt = w/h;
// 	text_size = w/text_cnt;

// 	if(h * int(text_cnt) != w) {
// 		stbi_image_free(pixmap);
// 		return -1;
// 	}

// 	texture = std::vector<uint32_t>(w*h);
// 	for(int i = 0; i < w; i++) {
// 		for(int j = 0; j < h; j++) {
// 			uint8_t r = pixmap[(i*h+j)*4 + 0];
// 			uint8_t g = pixmap[(i*h+j)*4 + 1];
// 			uint8_t b = pixmap[(i*h+j)*4 + 2];
// 			uint8_t a = pixmap[(i*h+j)*4 + 3];

// 			texture[i*h+j] = (a << 24) | (r << 16) | (g << 8) | b;
// 		}
// 	}

// 	stbi_image_free(pixmap);
// 	return 0;
// }