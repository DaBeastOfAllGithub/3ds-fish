// board_view.cpp
//
// First graphics milestone: just draw a checkered 8x8 board, centered
// on the bottom (touch) screen. No pieces, no touch handling yet --
// same "prove the smallest possible thing works" approach as the
// hello-world / Stockfish-headless steps earlier.
#include <3ds.h>
#include <citro2d.h>
#include "board_view.h"

// The 3DS bottom screen is 320x240. Giving each square 30x30 pixels
// makes a 240x240 board, leaving an even margin on both sides.
static constexpr int SQUARE_SIZE  = 30;
static constexpr int BOARD_PIXELS = SQUARE_SIZE * 8;              // 240
static constexpr int BOARD_X      = (320 - BOARD_PIXELS) / 2;      // centered horizontally
static constexpr int BOARD_Y      = (240 - BOARD_PIXELS) / 2;      // centered vertically

static C3D_RenderTarget* bottomTarget = nullptr;

void board_view_init()
{
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    bottomTarget = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
}

void board_view_draw()
{
    // Classic green/cream chess-board colors.
    const u32 lightSquare = C2D_Color32(0xEE, 0xEE, 0xD2, 0xFF);
    const u32 darkSquare  = C2D_Color32(0x76, 0x96, 0x56, 0xFF);
    const u32 background  = C2D_Color32(0x20, 0x20, 0x20, 0xFF);

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(bottomTarget, background);
    C2D_SceneBegin(bottomTarget);

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            bool isLight = ((row + col) % 2 == 0);
            u32 color = isLight ? lightSquare : darkSquare;

            float x = static_cast<float>(BOARD_X + col * SQUARE_SIZE);
            float y = static_cast<float>(BOARD_Y + row * SQUARE_SIZE);

            C2D_DrawRectSolid(x, y, 0.0f, SQUARE_SIZE, SQUARE_SIZE, color);
        }
    }

    C3D_FrameEnd(0);
}

void board_view_exit()
{
    C2D_Fini();
    C3D_Fini();
}
