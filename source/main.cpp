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

    // The absolute earliest checkpoint possible: if this doesn't show up
    // on screen, the crash is happening BEFORE main() even starts
    // running -- i.e. during automatic construction of Stockfish's
    // global objects (its options table, thread pool, lookup tables),
    // which C++ runs before main() is entered. That would explain a
    // truly instant crash with nothing printed.
    printf("[checkpoint 0] inside main(), console is alive\n");
    platform_flush_now();

    printf("Booting Stockfish on real 3DS hardware...\n\n");
    platform_flush_now();

    // ---- Stockfish's own startup sequence, copied exactly from its
    //      real main.cpp, just triggered by us instead of its main() ----
    // Checkpoints (printf after each call, forced onto screen
    // immediately) are here on purpose: if this crashes again, whatever
    // was last visible tells us exactly which call did it.
    std::cout << engine_info() << std::endl;
    printf("[ok] engine_info\n");
    platform_flush_now();

    UCI::init(Options);
    printf("[ok] UCI::init\n");
    platform_flush_now();

    PSQT::init();
    printf("[ok] PSQT::init\n");
    platform_flush_now();

    Bitboards::init();
    printf("[ok] Bitboards::init\n");
    platform_flush_now();

    Position::init();
    printf("[ok] Position::init\n");
    platform_flush_now();

    Bitbases::init();
    printf("[ok] Bitbases::init\n");
    platform_flush_now();

    Endgames::init();
    printf("[ok] Endgames::init\n");
    platform_flush_now();

    Threads.set(Options["Threads"]);
    printf("[ok] Threads.set (this spawns a real thread -- watch this one)\n");
    platform_flush_now();

    Search::clear(); // must happen after threads are up, same order as original
    printf("[ok] Search::clear\n");
    platform_flush_now();

    // ---- Feed it a hardcoded "conversation" instead of a keyboard ----
    std::istringstream fakeInput(
        "setoption name Hash value 1\n"     // cap transposition table to 1MB
        "setoption name Threads value 1\n"  // force single-threaded search
        "position startpos\n"               // set up the normal starting position
        "go depth 10\n"                     // search 10 ply deep, report its answer
        "quit\n"                            // tell it to stop and return control to us
    );
    std::cin.rdbuf(fakeInput.rdbuf());

    // IMPORTANT FIX: Stockfish's UCI::loop() only reads from cin (our
    // fake commands above) when argc == 1 -- passing 0 like we did
    // before silently skips reading cin entirely! We need to pass a
    // dummy argc=1 with a placeholder argv[0] (program name, unused)
    // so it takes the "read from cin" branch instead of the "treat
    // command-line args as a one-shot command" branch.
    char progName[] = "3ds-fish";
    char* fakeArgv[] = { progName };
    printf("[ok] about to enter UCI::loop (this runs the actual search)\n");
    UCI::loop(1, fakeArgv);
    printf("[ok] UCI::loop returned\n");

    Threads.set(0);
    printf("[ok] Threads.set(0)\n");

    platform_print_ready();

    while (platform_should_continue())
    {
        platform_present_frame();
    }

    platform_exit();
    return 0;
}
