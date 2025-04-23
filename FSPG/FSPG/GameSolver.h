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

char getOpponent(char player);
bool isInGoalZone(int row, int col, char player);
bool isWinningState(const GameState& state, char player);
bool isLosingState(const GameState& state, char player);
std::vector<Move> getAllPossibleMoves(const GameState& state, char player);
void applyMove(GameState& state, const Move& mv, char p);

int evaluateState(const GameState& state, char player);
static bool isJumpMove(const Move& m, char player);
static bool isGoalReaching(const Move& m, char player);

int backtrack(GameState& state, int depth, char originalPlayer);
Move findBestMove(const GameState& state, char originalPlayer);
std::vector<int> GetBestMove(std::vector<std::vector<char>> board, char turn);