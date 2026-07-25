// main.cpp
//
// Now that game_state.cpp owns the persistent Stockfish position (and
// does its own full boot sequence internally), this file no longer
// needs to touch Stockfish's headers at all -- it's purely UI
// orchestration: turn on the screens, boot the board+engine, loop.
//
// Doesn't #include <3ds.h> directly either -- see platform_3ds.h for
// why. All 3DS-specific calls go through those wrapper functions.
#include <cstdio>

#include "platform_3ds.h" // our own wrapper -- not <3ds.h> directly
#include "board_view.h"   // draws the board and boots the game state internally

int main(int argc, char** argv)
{
    platform_init();

    printf("Booting...\n");
    platform_flush_now();

    // board_view_init() boots the real Stockfish game state (see
    // game_state.cpp) and loads all the piece art.
    board_view_init();

    platform_print_ready();

    while (platform_should_continue())
    {
        int touchX, touchY;
        if (platform_get_touch_tap(touchX, touchY))
            board_view_handle_tap(touchX, touchY);

        board_view_draw();
    }

    board_view_exit();
    platform_exit();
    return 0;
}
