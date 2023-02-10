#include <stdlib.h>
#include "test.h"
#include "../src/tetromino.h"

void test_tetromino_copy_and_rotate() { //{{{
    for (int ti = TETROMINO_TYPE_NULL + 1; ti < TETROMINO_TYPE_QUANTITY; ++ti) {
        tetromino_t t = {(tetromino_type_t)ti, 0};

        const char type_char = tetromino_get_type_char(&t);

        uint16_t grid,
                 expected_grid,
                 expected_grid_rotation90,
                 expected_grid_rotation180,
                 expected_grid_rotation270;

        switch(t.type) {
            default:
            case I:
                expected_grid = 3840;
                expected_grid_rotation90 = 8738;
                expected_grid_rotation180 = 240;
                expected_grid_rotation270 = 17476;
                break;
            case J:
                expected_grid = 2272;
                expected_grid_rotation90 = 1604;
                expected_grid_rotation180 = 226;
                expected_grid_rotation270 = 1100;
                break;
            case L:
                expected_grid = 736;
                expected_grid_rotation90 = 1094;
                expected_grid_rotation180 = 232;
                expected_grid_rotation270 = 3140;
                break;
            case O:
                expected_grid = 1632;
                expected_grid_rotation90 = 1632;
                expected_grid_rotation180 = 1632;
                expected_grid_rotation270 = 1632;
                break;
            case S:
                expected_grid = 1728;
                expected_grid_rotation90 = 1122;
                expected_grid_rotation180 = 108;
                expected_grid_rotation270 = 2244;
                break;
            case T:
                expected_grid = 1248;
                expected_grid_rotation90 = 1124;
                expected_grid_rotation180 = 228;
                expected_grid_rotation270 = 1220;
                break;
            case Z:
                expected_grid = 3168;
                expected_grid_rotation90 = 612;
                expected_grid_rotation180 = 198;
                expected_grid_rotation270 = 1224;
                break;
        }

        grid = tetromino_get_grid(&t);
        assert(grid == expected_grid,
               "%c initial grid %d == %d", type_char, grid, expected_grid);

        grid = tetromino_rotate_clockwise(&t);
        assert(grid == expected_grid_rotation90,
               "%c 90deg clockwise %d == %d", type_char, grid, expected_grid_rotation90);

        grid = tetromino_rotate_clockwise(&t);
        assert(grid == expected_grid_rotation180,
               "%c 180deg clockwise %d == %d", type_char, grid, expected_grid_rotation180);

        grid = tetromino_rotate_clockwise(&t);
        assert(grid == expected_grid_rotation270,
               "%c 270deg clockwise %d == %d", type_char, grid, expected_grid_rotation270);

        grid = tetromino_rotate_clockwise(&t);
        assert(grid == expected_grid, "%c 360deg clockwise == initial grid", type_char);

        grid = tetromino_rotate_counterclockwise(&t);
        assert(grid == expected_grid_rotation270,
               "%c 270deg counter clockwise %d == %d", type_char, grid, expected_grid_rotation270);

        grid = tetromino_rotate_counterclockwise(&t);
        assert(grid == expected_grid_rotation180,
               "%c 180deg counter clockwise %d == %d", type_char, grid, expected_grid_rotation180);

        grid = tetromino_rotate_counterclockwise(&t);
        assert(grid == expected_grid_rotation90,
               "%c 90deg counter clockwise %d == %d", type_char, grid, expected_grid_rotation90);

        grid = tetromino_rotate_counterclockwise(&t);
        assert(grid == expected_grid, "%c 360deg counter clockwise == initial grid", type_char);

    }
/*}}}*/ }