#pragma once
#include "GameSolver.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <stack>  

using namespace std;

static stack<GameState> gameStateStack;
char getOpponent(char p) {
    return (p == 'R' ? 'G' : 'R');
}

bool isInGoalZone(int r, int c, char p) {
    if (p == 'R') {
        return (r >= 1 && r <= 3 && c == 4);
    }
    else {
        return (r == 4 && c >= 1 && c <= 3);
    }
}

bool isWinningState(const GameState& s, char p) {
    int cnt = 0, n = (int)s.board.size();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (s.board[i][j] == p && isInGoalZone(i, j, p)) {
                ++cnt;
            }
        }
    }
    return cnt >= 3;
}

bool isLosingState(const GameState& s, char p) {
    return isWinningState(s, getOpponent(p));
}

vector<Move> getAllPossibleMoves(const GameState& s, char p) {
    vector<Move> moves;
    int n = (int)s.board.size();
    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            if (s.board[r][c] != p) {
                continue;
            }
            if (isInGoalZone(r, c, p)) {
                continue;
            }
            if (p == 'R') {
                if (c + 1 < n && s.board[r][c + 1] == '.') {
                    moves.push_back({ r,c,r,c + 1 });
                }
                if (c + 2 < n && s.board[r][c + 1] == 'G' && s.board[r][c + 2] == '.') {
                    moves.push_back({ r,c,r,c + 2 });
                }
            }
            else {
                if (r + 1 < n && s.board[r + 1][c] == '.') {
                    moves.push_back({ r,c,r + 1,c });
                }
                if (r + 2 < n && s.board[r + 1][c] == 'R' && s.board[r + 2][c] == '.') {
                    moves.push_back({ r,c,r + 2,c });
                }
            }
        }
    }
    return moves;
}

void applyMove(GameState& state, const Move& mv, char p) {
    gameStateStack.push(state);
    state.board[mv.toRow][mv.toCol] = p;
    state.board[mv.fromRow][mv.fromCol] = '.';
    state.currentPlayer = getOpponent(p);
}

static bool isJumpMove(const Move& m, char player) {
    return (player == 'R' && abs(m.toCol - m.fromCol) == 2) || (player == 'G' && abs(m.toRow - m.fromRow) == 2);
}

static bool isGoalReaching(const Move& m, char player) {
    return isInGoalZone(m.toRow, m.toCol, player);
}

int evaluateState(const GameState& s, char player) {
    char opp = getOpponent(player);
    int score = 0;
    int n = (int)s.board.size();

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            if (s.board[r][c] == player) {
                score += (player == 'R' ? c : r);
                if (isInGoalZone(r, c, player)) {
                    score += 10;
                }
            }
            if (s.board[r][c] == opp) {
                score -= (opp == 'R' ? c : r);
                if (isInGoalZone(r, c, opp)) {
                    score -= 10;
                }
            }
        }
    }

    GameState tmp = s;
    tmp.currentPlayer = opp;
    auto om = getAllPossibleMoves(tmp, opp);
    for (auto& m : om) {
        if (isJumpMove(m, opp)) {
            score -= 5;
        }
    }

    for (auto& m : om) {
        if (isGoalReaching(m, opp)) {
            score -= 20; break;
        }
    }
    return score;
}

static int backtrack_impl(GameState& state, int depth, char originalPlayer) {
    gameStateStack.push(state);

    if (isWinningState(state, originalPlayer)) {
        gameStateStack.pop();
        return 1000 - depth;
    }
    if (isLosingState(state, originalPlayer)) {
        gameStateStack.pop();
        return -1000 + depth;
    }

    const int MAX_DEPTH = 8;
    if (depth >= MAX_DEPTH) {
        int score = evaluateState(state, originalPlayer);
        gameStateStack.pop();
        return score;
    }

    auto moves = getAllPossibleMoves(state, state.currentPlayer);
    int bestVal = (state.currentPlayer == originalPlayer) ? INT_MIN : INT_MAX;

    for (auto& mv : moves) {
        GameState prevState = state;
        applyMove(prevState, mv, state.currentPlayer);
        int val = backtrack_impl(prevState, depth + 1, originalPlayer);

        state = gameStateStack.top();
        gameStateStack.pop();

        if (prevState.currentPlayer == originalPlayer) {
            bestVal = max(bestVal, val);
        }
        else {
            bestVal = min(bestVal, val);
        }
    }

    gameStateStack.pop();
    return bestVal;
}

int backtrack(GameState& state, int depth, char originalPlayer) {
    while (!gameStateStack.empty()) {
        gameStateStack.pop();
    }
    return backtrack_impl(state, depth, originalPlayer);
}

Move findBestMove(const GameState& initialState, char originalPlayer) {
    while (!gameStateStack.empty()) {
        gameStateStack.pop();
    }

    GameState workingState = initialState;
    auto moves = getAllPossibleMoves(workingState, originalPlayer);
    int bestScore = INT_MIN;
    Move bestMove{ -1,-1,-1,-1 };

    for (auto& mv : moves) {
        gameStateStack.push(workingState);

        applyMove(workingState, mv, originalPlayer);

        if (isWinningState(workingState, originalPlayer)) {
            workingState = gameStateStack.top();
            gameStateStack.pop();
            return mv;
        }

        int score = backtrack_impl(workingState, 1, originalPlayer);
        workingState = gameStateStack.top();
        gameStateStack.pop();

        if (score > bestScore) {
            bestScore = score;
            bestMove = mv;
        }
    }
    return bestMove;
}

vector<int> GetBestMove(vector<vector<char>> board, char turn) {
    GameState s{ board, turn };
    if (isWinningState(s, turn)) return { 0 };
    Move mv = findBestMove(s, turn);
    if (mv.fromRow < 0) return { -1 };
    return { mv.fromRow, mv.fromCol, mv.toRow, mv.toCol };
}