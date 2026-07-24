#pragma once
// Draws the checkered chess board on the bottom (touch) screen using
// citro2d (a 2D graphics library built on top of the 3DS's GPU). Kept
// in its own file, same "wall" pattern as platform_3ds.h, so main.cpp
// doesn't need to touch citro2d/citro3d headers directly.

void board_view_init();   // sets up citro2d + the bottom screen's render target + loads piece art
void board_view_draw();   // clears + redraws one frame: the board, pieces, and selection highlight
void board_view_exit();   // clean shutdown

// Call with a tap's screen coordinates (from platform_get_touch_tap).
// First tap on a piece selects it (highlighted on next draw); a second
// tap on a different square moves it there. No legality checking yet --
// that comes once this is wired up and tested on real hardware.
void board_view_handle_tap(int screenX, int screenY);
