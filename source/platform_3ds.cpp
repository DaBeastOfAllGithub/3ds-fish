// platform_3ds.cpp
//
// All the actual libctru/3DS-specific calls live here, and ONLY here.
// This is the one file allowed to #include <3ds.h> -- see
// platform_3ds.h for why that matters.
#include <3ds.h>
#include <cstdio>
#include "platform_3ds.h"

// devkitPro convention: this weak symbol, if defined, overrides the
// default 32KB main-thread stack size. Stockfish's C++ standard library
// usage (iostream, strings, maps) needs considerably more headroom than
// small "hello world" homebrew apps typically do -- this is a common
// thing to bump when porting heavier C++ code onto the 3DS.
extern "C" {
    u32 __stacksize__ = 512 * 1024; // 512KB, up from the ~32KB default
}

void platform_init()
{
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);
}

void platform_flush_now()
{
    gfxFlushBuffers();
    gfxSwapBuffers();
    gspWaitForVBlank();
}

void platform_print_ready()
{
    printf("\nDone -- see Stockfish's output above.\n");
    printf("Press START to exit.\n");
}

bool platform_should_continue()
{
    if (!aptMainLoop())
        return false;

    hidScanInput();
    u32 kDown = hidKeysDown();
    if (kDown & KEY_START)
        return false;

    return true;
}

void platform_present_frame()
{
    gfxFlushBuffers();
    gfxSwapBuffers();
    gspWaitForVBlank();
}

void platform_exit()
{
    gfxExit();
}
