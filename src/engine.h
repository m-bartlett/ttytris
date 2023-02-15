#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>
#include <stdbool.h>
#include "tetromino.h"

#define ENGINE_DROP_LOCK_DELAY_MICROSECONDS 500000
#define ENGINE_GRAVITY_INITIAL_DELAY_MICROSECONDS 700000
#define ENGINE_FRAMES_PER_SECOND 30
#define ENGINE_MICROSECONDS_PER_FRAME (1000000/ENGINE_FRAMES_PER_SECOND)

typedef struct { const uint8_t x; const uint8_t y; } point_t;


const tetromino_t* engine_get_active_tetromino();
const tetromino_type_t engine_get_held_tetromino();
const point_t engine_get_active_xy();
const int8_t engine_update_hard_drop_y();

void engine_init();
void engine_clean();
void engine_input_loop(void);
bool engine_move_active_tetromino(int8_t dx, uint8_t dy);
void engine_swap_hold(void);
void engine_place_tetromino_at_xy(uint8_t x, uint8_t y);
void engine_rotate_active_tetromino_clockwise();
void engine_rotate_active_tetromino_counterclockwise();
void engine_hard_drop_tetromino();
void engine_soft_drop_tetromino();

#endif