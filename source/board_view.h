#pragma once
// Draws the checkered chess board on the bottom (touch) screen using
// citro2d (a 2D graphics library built on top of the 3DS's GPU). Kept
// in its own file, same "wall" pattern as platform_3ds.h, so main.cpp
// doesn't need to touch citro2d/citro3d headers directly.

void board_view_init();  // sets up citro2d + the bottom screen's render target + loads piece art
void board_view_draw();  // clears + redraws one frame: the board plus a test row of pieces
void board_view_exit();  // clean shutdown
