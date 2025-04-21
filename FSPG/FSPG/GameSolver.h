#pragma once
#include <vector>
#include <string>

struct Move {
    int fromRow;
    int fromCol;
    int toRow;
    int toCol;
};

struct GameState {
    std::vector<std::vector<char>> board;
    char currentPlayer;
};

std::string encodeGameState(const GameState& state, char originalPlayer);
char getOpponent(char player);
bool isInGoalZone(int row, int col, char player);
bool isWinningState(const GameState& state, char player);
bool isLosingState(const GameState& state, char player);
std::vector<Move> getAllPossibleMoves(const GameState& state, char player);
GameState applyMove(const GameState& state, const Move& move, char player);

int evaluateState(const GameState& state, char player);
static bool isJumpMove(const Move& m, char player);
static bool isGoalReaching(const Move& m, char player);

int alphaBeta(const GameState& state, int depth, char originalPlayer, int alpha, int beta);
inline int  minimax(const GameState& state, int depth, char player) {
    return alphaBeta(state, depth, player, INT_MIN, INT_MAX);
}
Move findBestMove(const GameState& state, char originalPlayer);
std::vector<int> GetBestMove(std::vector<std::vector<char>> board, char turn);
