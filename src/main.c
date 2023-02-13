#include "playfield.h"
#include "graphics.h"
#include "engine.h"


int main(int argc, char *argv[]) {
    engine_init();    
    graphics_init();
    draw_game();
    
    engine_input_loop();

    graphics_clean();
    engine_clean();
    return 0;
}
