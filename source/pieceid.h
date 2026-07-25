#pragma once
// Shared piece-ID convention between board_view.cpp (drawing) and
// game_state.cpp (the real Stockfish position). Deliberately has zero
// dependencies -- no <3ds.h>, no Stockfish headers -- so both files can
// include it without pulling in anything that would collide.
enum PieceId
{
    WK = 0, WQ, WR, WB, WN, WP,
    BK, BQ, BR, BB, BN, BP,
    EMPTY = -1
};
