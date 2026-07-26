// game_state.cpp
//
// Owns the one persistent Stockfish Position for the whole play
// session. All the Stockfish boot-up that used to live in main.cpp
// (before this module existed) now lives here instead, since this is
// the thing that actually needs Stockfish alive for the long haul.
#include <memory>
#include <deque>

#include "game_state.h"

#include "bitboard.h"
#include "position.h"
#include "search.h"
#include "thread.h"
#include "tt.h"
#include "uci.h"
#include "endgame.h"
#include "movegen.h"
#include "misc.h"
#include "syzygy/tbprobe.h"

namespace PSQT {
  void init();
}

static Position pos;
static StateListPtr states;

static const char* START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// Converts our (row, col) board coordinates to a Stockfish Square.
// row 0 = rank 8 (black's back rank), row 7 = rank 1 (white's back
// rank); col 0 = file a ... col 7 = file h. Matches board_view exactly.
static Square square_from_rc(int row, int col)
{
    return make_square(static_cast<File>(col), static_cast<Rank>(7 - row));
}

static void rc_from_square(Square sq, int& row, int& col)
{
    col = static_cast<int>(file_of(sq));
    row = 7 - static_cast<int>(rank_of(sq));
}

static int piece_id_from_sf(Piece p)
{
    if (p == NO_PIECE)
        return EMPTY;

    bool isWhite = (color_of(p) == WHITE);

    switch (type_of(p))
    {
        case KING:   return isWhite ? WK : BK;
        case QUEEN:  return isWhite ? WQ : BQ;
        case ROOK:   return isWhite ? WR : BR;
        case BISHOP: return isWhite ? WB : BB;
        case KNIGHT: return isWhite ? WN : BN;
        case PAWN:   return isWhite ? WP : BP;
        default:     return EMPTY;
    }
}

void game_state_init()
{
    // Same startup sequence Stockfish's own main.cpp does -- see the
    // conversation history, we copied this from its real source once
    // already for the original headless build. Order matters a lot
    // here: setting Options["Threads"] or Options["Hash"] TRIGGERS
    // real work immediately (on_threads calls Threads.set(), on_hash
    // calls TT.resize()) -- so those must come AFTER the subsystems
    // they depend on are ready, not before.
    UCI::init(Options);

    PSQT::init();
    Bitboards::init();
    Position::init();
    Bitbases::init();
    Endgames::init();
    Threads.set(static_cast<size_t>(Options["Threads"])); // default is already 1, no override needed
    Search::clear();

    // NOW it's safe to cap Hash -- Threads pool already exists, so
    // TT.resize() (triggered by this assignment) isn't touching
    // anything that doesn't exist yet.
    Options["Hash"] = std::string("1");

    states = StateListPtr(new std::deque<StateInfo>(1));
    pos.set(START_FEN, false, &states->back(), Threads.main());
}

void game_state_get_board(int board[8][8])
{
    for (int row = 0; row < 8; row++)
        for (int col = 0; col < 8; col++)
            board[row][col] = piece_id_from_sf(pos.piece_on(square_from_rc(row, col)));
}

void game_state_get_legal_destinations(int fromRow, int fromCol, bool dest[8][8])
{
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            dest[r][c] = false;

    Square from = square_from_rc(fromRow, fromCol);

    for (const auto& extMove : MoveList<LEGAL>(pos))
    {
        Move m = extMove;
        if (from_sq(m) == from)
        {
            int r, c;
            rc_from_square(to_sq(m), r, c);
            dest[r][c] = true;
        }
    }
}

bool game_state_try_move(int fromRow, int fromCol, int toRow, int toCol)
{
    Square from = square_from_rc(fromRow, fromCol);
    Square to   = square_from_rc(toRow, toCol);

    for (const auto& extMove : MoveList<LEGAL>(pos))
    {
        Move m = extMove;
        // NOTE: pawn promotions generate 4 separate legal moves (one
        // per promotion piece) sharing the same from/to squares -- this
        // will match whichever one the generator lists first, which
        // isn't guaranteed to be a queen promotion. Fine for now; a
        // promotion-choice UI is a later problem, not this step's.
        if (from_sq(m) == from && to_sq(m) == to)
        {
            states->emplace_back();
            pos.do_move(m, states->back());
            return true;
        }
    }
    return false;
}

bool game_state_engine_move()
{
    // Same call Stockfish's own "go depth N" UCI command makes
    // internally (uci.cpp's go() function) -- just triggered directly
    // instead of through the UCI text layer.
    Search::LimitsType limits;
    limits.startTime = now(); // "As early as possible!" -- same comment as the real code
    limits.depth = 8;          // real bug was elsewhere (see below) -- restoring depth now that it's fixed

    Threads.start_thinking(pos, states, limits, false);
    Threads.main()->wait_for_search_finished();

    if (Threads.main()->rootMoves.empty())
        return false; // no legal moves -- checkmate or stalemate

    Move best = Threads.main()->rootMoves[0].pv[0];
    if (best == MOVE_NONE)
        return false;

    // start_thinking() steals our `states` object internally (moves it
    // into its own storage -- confirmed from Stockfish's real source,
    // which even comments "states is now empty" right where it happens).
    // Our copy is null at this point, so we need a fresh one before we
    // can apply the chosen move to `pos`.
    states = StateListPtr(new std::deque<StateInfo>(1));
    states->emplace_back();
    pos.do_move(best, states->back());
    return true;
}
