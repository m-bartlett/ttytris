#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>
#include "tetromino.h"

typedef struct { const uint8_t x; const uint8_t y; } point_t;


const tetromino_t* engine_get_active_tetromino();
const tetromino_type_t engine_get_held_tetromino();
const point_t engine_get_active_xy();
const int8_t engine_get_hard_drop_y();

void engine_move_active_tetromino(int8_t dx, uint8_t dy);
void engine_swap_hold(void);
void engine_place_tetromino_at_xy(uint8_t x, uint8_t y);
void engine_rotate_active_tetromino_clockwise();
void engine_rotate_active_tetromino_counterclockwise();
void engine_init();
void engine_clean();

#endif