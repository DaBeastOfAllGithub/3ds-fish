// tablebase_stub.cpp
//
// Real Stockfish endgame tablebase support (in its "syzygy" folder)
// needs mmap() to map lookup files directly into memory -- a feature
// the 3DS's C library doesn't have. We never use tablebases anyway (we
// never set a SyzygyPath in our hardcoded command list in main.cpp), so
// instead of porting real file-mapping code, this file provides the
// same function names Stockfish expects, each just saying "nothing to
// probe, no tablebase loaded."
//
// This replaces stockfish-src/src/syzygy/tbprobe.cpp entirely -- that
// real file gets deleted before compiling (see build.yml), same as we
// did with Stockfish's own main.cpp earlier.
#include <string>

#include "position.h"
#include "search.h"
#include "syzygy/tbprobe.h"

namespace Tablebases {

// 0 means "no tablebase covers any position of any size." Stockfish
// checks this before ever bothering to call probe_wdl/probe_dtz for a
// given position, so in practice these functions won't even get
// reached -- we still need them to exist so linking succeeds.
int MaxCardinality = 0;

void init(const std::string& paths)
{
    // Real Stockfish would load tablebase files from this path.
    // We have none, so this is intentionally a no-op.
    (void)paths;
}

WDLScore probe_wdl(Position& pos, ProbeState* result)
{
    (void)pos;
    *result = FAIL;
    return WDLScoreNone;
}

int probe_dtz(Position& pos, ProbeState* result)
{
    (void)pos;
    *result = FAIL;
    return 0;
}

bool root_probe(Position& pos, Search::RootMoves& rootMoves)
{
    (void)pos;
    (void)rootMoves;
    return false;
}

bool root_probe_wdl(Position& pos, Search::RootMoves& rootMoves)
{
    (void)pos;
    (void)rootMoves;
    return false;
}

} // namespace Tablebases
