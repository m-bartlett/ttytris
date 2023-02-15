#include <time.h>
#include <sys/time.h>
#include <unistd.h>  // usleep

#include "engine.h"
#include "playfield.h"
#include "shuffle.h"
#include "graphics.h"
#include "scoring.h"
#include "timeutils.h"


/* Wallkick values for the Super Rotation System
    https://tetris.fandom.com/wiki/SRS#Basic_Rotation 

    NOTE: the sign of the Y value is opposite to what the Tetris wiki uses,
    i.e. ΔY > 0 is downward in this implementation, whereas it means upward in the wiki
    so the Y-values of the wallkicks below are negated relative to what the wiki shows.
*/
static const int8_t WALLKICKS_JLTSZ[4][4][2] = { {{-1, 0}, {-1,-1}, {0, 2}, {-1, 2}},
                                                 {{ 1, 0}, { 1, 1}, {0,-2}, { 1,-2}},
                                                 {{ 1, 0}, { 1,-1}, {0, 2}, { 1, 2}},
                                                 {{-1, 0}, {-1, 1}, {0,-2}, {-1,-2}} };

static const int8_t WALLKICKS_I[4][4][2] = { {{-2, 0}, { 1, 0}, {-2, 1}, { 1,-2}},
                                             {{-1, 0}, { 2, 0}, {-1,-2}, { 2, 1}},
                                             {{ 2, 0}, {-1, 0}, { 2,-1}, {-1, 2}},
                                             {{ 1, 0}, {-2, 0}, { 1, 2}, {-2,-1}} };

static tetromino_t tetromino;
static uint8_t X, Y;
static int8_t Y_hard_drop = -1;
static tetromino_type_t held_tetromino = TETROMINO_TYPE_NULL;
static bool tetromino_swapped = false;

static uint32_t gravity_delay = ENGINE_GRAVITY_INITIAL_DELAY_MICROSECONDS;
static timespec_t drop_lock_timer = {0};
static timespec_t gravity_timer;



static void engine_check_drop_lock();
static void engine_update_gravity();


void engine_input_loop(void)
{ //{{{
    int input=0;
    uint32_t elapsed_us=0;
    int32_t remaining_us=0;
    // clock_t loop_timer;
    uint32_t loop_timer;
    timespec_t start_time, end_time;

    while(1) {
        elapsed_us=0;
        remaining_us=0;

        timer_set_current_time(&start_time);

        input = wgetch(stdscr);

        switch(input) {
            case KEY_LEFT:  engine_move_active_tetromino(-1,0); break;
            case KEY_RIGHT: engine_move_active_tetromino(1,0);  break;
            case KEY_DOWN:  engine_soft_drop_tetromino(); break;
            case KEY_UP:    engine_hard_drop_tetromino(); break;
            case 's': engine_rotate_active_tetromino_counterclockwise();  break;
            case 'd': engine_rotate_active_tetromino_clockwise();  break;
            case 'r': engine_swap_hold(); break;
            case 'q': return;
            default:;
        }

        engine_update_gravity();
        engine_check_drop_lock();
        draw_game();

        timer_set_current_time(&end_time);
        elapsed_us = timer_get_elapsed_microseconds(&start_time, &end_time);

        remaining_us = ENGINE_MICROSECONDS_PER_FRAME - elapsed_us;
        if (remaining_us > 0) usleep(remaining_us);

        // draw_debug("%d\n%d", elapsed_us, remaining_us);
        if (!timer_is_null(&drop_lock_timer)) {
            uint32_t lock_countdown = timer_get_elapsed_microseconds(&drop_lock_timer, &end_time);
            lock_countdown = ENGINE_DROP_LOCK_DELAY_MICROSECONDS - lock_countdown;
            draw_debug("%d", lock_countdown/10000);
        }
    };
/*}}}*/ }


static void engine_check_drop_lock()
{ //{{{
    if (!timer_is_null(&drop_lock_timer)) {
        timespec_t now_time;
        timer_set_current_time(&now_time);
        uint32_t elapsed_us = timer_get_elapsed_microseconds(&drop_lock_timer, &now_time);
        if (elapsed_us >= ENGINE_DROP_LOCK_DELAY_MICROSECONDS) {
            timer_unset(&drop_lock_timer);
            if (!playfield_validate_tetromino_placement(&tetromino, X, Y+1)) {
                // Only lock the piece if it cannot proceed downward
                engine_place_tetromino_at_xy(X,Y);
                draw_game();
            }
        }
    }
/*}}}*/ }


static void engine_update_gravity()
{ //{{{
    timespec_t now_time; 
    timer_set_current_time(&now_time);

    uint32_t elapsed_us = timer_get_elapsed_microseconds(&gravity_timer, &now_time);
    // draw_debug("%lu\n%lu", elapsed_us, gravity_delay);
    if (elapsed_us >= gravity_delay) {
        if (!engine_move_active_tetromino(0,1) && timer_is_null(&drop_lock_timer)) {
            drop_lock_timer = now_time;
        }
        gravity_timer = now_time;
    }
/*}}}*/ }


void engine_init()
{ //{{{
    bag_of_7_init(time(NULL));
    X = PLAYFIELD_SPAWN_X, Y = PLAYFIELD_SPAWN_Y;
    tetromino = (tetromino_t){ (tetromino_type_t)(bag_of_7_pop_sample()+1), 0 };
    timer_set_current_time(&gravity_timer);
/*}}}*/ }


void engine_clean()
{ //{{{
    return;
/*}}}*/ }


const tetromino_t* engine_get_active_tetromino() { return &tetromino; }


const tetromino_type_t engine_get_held_tetromino() { return held_tetromino; }


const point_t engine_get_active_xy() { return (const point_t){X, Y}; }


const int8_t engine_update_hard_drop_y()
{ //{{{
    Y_hard_drop = -1;
    int8_t _Y = Y;
    bool previous_valid = false;
    while(_Y < PLAYFIELD_HEIGHT1+2) {
        bool current_valid = playfield_validate_tetromino_placement(&tetromino, X, _Y);
        if (previous_valid && (!current_valid)) {
            if(_Y > Y) {
                Y_hard_drop = _Y-1;
                break;
            }
        }
        previous_valid = current_valid;
        ++_Y;
    }
    return Y_hard_drop;
/*}}}*/ }


bool engine_move_active_tetromino(int8_t dx, uint8_t dy)
{ //{{{
    uint8_t _X=X+dx, _Y=Y+dy;
    if (playfield_validate_tetromino_placement(&tetromino, _X, _Y)) {
        X=_X;
        Y=_Y;
        return true;
    }
    return false;
/*}}}*/ }


void engine_hard_drop_tetromino()
{ //{{{
    if (Y_hard_drop > -1) {
        uint8_t drop_height = Y_hard_drop - Y;
        scoring_add_hard_drop(drop_height);
        engine_place_tetromino_at_xy(X, Y_hard_drop);
    }
/*}}}*/ }


void engine_soft_drop_tetromino()
{ //{{{
    if (!engine_move_active_tetromino(0,1)) {  // Pressing down against the ground locks the piece
        engine_place_tetromino_at_xy(X,Y);
    }
    timer_set_current_time(&gravity_timer); // Reset gravity timer to prevent double-down
    scoring_add_soft_drop();
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
    draw_playfield();
    tetromino_swapped = false;
    tetromino = (tetromino_t){ (tetromino_type_t)(bag_of_7_pop_sample()+1), 0 };
    uint8_t lines = playfield_clear_lines(animate_line_kill);
    uint8_t new_level = scoring_add_line_clears(lines);
    if (new_level) {
        gravity_delay = (ENGINE_GRAVITY_INITIAL_DELAY_MICROSECONDS 
                       - ((uint32_t)ENGINE_GRAVITY_INITIAL_DELAY_MICROSECONDS * new_level
                                                                       / SCORING_MAX_LEVEL));
    }
    X = PLAYFIELD_SPAWN_X;
    Y = PLAYFIELD_SPAWN_Y;
    draw_queue_preview();
/*}}}*/}


void engine_rotate_active_tetromino_clockwise()  // Rotation with wallkicks
{ //{{{
    tetromino_rotate_clockwise(&tetromino);

    if (playfield_validate_tetromino_placement(&tetromino, X, Y)) goto valid_exit;

    uint8_t x, y;
    const int8_t (*wallkicks)[2];

    switch(tetromino.type) {
        case TETROMINO_TYPE_O:  // O-piece cannot rotate, nothing to be done
            goto valid_exit;
        case TETROMINO_TYPE_I:
            wallkicks = WALLKICKS_I[tetromino.rotation]; break;
        default:
            wallkicks = WALLKICKS_JLTSZ[tetromino.rotation]; break;
    }

    for (uint8_t i = 0; i < 4; ++i) {
        x = X + wallkicks[i][0];
        y = Y + wallkicks[i][1];
        if (playfield_validate_tetromino_placement(&tetromino, x, y)) {
            X = x;
            Y = y;
            goto valid_exit;
        }
    }
    goto invalid_exit;

    valid_exit:
        timer_unset(&drop_lock_timer);  // valid rotations restart drop-lock timer
        return;

    invalid_exit:
        tetromino_rotate_clockwise(&tetromino); // undo rotation if no kicks are valid
        return;
/*}}}*/ }


void engine_rotate_active_tetromino_counterclockwise()  // Rotation with wallkicks
{ //{{{
    tetromino_rotate_counterclockwise(&tetromino);

    if (playfield_validate_tetromino_placement(&tetromino, X, Y)) goto valid_exit;

    uint8_t x, y;
    const int8_t (*wallkicks)[2];

    switch(tetromino.type) {
        case TETROMINO_TYPE_O:  // O-piece cannot rotate, nothing to be done
            goto valid_exit;
        case TETROMINO_TYPE_I:
            wallkicks = WALLKICKS_I[tetromino.rotation]; break;
        default:
            wallkicks = WALLKICKS_JLTSZ[tetromino.rotation]; break;
    }

    for (uint8_t i = 0; i < 4; ++i) {
        x = X - wallkicks[i][0];
        y = Y - wallkicks[i][1];
        if (playfield_validate_tetromino_placement(&tetromino, x, y)) {
            X = x;
            Y = y;
            goto valid_exit;
        }
    }
    goto invalid_exit;

valid_exit:
    timer_unset(&drop_lock_timer);  // valid rotations restart drop-lock timer
    return;

invalid_exit:
    tetromino_rotate_clockwise(&tetromino); // undo rotation if no kicks are valid
    return;
/*}}}*/ }