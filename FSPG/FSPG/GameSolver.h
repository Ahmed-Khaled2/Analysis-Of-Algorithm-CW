// GameSolver.h
#pragma once
#include <vector>
#include <string>
#include <unordered_map>

struct Move {
    int fromRow;
    int fromCol;
    int toRow;
    int toCol;
};

struct GameState {
    std::vector<std::vector<char>> board;
    char currentPlayer;  // 'R' or 'G'
};

// Basic game logic
std::string encodeGameState(const GameState& state, char originalPlayer);
char        getOpponent(char player);
bool        isInGoalZone(int row, int col, char player);
bool        isWinningState(const GameState& state, char player);
bool        isLosingState(const GameState& state, char player);
std::vector<Move> getAllPossibleMoves(const GameState& state, char player);
GameState   applyMove(const GameState& state, const Move& move, char player);

// Static evaluation and helpers
int         evaluateState(const GameState& state, char player);
static bool isJumpMove(const Move& m, char player);
static bool isGoalReaching(const Move& m, char player);

// Alpha‐beta search
int         alphaBeta(const GameState& state,
    int depth,
    char originalPlayer,
    int alpha,
    int beta);

// Convenience full‐depth wrapper
inline int  minimax(const GameState& state, int depth, char player) {
    return alphaBeta(state, depth, player,
        std::numeric_limits<int>::min(),
        std::numeric_limits<int>::max());
}

// Top‐level move picker
Move        findBestMove(const GameState& state, char originalPlayer);

// Utilities
void        printBoard(const std::vector<std::vector<char>>& board);
void        printMove(const Move& mv);

// C‑API for GUI
std::vector<int> GetBestMove(std::vector<std::vector<char>> board,
    char turn);
