#include <unistd.h>     // STDOUTFILENO, usleep
#include "graphics.h"
#include "playfield.h"
#include "shuffle.h"
#include "engine.h"
#include "scoring.h"


static WINDOW *root_window;
static WINDOW *playfield_window;
static WINDOW *preview_window;
static WINDOW *hold_window;
static WINDOW *score_window;
static WINDOW *debug_window;

#define ANSI_BLACK   0
#define ANSI_RED     1
#define ANSI_GREEN   2
#define ANSI_YELLOW  3
#define ANSI_BLUE    4
#define ANSI_MAGENTA 5
#define ANSI_CYAN    6
#define ANSI_WHITE   7

#define FRAME_DELAY_us 15000

#define TETROMINO_QUEUE_PREVIEW_QUANTITY 6
#define TETROMINO_QUEUE_PREVIEW_HEIGHT (TETROMINO_QUEUE_PREVIEW_QUANTITY*3)


static const uint8_t TETROMINO_ANSI_COLORS[] = {ANSI_BLACK, ANSI_WHITE, ANSI_YELLOW, ANSI_MAGENTA,
                                                ANSI_BLUE, ANSI_CYAN, ANSI_GREEN, ANSI_RED};

void draw_tetromino_at_xy(WINDOW *w,
                          const tetromino_t *t,
                          const uint8_t X,
                          const uint8_t Y,
                          const char symbol)
{ //{{{
    uint16_t grid = tetromino_get_grid(t);
    uint16_t maskbit = (uint16_t)1<<15;
    const uint8_t X1=X+1, Y1=Y+1;

    for (int8_t y = Y-3; y < Y1; ++y) {
        if (y < 0 || y > PLAYFIELD_HEIGHT) {
            maskbit>>=4;
            continue;
        }
        for (int8_t x = X-3; x < X1; ++x) {
            if (!(x < 0 || x > PLAYFIELD_WIDTH) && (maskbit&grid)) {
                mvwaddch(w, y, x, symbol);
            }
            maskbit >>=1;
        }
    }
/*}}}*/ }


void draw_tetromino_at_xy_noclip(WINDOW *w,
                                 const tetromino_t *t,
                                 const uint8_t X,
                                 const uint8_t Y,
                                 const char symbol)
{ //{{{
    uint16_t grid = tetromino_get_grid(t);
    uint16_t maskbit = (uint16_t)1<<15;
    const uint8_t X1=X+1, Y1=Y+1;
    for (int8_t y = Y-3; y < Y1; ++y) {
        for (int8_t x = X-3; x < X1; ++x) {
            if (maskbit&grid) mvwaddch(w, y, x, symbol);
            maskbit >>=1;
        }
    }
/*}}}*/ }


void draw_playfield(void)
{ //{{{
    playfield_view_t playfield = playfield_view(); 
    wmove(playfield_window, 0, 0);
    for (int y = 0; y < PLAYFIELD_HEIGHT; ++y) {
        for (int x = 0; x < PLAYFIELD_WIDTH; ++x) {
            const char block = playfield[y][x];
            // char symbol = tetromino_type_t2char((tetromino_type_t)block);
            // if (symbol == '\0') symbol = ' ';
            const char symbol = ' ';
            const uint8_t color = TETROMINO_ANSI_COLORS[block];
            wattron(playfield_window, COLOR_PAIR(color));
            wprintw(playfield_window, "%c", symbol);
            wattroff(playfield_window, COLOR_PAIR(color));
        }
    }
/*}}}*/ }


void draw_queue_preview(void)
{ //{{{
    uint8_t queue[TETROMINO_QUEUE_PREVIEW_QUANTITY];
    bag_of_7_write_queue(queue, TETROMINO_QUEUE_PREVIEW_QUANTITY);
    wclear(preview_window);
    box(preview_window, 0, 0);
    for (uint8_t i = 0; i < TETROMINO_QUEUE_PREVIEW_QUANTITY; ++i) {
        uint8_t y=(i+1)*3, x=4;
        tetromino_t t = {queue[i] + 1, 0};
        const uint8_t color = TETROMINO_ANSI_COLORS[t.type];
        wattron(preview_window, COLOR_PAIR(color));
        draw_tetromino_at_xy_noclip(preview_window, &t, x, y, ' ');
        wattroff(preview_window, COLOR_PAIR(color));
    }
    wrefresh(preview_window);
/*}}}*/ }


void draw_held_tetromino(void)
{ //{{{
    wclear(hold_window);
    box(hold_window, 0, 0);
    const tetromino_type_t held_tetromino_type = engine_get_held_tetromino();
    const tetromino_t t = {held_tetromino_type, 0};
    const uint8_t color = TETROMINO_ANSI_COLORS[held_tetromino_type];
    wattron(hold_window, COLOR_PAIR(color));
    draw_tetromino_at_xy_noclip(hold_window, &t, 4, 4, ' ');
    wattroff(hold_window, COLOR_PAIR(color));
    wrefresh(hold_window);
/*}}}*/ }


void draw_active_tetromino(void)
{ //{{{
    const point_t p = engine_get_active_xy();
    const tetromino_t* tetromino = engine_get_active_tetromino();
    const uint8_t color = TETROMINO_ANSI_COLORS[tetromino->type];
    // const char symbol = tetromino_get_type_char(tetromino);
    const char symbol = ' ';
    wattron(playfield_window, COLOR_PAIR(color));
    draw_tetromino_at_xy(playfield_window, tetromino, p.x, p.y, symbol);
    wattroff(playfield_window, COLOR_PAIR(color));
/*}}}*/ }


void draw_hard_drop_preview(void)
{ //{{{
    int8_t Y_harddrop = engine_update_hard_drop_y();
    if (Y_harddrop > -1) {
        const point_t p = engine_get_active_xy();
        const tetromino_t* tetromino = engine_get_active_tetromino();
        draw_tetromino_at_xy(playfield_window, tetromino, p.x, Y_harddrop, '.');
    }
/*}}}*/ }


void draw_score(void)
{ //{{{
    wclear(score_window);
    wattron(score_window, A_UNDERLINE);
    mvwaddstr(score_window, 0, 0, "LEVEL");
    mvwaddstr(score_window, 0, 8, "SCORE");
    mvwaddstr(score_window, 0, 17, "LINES");
    wattroff(score_window, A_UNDERLINE);
    mvwprintw(score_window, 1, 0,
              " % 3d   %07d      %d",
              scoring_get_level()+1, scoring_get_score(), scoring_get_cleared_lines());
    wrefresh(score_window);
/*}}}*/ }


void draw_debug(const char* format, ...)
{ //{{{
    va_list args;
    va_start(args, format);

    char body[100];
    vsprintf(body, format, args);
    va_end(args);

    wclear(debug_window);
    // mvwprintw(debug_window, 0, 0, "% 60s", "");
    mvwaddstr(debug_window, 0, 0, body);
    wrefresh(debug_window);
/*}}}*/ }


void draw_game(void)
{ //{{{
    draw_playfield();
    draw_hard_drop_preview();
    draw_active_tetromino();
    wrefresh(playfield_window);
/*}}}*/ }


void animate_line_kill(uint8_t Y)
{ //{{{
    const char symbol = ' ';
    for (uint8_t x = 0; x < PLAYFIELD_WIDTH; ++x) {
        mvwaddch(playfield_window, Y, x, symbol);
        usleep(FRAME_DELAY_us);
        wrefresh(playfield_window);
    }
/*}}}*/ }


void graphics_init(void)
{ //{{{

    /* Initialize ncurses in alternate scrollback */
    initscr(); /* start curses mode */
    cbreak();  /* character input accepted immediatley */
    noecho();  /* don't print input characters */
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); // Turn on non-blocking mode

    // TODO: Handle SIGWINCH and readjust accordingly
    uint16_t height, width;
    getmaxyx(stdscr, height, width);
    const uint8_t X_offset = (width>>1) - (PLAYFIELD_WIDTH/2),
                  Y_offset = (height>>1) - (PLAYFIELD_HEIGHT/2);


    /* Initialize ncurses colors */
    start_color();
    init_pair(ANSI_BLACK,   A_NORMAL, COLOR_BLACK);
    init_pair(ANSI_RED,     A_NORMAL, COLOR_RED);
    init_pair(ANSI_GREEN,   A_NORMAL, COLOR_GREEN);
    init_pair(ANSI_YELLOW,  A_NORMAL, COLOR_YELLOW);
    init_pair(ANSI_BLUE,    A_NORMAL, COLOR_BLUE);
    init_pair(ANSI_MAGENTA, A_NORMAL, COLOR_MAGENTA);
    init_pair(ANSI_CYAN,    A_NORMAL, COLOR_CYAN);
    init_pair(ANSI_WHITE,   A_NORMAL, COLOR_WHITE);

    /* Initialize ncurses windows */
    root_window = newwin(PLAYFIELD_HEIGHT+2, PLAYFIELD_WIDTH+2, Y_offset, X_offset);
    playfield_window = derwin(root_window, PLAYFIELD_HEIGHT, PLAYFIELD_WIDTH, 1, 1);
    preview_window = newwin(TETROMINO_QUEUE_PREVIEW_HEIGHT+1,6,Y_offset,X_offset+PLAYFIELD_WIDTH+2);
    hold_window = newwin(5, 6, Y_offset, X_offset-(PLAYFIELD_WIDTH/2)-1);
    score_window = newwin(2, PLAYFIELD_WIDTH*2+3, Y_offset-2, X_offset-(PLAYFIELD_WIDTH/2));
    debug_window = newwin(10, 6, Y_offset+6, X_offset-(PLAYFIELD_WIDTH/2)-1);

    /* Must refresh root window before drawing to subwindows */
    refresh();
    box(root_window, 0, 0);
    wrefresh(root_window);
    draw_queue_preview();
    draw_held_tetromino();
    draw_score();
    draw_debug("");
    wrefresh(playfield_window);

/*}}}*/ }


void graphics_clean(void) { endwin(); }