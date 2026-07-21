// main.cpp
//
// Boots the real Stockfish 11 engine and asks it to find a move.
//
// Important: this file does NOT #include <3ds.h> directly, on purpose
// -- see platform_3ds.h for why. All 3DS-specific calls go through
// those wrapper functions instead.
//
// The Stockfish side: its own main.cpp (which we've deleted from this
// build -- see build.yml) does a fixed startup sequence, then calls
// UCI::loop() which reads commands forever from the keyboard until it
// sees "quit". We copy that same startup sequence by hand below, then
// instead of a keyboard we hand it a hardcoded string containing the
// commands we'd have typed ourselves. Stockfish can't tell the
// difference -- as far as it knows, someone is typing at it live.
#include <cstdio>
#include <iostream>
#include <sstream>

#include "platform_3ds.h" // our own wrapper -- not <3ds.h> directly

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
    platform_init();

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
    std::istringstream fakeInput(
        "setoption name Hash value 1\n"     // cap transposition table to 1MB
        "setoption name Threads value 1\n"  // force single-threaded search
        "position startpos\n"               // set up the normal starting position
        "go depth 10\n"                     // search 10 ply deep, report its answer
        "quit\n"                            // tell it to stop and return control to us
    );
    std::cin.rdbuf(fakeInput.rdbuf());

    // Blocks until it processes "quit" above. Same function real
    // Stockfish calls -- just reading from our string instead of a
    // real keyboard.
    UCI::loop(0, nullptr);

    Threads.set(0);

    platform_print_ready();

    while (platform_should_continue())
    {
        platform_present_frame();
    }

    platform_exit();
    return 0;
}
