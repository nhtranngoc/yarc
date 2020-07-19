#include <vector>
#include <math.h>

#include "clock.h"
#include "console.h"
#include "lcd-spi.h"
#include "sdram.h"
#include "lcd-dma.h"

#include "map.h"
#include "texture.h"
#include "player.h"

#include "walltext.h"

#define PI 3.1415f
#define FOV PI/3.f

Map map;
Player player;

const uint16_t rect_w = LCD_WIDTH / (MAP_WIDTH);
const uint16_t rect_h = LCD_HEIGHT / MAP_HEIGHT;

layer1_pixel *const raycaster_canvas = (uint32_t *)SDRAM_BASE_ADDRESS;
layer1_pixel *const buffer = (uint32_t *)SDRAM_BASE_ADDRESS + (LCD_HEIGHT * LCD_WIDTH * sizeof(layer1_pixel));

void draw();
uint16_t wall_x_texcoord(const float x, const float y, Texture &texture);

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

// Takes in drawing coordinates (x,y) and texture,
// Returns x-coordinate of the texture
uint16_t wall_x_texcoord(const float x, const float y, Texture &texture) {
	float hitx = x - floor(x + .5f); // hitx and hity contain (signed) fractional parts of x and y
	float hity = y - floor(y + .5f); // they vary between -0.5 and +0.5, and one of them is supposed to be very close to 0
	uint16_t tex = hitx * walltext.h;

	if(std::abs(hity) > std::abs(hitx)) { // we need to determine whether we hit a "vertical" or a "horizontal" wall
		tex = hity * texture.h;
	}

	if(tex < 0) { // x_texcoord can be negative, let's fix that
		tex += texture.h;
	}

	return tex;
}

void draw() {
	fill(buffer, WHITE);

	// Draw map
	// for(uint16_t j = 0; j < MAP_HEIGHT; j++) {
	// 	for(uint16_t i = 0; i < MAP_WIDTH; i++) {
	// 		if(!map.isEmpty(i,j)) {
	// 			uint16_t rect_x  = i * rect_w;
	// 			uint16_t rect_y = j * rect_h;

	// 			size_t texid = map.get(i,j);

	// 			draw_rectangle(buffer, rect_x, rect_y, rect_w, rect_h, walltext.data[walltext.h * texid]);
	// 		}
	// 	}
	// }

	// Draw cone of vision
	for(uint16_t i = 0; i < LCD_WIDTH; i++) {
		float angle = player.a - FOV/2 + FOV * i / float(LCD_WIDTH);
		for(float c = 0; c < 20; c+=.05) {
			float cx = player.x + c * cosf(angle);
			float cy = player.y + c * sinf(angle);

			uint16_t pix_x = (uint16_t) cx * rect_w;
			uint16_t pix_y = (uint16_t) cy * rect_h;
			// write_pixel(buffer, pix_x, pix_y, RED);

			if(!map.isEmpty(cx, cy)) {
				// Our ray intersects a wall, so let's render it
				uint16_t column_height = (uint16_t) (LCD_HEIGHT/(c*cosf(angle-player.a)));
				size_t texid = map.get(cx, cy);

				uint16_t x_texcoord = wall_x_texcoord(cx, cy, walltext);

				std::vector<uint32_t> column = walltext.GetColumnScaled(texid, x_texcoord, column_height);
				pix_x = i;
				
				for(size_t j = 0; j < column_height; j++) {
					pix_y = j + LCD_HEIGHT/2 - column_height/2;
					if(pix_y >= 0 && pix_y < (int) LCD_HEIGHT) {
						write_pixel(buffer, pix_x, pix_y, column[j]);
					}
				}

				break;
			}
		}
	}

}