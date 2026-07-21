// main.cpp
//
// This is the whole program. Think of everything below as one long "main
// subroutine" that never RTS's until the user quits.
//
// #include is like your NES project pulling in a table of subroutine
// addresses/constants that someone else already wrote (3ds.h = libctru's
// "here's how to talk to the hardware" header).
#include <3ds.h>
#include <cstdio>

// In C/C++, `int main(...)` is just the label the OS jumps to first ---
// like a reset vector pointing at your init code. `int argc, char** argv`
// are command-line args; we don't use them, they're just part of the
// required "function signature" (think: a fixed calling convention, like
// "A register = arg1" for a specific subroutine).
int main(int argc, char** argv)
{
    // ---- SETUP (like your power-on init: clear RAM, set up PPU) ----

    // gfxInitDefault() = "turn on both LCD screens and set up their
    // framebuffers". Nothing will show up until you call this, same as
    // an NES needing PPUCTRL/PPUMASK set before anything renders.
    gfxInitDefault();

    // consoleInit() turns one screen into a simple text terminal, so we
    // can printf() text to it instead of drawing pixels/sprites by hand.
    // GFX_TOP = top screen. GFX_BOTTOM would be the touch screen.
    // (Second argument NULL = "manage the console buffer yourself,
    // don't make me pass in a custom one".)
    consoleInit(GFX_TOP, NULL);

    // printf is a subroutine (from cstdio) that, because of consoleInit,
    // has been "rewired" to write to the 3DS screen instead of nowhere.
    printf("Hello from a real 3DS build!\n");
    printf("This proves the toolchain + CI pipeline works.\n");
    printf("Press START to exit.\n");

    // ---- MAIN LOOP (like your NES loop: wait for vblank, poll pads, repeat) ----

    // aptMainLoop() returns true every frame until the OS tells the app
    // to close (e.g. user pressed the HOME button and closed it).
    // It's your `forever: ... jmp forever` label, but with a built-in
    // "unless the OS says stop" check.
    while (aptMainLoop())
    {
        // hidScanInput() = poll the controller/touchscreen this frame,
        // like reading $4016/$4017 on NES.
        hidScanInput();

        // hidKeysDown() returns a bitmask (a "status byte", but 32 bits)
        // of which buttons were newly pressed THIS frame.
        u32 kDown = hidKeysDown();

        // KEY_START is just a #define'd bit-mask constant. This is the
        // same idea as `AND #%00010000` / `BNE` to test one bit of a
        // status register.
        if (kDown & KEY_START)
            break; // "RTS out of the main loop"

        // These three lines are the 3DS equivalent of "wait for vblank,
        // then flip the double-buffered framebuffer":
        gfxFlushBuffers();   // make sure all drawing commands are done
        gfxSwapBuffers();    // swap front/back buffer (what's shown vs what's drawn to)
        gspWaitForVBlank();  // block until the screen's vertical blank, avoids tearing
    }

    // ---- TEARDOWN ----
    gfxExit(); // opposite of gfxInitDefault() -- release the screens cleanly

    return 0; // like RTS with a "success" code left in the accumulator
}
