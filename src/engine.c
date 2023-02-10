#include <time.h>
#include "engine.h"
#include "playfield.h"
#include "shuffle.h"
#include "graphics.h"


static tetromino_t tetromino;
static uint8_t X, Y;
static tetromino_type_t held_tetromino = TETROMINO_TYPE_NULL;
static bool tetromino_swapped = false;


const tetromino_t* engine_get_active_tetromino() { return &tetromino; }
const tetromino_type_t engine_get_held_tetromino() { return held_tetromino; }
const point_t engine_get_active_xy() { return (const point_t){X, Y}; }


const int8_t engine_get_hard_drop_y()
{ //{{{
    int8_t Y_harddrop = -1;
    int8_t _Y = Y;
    bool previous_valid = false;
    while(_Y < PLAYFIELD_HEIGHT1+2) {
        bool current_valid = playfield_validate_tetromino_placement(&tetromino, X, _Y);
        if (previous_valid && (!current_valid)) {
            if(_Y > Y) {
                Y_harddrop = _Y-1;
                break;
            }
        }
        previous_valid = current_valid;
        ++_Y;
    }
    return Y_harddrop;
/*}}}*/ }


void engine_move_active_tetromino(int8_t dx, uint8_t dy)
{ //{{{
    uint8_t _X=X+dx, _Y=Y+dy;
    if (playfield_validate_tetromino_placement(&tetromino, _X, _Y)) {
        X=_X;
        Y=_Y;
    }
/*}}}*/ }


void engine_swap_hold(void)
{ //{{{
    if (!tetromino_swapped) {
        tetromino_swapped = true;
        tetromino_type_t current = tetromino.type;
        if (held_tetromino == TETROMINO_TYPE_NULL) {
            held_tetromino = bag_of_7_pop_sample()+1;
            draw_queue_preview();
        }
        tetromino.type = held_tetromino;
        held_tetromino = current;
        tetromino.rotation = 0;
        X = PLAYFIELD_SPAWN_X;
        Y = PLAYFIELD_SPAWN_Y;
        draw_held_tetromino();
    }
/*}}}*/ }


void engine_place_tetromino_at_xy(uint8_t x, uint8_t y)
{ //{{{
    playfield_place_tetromino(&tetromino, x, y);
    tetromino_swapped = false;
    tetromino = (tetromino_t){ (tetromino_type_t)(bag_of_7_pop_sample()+1), 0 };
    X = PLAYFIELD_SPAWN_X;
    Y = PLAYFIELD_SPAWN_Y;
    draw_playfield();
    playfield_clear_lines(animate_line_kill);
    draw_queue_preview();
/*}}}*/}


void engine_rotate_active_tetromino_clockwise()  // Rotation with wallkicks
{ //{{{
    tetromino_rotate_clockwise(&tetromino);
    if (playfield_validate_tetromino_placement(&tetromino, X, Y)) return;
    uint8_t _X, _Y;
    if (
        playfield_validate_tetromino_placement(&tetromino, (_X=X+1), Y) ||
        playfield_validate_tetromino_placement(&tetromino, (_X=X-1), Y) ||
        playfield_validate_tetromino_placement(&tetromino, (_X=X+2), Y) ||
        playfield_validate_tetromino_placement(&tetromino, (_X=X-2), Y)
    ) { X=_X; return; }
    if (
        playfield_validate_tetromino_placement(&tetromino, X, (_Y=Y+1)) ||
        playfield_validate_tetromino_placement(&tetromino, X, (_Y=Y-1))
    ) { Y=_Y; return; }
    tetromino_rotate_counterclockwise(&tetromino); // undo rotation if no kicks are valid
/*}}}*/ }


void engine_rotate_active_tetromino_counterclockwise()  // Rotation with wallkicks
{ //{{{
    tetromino_rotate_counterclockwise(&tetromino);
    if (playfield_validate_tetromino_placement(&tetromino, X, Y)) return;
    uint8_t _X, _Y;
    if (
        playfield_validate_tetromino_placement(&tetromino, (_X=X-1), Y) ||
        playfield_validate_tetromino_placement(&tetromino, (_X=X+1), Y) ||
        playfield_validate_tetromino_placement(&tetromino, (_X=X-2), Y) ||
        playfield_validate_tetromino_placement(&tetromino, (_X=X+2), Y)
    ) { X=_X; return; }
    if (
        playfield_validate_tetromino_placement(&tetromino, X, (_Y=Y+1)) ||
        playfield_validate_tetromino_placement(&tetromino, X, (_Y=Y-1))
    ) { Y=_Y; return; }
    tetromino_rotate_clockwise(&tetromino); // undo rotation if no kicks are valid
/*}}}*/ }


void engine_init()
{ //{{{
    bag_of_7_init(time(NULL));
    X = PLAYFIELD_SPAWN_X, Y = PLAYFIELD_SPAWN_Y;
    tetromino = (tetromino_t){ (tetromino_type_t)(bag_of_7_pop_sample()+1), 0 };
/*}}}*/ }


void engine_clean() { return; }