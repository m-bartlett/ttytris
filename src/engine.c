#include <time.h>
#include <pthread.h>
#include <unistd.h>  // usleep
#include "engine.h"
#include "playfield.h"
#include "shuffle.h"
#include "graphics.h"


/* Wallkick values for the Super Rotation System
    https://tetris.fandom.com/wiki/SRS#Basic_Rotation */
static const int8_t WALLKICKS_JLTSZ[4][4][2] = { {{-1, 0}, {-1, 1}, {0,-2}, {-1,-2}},
                                                 {{ 1, 0}, { 1,-1}, {0, 2}, { 1, 2}},
                                                 {{ 1, 0}, { 1, 1}, {0,-2}, { 1,-2}},
                                                 {{-1, 0}, {-1,-1}, {0, 2}, {-1, 2}} };

static const int8_t WALLKICKS_I[4][4][2] = { {{-2, 0}, { 1, 0}, {-2,-1}, { 1, 2}},
                                             {{-1, 0}, { 2, 0}, {-1, 2}, { 2,-1}},
                                             {{ 2, 0}, {-1, 0}, { 2, 1}, {-1,-2}},
                                             {{ 1, 0}, {-2, 0}, { 1,-2}, {-2, 1}} };

static tetromino_t tetromino;
static uint8_t X, Y;
static int8_t Y_hard_drop = -1;
static tetromino_type_t held_tetromino = TETROMINO_TYPE_NULL;
static bool tetromino_swapped = false;
static uint8_t cleared_lines = 0;
static uint8_t level = 0;
static uint32_t fall_delay = ENGINE_GRAVITY_INITIAL_DELAY_us;
static bool game_loop = true;
static bool prevent_drop_lock = false;

static pthread_mutex_t mutex;
static pthread_t engine_gravity_thread;
static pthread_t engine_drop_lock_thread;


void engine_input_loop(void)
{ //{{{
    int input=0;

    while(1) {
        input = getch();

        pthread_mutex_lock(&mutex);

        switch(input) {
            case KEY_LEFT:  engine_move_active_tetromino(-1,0); break;
            case KEY_RIGHT: engine_move_active_tetromino(1,0);  break;
            case KEY_DOWN:  engine_move_active_tetromino(0,1);  break;
            case KEY_UP:
                engine_place_tetromino_at_xy(X, Y_hard_drop);
                break;
            case 's': engine_rotate_active_tetromino_counterclockwise();  break;
            case 'd': engine_rotate_active_tetromino_clockwise();  break;
            case 'r': engine_swap_hold(); break;
            case 'q': return;
            default:;
        }

        draw_game();

        pthread_mutex_unlock(&mutex);
    };
/*}}}*/ }


static void *engine_gravity_thread_task(void*)
{ //{{{
    while(game_loop) {
        usleep(fall_delay);
        pthread_mutex_lock(&mutex);
        engine_move_active_tetromino(0, 1);
        draw_game();
        pthread_mutex_unlock(&mutex);
    };
    return NULL;
/*}}}*/ }


static void *engine_drop_lock_thread_task(void*)
{ //{{{
    while(game_loop) {
        usleep(ENGINE_DROP_LOCK_DELAY_us);
        pthread_mutex_lock(&mutex);
        if (!playfield_validate_tetromino_placement(&tetromino, X, Y+1)) {
            if (!prevent_drop_lock) {
                prevent_drop_lock = true;
                engine_place_tetromino_at_xy(X,Y);
                draw_game();
            }
            else prevent_drop_lock = false;
        }
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
/*}}}*/ }



void engine_init()
{ //{{{
    bag_of_7_init(time(NULL));
    X = PLAYFIELD_SPAWN_X, Y = PLAYFIELD_SPAWN_Y;
    tetromino = (tetromino_t){ (tetromino_type_t)(bag_of_7_pop_sample()+1), 0 };
    pthread_mutex_init(&mutex, NULL);
    pthread_create(&engine_gravity_thread, NULL, &engine_gravity_thread_task, NULL);
    pthread_create(&engine_drop_lock_thread, NULL, &engine_drop_lock_thread_task, NULL);
    game_loop = 1;
/*}}}*/ }


void engine_clean()
{ //{{{
    game_loop = 0;
    pthread_cancel(engine_gravity_thread);
    pthread_cancel(engine_drop_lock_thread);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_join(engine_gravity_thread, NULL);
    pthread_join(engine_drop_lock_thread, NULL);
/*}}}*/ }


const tetromino_t* engine_get_active_tetromino() { return &tetromino; }


const tetromino_type_t engine_get_held_tetromino() { return held_tetromino; }


const uint8_t engine_get_level() { return level; }


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
    uint8_t lines = playfield_clear_lines(animate_line_kill);
    if (lines) {
        cleared_lines += lines;
        if (cleared_lines >= ENGINE_LINES_PER_LEVEL) {
            cleared_lines %= ENGINE_LINES_PER_LEVEL;
            if (level < ENGINE_MAX_LEVEL) {
                level++;
                fall_delay = (ENGINE_GRAVITY_INITIAL_DELAY_us 
                              - ((int32_t)ENGINE_GRAVITY_INITIAL_DELAY_us*level/11));
            }
        }
    }
    draw_score();
    draw_playfield();
    draw_queue_preview();
/*}}}*/}


void engine_rotate_active_tetromino_clockwise()  // Rotation with wallkicks
{ //{{{
    prevent_drop_lock = true;
    tetromino_rotate_clockwise(&tetromino);
    if (playfield_validate_tetromino_placement(&tetromino, X, Y)) return;

    uint8_t x, y;
    const int8_t (*wallkicks)[2];

    switch(tetromino.type) {
        case TETROMINO_TYPE_O:
            return;
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
            return;
        }
    }

    prevent_drop_lock = false;
    tetromino_rotate_counterclockwise(&tetromino); // undo rotation if no kicks are valid
/*}}}*/ }


void engine_rotate_active_tetromino_counterclockwise()  // Rotation with wallkicks
{ //{{{
    prevent_drop_lock = true;
    tetromino_rotate_counterclockwise(&tetromino);
    if (playfield_validate_tetromino_placement(&tetromino, X, Y)) return;

    uint8_t x, y;
    const int8_t (*wallkicks)[2];

    switch(tetromino.type) {
        case TETROMINO_TYPE_O:
            return;
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
            return;
        }
    }

    prevent_drop_lock = false;
    tetromino_rotate_clockwise(&tetromino); // undo rotation if no kicks are valid
/*}}}*/ }