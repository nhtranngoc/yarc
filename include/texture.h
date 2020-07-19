#pragma once

#include "stdint.h"

class Texture {
    public:
    const uint32_t *data;
    uint16_t w,h, size;
    uint8_t count;

    Texture(const uint32_t *data_, const uint16_t &w_, const uint16_t &h_, const uint16_t &size_) :
        data(data_),
        w(w_),
        h(h_),
        size(size_),
        count(w_/h_) {}
};