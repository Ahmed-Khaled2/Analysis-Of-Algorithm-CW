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

int minimax(const GameState& state, int depth, char originalPlayer);

Move findBestMove(const GameState& state, char originalPlayer);

void printBoard(const std::vector<std::vector<char>>& board);
void printMove(const Move& mv);

std::vector<int> GetBestMove(std::vector<std::vector<char>> board, char turn);
