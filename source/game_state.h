#pragma once
#include "piece_ids.h"
// Owns the one live chess position (a real Stockfish Position object)
// for the whole play session -- unlike the very first version of
// main.cpp, which ran one throwaway search and exited, this stays
// alive so we can check legal moves now, and let Stockfish actually
// play replies later. This is the only file besides itself allowed to
// touch Stockfish's headers; board_view.cpp never does, to avoid the
// libctru-vs-Stockfish "Thread" name collision from earlier.

void game_state_init(); // full Stockfish boot sequence + starting position

// Fills board[8][8] (row 0 = black's back rank, col 0 = file a -- same
// convention board_view already uses) from the real current position.
void game_state_get_board(int board[8][8]);

// Fills dest[8][8] with true for every square the piece at
// (fromRow, fromCol) can legally move to right now. Clears dest first.
// If the square is empty, everything is left false.
void game_state_get_legal_destinations(int fromRow, int fromCol, bool dest[8][8]);

// Attempts the move; only succeeds if it's actually legal. Returns
// true and applies it, or returns false and changes nothing.
bool game_state_try_move(int fromRow, int fromCol, int toRow, int toCol);
