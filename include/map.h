#pragma once

#include <stdint.h>

#define MAP_WIDTH 	16
#define MAP_HEIGHT 	16

typedef uint8_t map_t;
static const map_t map_data[] =    "0000222222220000"\
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

class Map {
    public:
    uint16_t w, h;
    Map() : w(MAP_WIDTH), h(MAP_HEIGHT) {}
    inline uint32_t get(const uint16_t x, const uint16_t y) {return map_data[x+y*w] - '0';}
    inline bool isEmpty(const uint16_t x, const uint16_t y) {return map_data[x+y*w] == ' ';}
};