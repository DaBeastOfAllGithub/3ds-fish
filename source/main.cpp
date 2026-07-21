// main.cpp
//
// This version boots the real Stockfish 11 engine and asks it to find a
// move, instead of just printing hello-world text.
//
// The approach: Stockfish's own main.cpp (the one we're NOT using --
// see the Makefile, its main.cpp is deleted from the build) does a
// fixed startup sequence, then calls UCI::loop() which reads commands
// forever from the keyboard until it sees "quit". We copy that exact
// same startup sequence by hand here, then instead of a keyboard we
// hand it a hardcoded string containing the commands we'd have typed
// ourselves. Stockfish can't tell the difference -- as far as it knows,
// someone is typing at it in real time.
#include <3ds.h>
#include <cstdio>
#include <iostream>
#include <sstream>

// Stockfish's own headers -- same ones its real main.cpp includes.
#include "bitboard.h"
#include "position.h"
#include "search.h"
#include "thread.h"
#include "tt.h"
#include "uci.h"
#include "endgame.h"
#include "syzygy/tbprobe.h"

namespace PSQT {
  void init();
}

int main(int argc, char** argv)
{
    // ---- 3DS screen setup, same as hello world ----
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    printf("Booting Stockfish on real 3DS hardware...\n\n");

    // ---- Stockfish's own startup sequence, copied exactly from its
    //      real main.cpp, just triggered by us instead of its main() ----
    std::cout << engine_info() << std::endl;
    UCI::init(Options);
    PSQT::init();
    Bitboards::init();
    Position::init();
    Bitbases::init();
    Endgames::init();
    Threads.set(Options["Threads"]);
    Search::clear(); // must happen after threads are up, same order as original

    // ---- Feed it a hardcoded "conversation" instead of a keyboard ----
    // A real UCI-speaking chess GUI would type these same lines by hand,
    // one at a time, waiting for replies in between. We don't have a
    // keyboard wired up yet, so we pre-load all of them into a fake
    // input stream and point cin at it instead.
    std::istringstream fakeInput(
        "setoption name Hash value 1\n"     // cap transposition table to 1MB (3DS has no gigabytes to spare)
        "setoption name Threads value 1\n"  // force single-threaded search
        "position startpos\n"               // set up the normal chess starting position
        "go depth 10\n"                     // search 10 ply deep, then report its answer
        "quit\n"                            // tell it to stop and return control to us
    );
    std::cin.rdbuf(fakeInput.rdbuf());

    // This call blocks until it processes "quit" above. It's the exact
    // same function real Stockfish calls from its own main() -- we're
    // just handing it our fake conversation instead of a real terminal.
    UCI::loop(0, nullptr);

    Threads.set(0);

    printf("\nDone -- see Stockfish's output above.\n");
    printf("Press START to exit.\n");

    // ---- keep the screen up so you can actually read the output ----
    while (aptMainLoop())
    {
        hidScanInput();
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START)
            break;

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 0;
}
