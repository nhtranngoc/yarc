#include "texture.h"

std::vector<uint32_t> Texture::GetColumnScaled(const uint16_t texture_id, const uint16_t tex_coord, const uint16_t column_height) {
    std::vector<uint32_t> column(column_height);
    for( uint16_t y = 0; y < column_height; y++) {
        uint16_t pix_x = texture_id * this->h + tex_coord;
        uint16_t pix_y = (y * this->h) / column_height;

        column[y] = this->Get(pix_x, pix_y);
    }

    return column;
}