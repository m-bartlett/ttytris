#include "playfield.h"
#include "graphics.h"
#include "engine.h"

int main(int argc, char *argv[]) {
    engine_init();    
    graphics_init();

    int input=0;

    do {
        const point_t p = engine_get_active_xy();

        switch(input) {
            case KEY_LEFT:  engine_move_active_tetromino(-1,0); break;
            case KEY_RIGHT: engine_move_active_tetromino(1,0);  break;
            case KEY_DOWN:  engine_move_active_tetromino(0,1);  break;
            case KEY_UP:    engine_place_tetromino_at_xy(p.x, engine_get_hard_drop_y()); break;
            case 's': engine_rotate_active_tetromino_counterclockwise();  break;
            case 'd': engine_rotate_active_tetromino_clockwise();  break;
            case ' ': engine_place_tetromino_at_xy(p.x, p.y); break;
            case 'r': engine_swap_hold(); break;
            default:;
        }

        draw_game();

    } while((input = getch()) != 'q');

    graphics_clean();
    engine_clean();
    return 0;
}
