#pragma once
#include <vector>
#include <string>

struct Move {
    int fromRow, fromCol;
    int toRow, toCol;
};

struct GameState {
    std::vector<std::vector<char>> board;
    char currentPlayer;
};

char getOpponent(char player);
bool isWinningState(const GameState& state, char player);
bool isLosingState(const GameState& state, char player);
std::vector<Move> getAllPossibleMoves(const GameState& state, char player);
GameState applyMove(const GameState& state, const Move& move, char player);
std::string PlayAnyGame(const GameState& state, char player);
Move findBestMove(const GameState& state, char player);

void printBoard(const std::vector<std::vector<char>>& board);
void printMove(const Move& move);
std::string GetBestMove(std::vector<std::vector<char>> board, char turn);
