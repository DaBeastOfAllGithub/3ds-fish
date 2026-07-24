// board_view.cpp
//
// [resync checkpoint -- this is the full, current, known-good version
// as of the RomFS/SMDH debugging session. If your repo has partial
// edits from an earlier interrupted attempt, replace the whole file
// with this one rather than trying to merge.]
//
// Draws a checkered 8x8 board with all pieces in the real chess
// starting position.
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

// Each PNG was converted into its own individual .t3x file (see
// build.yml), so each C2D_SpriteSheet here contains exactly one
// sub-image, always at index 0.
struct PieceTexture
{
    const char* filename; // matches romfs/gfx/<filename>.t3x
    C2D_SpriteSheet sheet = nullptr;
    C2D_Image image;
};

static PieceTexture pieces[] = {
    { "white_king"   },
    { "white_queen"  },
    { "white_rook"   },
    { "white_bishop" },
    { "white_knight" },
    { "white_pawn"   },
    { "black_king"   },
    { "black_queen"  },
    { "black_rook"   },
    { "black_bishop" },
    { "black_knight" },
    { "black_pawn"   },
};
static constexpr int NUM_PIECES = sizeof(pieces) / sizeof(pieces[0]);

// Named indices into `pieces` above, so the board layout below reads
// clearly instead of being a grid of bare numbers.
enum PieceId
{
    WK = 0, WQ, WR, WB, WN, WP,
    BK, BQ, BR, BB, BN, BP,
    EMPTY = -1
};

// board[row][col]: row 0 = top of screen (black's back rank), row 7 =
// bottom (white's back rank) -- standard "white at the bottom" chess
// board orientation. This is a plain mutable array on purpose: once we
// add touch input and move-making, updating a piece's square is just
// writing a new value here.
static int board[8][8] = {
    { BR, BN, BB, BQ, BK, BB, BN, BR },
    { BP, BP, BP, BP, BP, BP, BP, BP },
    { EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
    { EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
    { EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
    { EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
    { WP, WP, WP, WP, WP, WP, WP, WP },
    { WR, WN, WB, WQ, WK, WB, WN, WR },
};

// -1 means "nothing selected". Set by board_view_handle_tap, read by
// board_view_draw to render the highlight.
static int selectedRow = -1;
static int selectedCol = -1;

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
    printf("[board_view] C3D_Init called\n");

    bool c2dOk = C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    printf("[board_view] C2D_Init returned %s\n", c2dOk ? "true" : "FALSE");

    C2D_Prepare();
    printf("[board_view] C2D_Prepare called\n");

    bottomTarget = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    if (!bottomTarget)
        printf("[board_view] C2D_CreateScreenTarget FAILED (null target!)\n");
    else
        printf("[board_view] bottom screen target created ok\n");

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
    printf("[board_view] init complete, entering draw loop\n");
}

void board_view_draw()
{
    static bool printedOnce = false;
    if (!printedOnce)
    {
        printf("[board_view] first board_view_draw() call happening now\n");
        printedOnce = true;
    }

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

    // Highlight the selected square, if any, before drawing pieces on top.
    if (selectedRow != -1)
    {
        const u32 highlight = C2D_Color32(0xFF, 0xFF, 0x40, 0xA0); // semi-transparent yellow
        float x = static_cast<float>(BOARD_X + selectedCol * SQUARE_SIZE);
        float y = static_cast<float>(BOARD_Y + selectedRow * SQUARE_SIZE);
        C2D_DrawRectSolid(x, y, 0.25f, SQUARE_SIZE, SQUARE_SIZE, highlight);
    }

    // Draw each piece at its actual square, per the board[][] state above.
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            int pieceId = board[row][col];
            if (pieceId == EMPTY)
                continue;
            if (!pieces[pieceId].sheet)
                continue; // skip any that failed to load, rather than crash

            float x = static_cast<float>(BOARD_X + col * SQUARE_SIZE);
            float y = static_cast<float>(BOARD_Y + row * SQUARE_SIZE);

            // Our piece textures are 64x64 pixels, but each board square
            // is only SQUARE_SIZE (30) pixels -- without an explicit
            // scale, C2D_DrawImageAt draws at the texture's native size.
            float scale = static_cast<float>(SQUARE_SIZE) / 64.0f;
            C2D_DrawImageAt(pieces[pieceId].image, x, y, 0.5f, nullptr, scale, scale);
        }
    }

    C3D_FrameEnd(0);
}

// Converts a screen tap into a board square, then either selects a
// piece (first tap) or moves the currently-selected piece there
// (second tap). No legality checking yet -- any square is a "legal"
// destination for now, purely to test that touch input and board
// updates work correctly before adding real chess rules.
void board_view_handle_tap(int screenX, int screenY)
{
    // Outside the board entirely -- treat as "cancel selection."
    if (screenX < BOARD_X || screenX >= BOARD_X + BOARD_PIXELS ||
        screenY < BOARD_Y || screenY >= BOARD_Y + BOARD_PIXELS)
    {
        selectedRow = -1;
        selectedCol = -1;
        return;
    }

    int col = (screenX - BOARD_X) / SQUARE_SIZE;
    int row = (screenY - BOARD_Y) / SQUARE_SIZE;

    if (selectedRow == -1)
    {
        // Nothing selected yet -- only select if this square has a piece.
        if (board[row][col] != EMPTY)
        {
            selectedRow = row;
            selectedCol = col;
        }
    }
    else
    {
        if (row == selectedRow && col == selectedCol)
        {
            // Tapped the same square again -- deselect instead of "moving in place."
            selectedRow = -1;
            selectedCol = -1;
        }
        else
        {
            // Move the selected piece to the tapped square (overwrites
            // whatever was there, including capturing -- fine for now,
            // legality/capture rules come once this base is confirmed
            // working).
            board[row][col] = board[selectedRow][selectedCol];
            board[selectedRow][selectedCol] = EMPTY;
            selectedRow = -1;
            selectedCol = -1;
        }
    }
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
