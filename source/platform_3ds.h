// platform_3ds.h
//
// Why this file exists: libctru (the 3DS library) defines its own type
// called "Thread" for native 3DS-level threads. Stockfish ALSO defines
// its own class called "Thread" for search worker threads -- two
// unrelated things that just happen to share a name. If a single file
// includes both <3ds.h> and Stockfish's headers, the compiler can't
// tell the two "Thread"s apart and everything breaks.
//
// The fix: only ONE file (platform_3ds.cpp) is allowed to #include
// <3ds.h>. Every other file, including main.cpp, only sees these plain
// function names instead -- no libctru types leak out of this wall.
#pragma once

void platform_init();             // turn on screens, set up the console
void platform_flush_now();        // force whatever's been printf'd so far onto the actual screen right now
void platform_print_ready();      // print the "press START to exit" prompt
bool platform_should_continue();  // polls input; false once START is pressed
void platform_present_frame();    // swap buffers / wait for vblank
void platform_exit();             // clean shutdown
