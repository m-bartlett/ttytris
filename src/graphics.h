#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <ncurses.h>
#include "tetromino.h"


void graphics_init(void);
void graphics_clean(void);

void draw_tetromino_at_xy(WINDOW *w,
                          const tetromino_t *t,
                          const uint8_t X,
                          const uint8_t Y,
                          const char symbol);

void draw_tetromino_at_xy_noclip(WINDOW *w,
                                 const tetromino_t *t,
                                 const uint8_t X,
                                 const uint8_t Y,
                                 const char symbol);

void draw_playfield(void);
void draw_queue_preview(void);
void draw_score(void);
void draw_held_tetromino(void);
void draw_screen(void);
void draw_active_tetromino(void);
void draw_hard_drop_preview(void);
void draw_game(void);
void animate_line_kill(uint8_t Y);
void draw_debug(const char* msg);

#endif