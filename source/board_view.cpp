// board_view.cpp
//
// Draws a checkered 8x8 board, then a test row of every piece sprite we
// have (missing the Queen for now -- see the conversation, the source
// spritesheet didn't include one). This isn't real chess position
// layout yet, it's just "does every texture actually load and draw."
#include <3ds.h>
#include <citro2d.h>
#include <cstdio>
#include "board_view.h"

// The 3DS bottom screen is 320x240. Giving each square 30x30 pixels
// makes a 240x240 board, leaving an even margin on both sides.
static constexpr int SQUARE_SIZE  = 30;
static constexpr int BOARD_PIXELS = SQUARE_SIZE * 8;              // 240
static constexpr int BOARD_X      = (320 - BOARD_PIXELS) / 2;      // centered horizontally
static constexpr int BOARD_Y      = (240 - BOARD_PIXELS) / 2;      // centered vertically

static C3D_RenderTarget* bottomTarget = nullptr;

// One sprite sheet + image per piece file. Each PNG was converted
// individually (not combined into one multi-image sheet), so each
// C2D_SpriteSheet here contains exactly one sub-image, at index 0.
struct PieceTexture
{
    const char* filename; // matches romfs/gfx/<filename>.t3x
    C2D_SpriteSheet sheet = nullptr;
    C2D_Image image;
};

static PieceTexture pieces[] = {
    { "white_king"   },
    { "white_rook"   },
    { "white_bishop" },
    { "white_knight" },
    { "white_pawn"   },
    { "black_king"   },
    { "black_rook"   },
    { "black_bishop" },
    { "black_knight" },
    { "black_pawn"   },
};
static constexpr int NUM_PIECES = sizeof(pieces) / sizeof(pieces[0]);

void board_view_init()
{
    // Mount the embedded RomFS so "romfs:/..." paths work. If this
    // fails, every texture load below will fail too, so we print
    // clearly rather than silently drawing nothing.
    Result romfsResult = romfsInit();
    if (romfsResult != 0)
        printf("[board_view] romfsInit FAILED: 0x%08lX\n", romfsResult);
    else
        printf("[board_view] romfsInit ok\n");

    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    bottomTarget = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    for (int i = 0; i < NUM_PIECES; i++)
    {
        char path[64];
        snprintf(path, sizeof(path), "romfs:/gfx/%s.t3x", pieces[i].filename);

        pieces[i].sheet = C2D_SpriteSheetLoad(path);
        if (!pieces[i].sheet)
        {
            printf("[board_view] FAILED to load %s\n", path);
            continue;
        }
        pieces[i].image = C2D_SpriteSheetGetImage(pieces[i].sheet, 0);
    }
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

    // Test row: draw every loaded piece across the top two ranks, just
    // to confirm each texture actually shows up. Not a real position --
    // we're missing a Queen sprite, so this isn't meant to be one yet.
    for (int i = 0; i < NUM_PIECES; i++)
    {
        if (!pieces[i].sheet)
            continue; // skip any that failed to load, rather than crash

        int col = i % 5;
        int row = (i < 5) ? 0 : 1;

        float x = static_cast<float>(BOARD_X + col * SQUARE_SIZE);
        float y = static_cast<float>(BOARD_Y + row * SQUARE_SIZE);

        C2D_DrawImageAt(pieces[i].image, x, y, 0.5f);
    }

    C3D_FrameEnd(0);
}

void board_view_exit()
{
    for (int i = 0; i < NUM_PIECES; i++)
        if (pieces[i].sheet)
            C2D_SpriteSheetFree(pieces[i].sheet);

    C2D_Fini();
    C3D_Fini();
    romfsExit();
}

