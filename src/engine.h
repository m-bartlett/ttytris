#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>
#include <stdbool.h>
#include "tetromino.h"

#define ENGINE_DROP_LOCK_DELAY_us 500000
#define ENGINE_GRAVITY_INITIAL_DELAY_us 750000
#define ENGINE_LINES_PER_LEVEL 10
#define ENGINE_MAX_LEVEL 15

typedef struct { const uint8_t x; const uint8_t y; } point_t;


const tetromino_t* engine_get_active_tetromino();
const tetromino_type_t engine_get_held_tetromino();
const point_t engine_get_active_xy();
const int8_t engine_update_hard_drop_y();
const uint8_t engine_get_level();

void engine_init();
void engine_clean();
void engine_input_loop(void);
bool engine_move_active_tetromino(int8_t dx, uint8_t dy);
void engine_swap_hold(void);
void engine_place_tetromino_at_xy(uint8_t x, uint8_t y);
void engine_rotate_active_tetromino_clockwise();
void engine_rotate_active_tetromino_counterclockwise();

#endif