#include "GameSolver.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <limits>
using namespace std;

static string encodeGameState(const GameState& state, char originalPlayer) {
    stringstream ss;
    for (const auto& row : state.board)
        for (char cell : row)
            ss << cell;
    ss << state.currentPlayer << originalPlayer;
    return ss.str();
}

static unordered_map<string, int> memoMinimax;

char getOpponent(char player) {
    return (player == 'R' ? 'G' : 'R');
}

bool isInGoalZone(int row, int col, char player) {
    if (player == 'R')
        return (row >= 1 && row <= 3 && col == 4);
    else // 'G'
        return (row == 4 && col >= 1 && col <= 3);
}

bool isWinningState(const GameState& state, char player) {
    int count = 0;
    int n = state.board.size();
    for (int r = 0; r < n; ++r)
        for (int c = 0; c < n; ++c)
            if (state.board[r][c] == player && isInGoalZone(r, c, player))
                ++count;
    return (count >= 3);
}

bool isLosingState(const GameState& state, char player) {
    return isWinningState(state, getOpponent(player));
}

vector<Move> getAllPossibleMoves(const GameState& state, char player) {
    vector<Move> moves;
    int n = state.board.size();
    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            if (state.board[r][c] != player) continue;
            if (isInGoalZone(r, c, player)) continue;

            // Normal step
            if (player == 'R') {
                if (c + 1 < n && (state.board[r][c + 1] == '.' || state.board[r][c + 1] == '*'))
                    moves.push_back({ r,c, r, c + 1 });
                // Jump over 'G'
                if (c + 2 < n && state.board[r][c + 1] == 'G' &&
                    (state.board[r][c + 2] == '.' || state.board[r][c + 2] == '*'))
                    moves.push_back({ r,c, r, c + 2 });
            }
            else {
                // player == 'G'
                if (r + 1 < n && (state.board[r + 1][c] == '.' || state.board[r + 1][c] == '*'))
                    moves.push_back({ r,c, r + 1, c });
                // Jump over 'R'
                if (r + 2 < n && state.board[r + 1][c] == 'R' &&
                    (state.board[r + 2][c] == '.' || state.board[r + 2][c] == '*'))
                    moves.push_back({ r,c, r + 2, c });
            }
        }
    }
    return moves;
}

GameState applyMove(const GameState& state, const Move& move, char player) {
    GameState ns = state;
    ns.board[move.toRow][move.toCol] = player;
    ns.board[move.fromRow][move.fromCol] = '.';
    ns.currentPlayer = getOpponent(player);
    return ns;
}

int minimax(const GameState& state, int depth, char originalPlayer) {
    // Terminal: someone has won/lost
    if (isWinningState(state, originalPlayer))
        return 1000 - depth;
    if (isLosingState(state, originalPlayer))
        return -1000 + depth;

    // Generate all moves
    vector<Move> moves = getAllPossibleMoves(state, state.currentPlayer);

    // If no moves, pass turn to opponent
    if (moves.empty()) {
        GameState passed = state;
        passed.currentPlayer = getOpponent(state.currentPlayer);
        return minimax(passed, depth + 1, originalPlayer);
    }

    // Memo key
    string key = encodeGameState(state, originalPlayer);
    auto it = memoMinimax.find(key);
    if (it != memoMinimax.end())
        return it->second;

    int bestScore;
    if (state.currentPlayer == originalPlayer) {
        // Maximizer
        bestScore = numeric_limits<int>::min();
        for (auto& mv : moves) {
            GameState nxt = applyMove(state, mv, state.currentPlayer);
            bestScore = max(bestScore, minimax(nxt, depth + 1, originalPlayer));
        }
    }
    else {
        // Minimizer
        bestScore = numeric_limits<int>::max();
        for (auto& mv : moves) {
            GameState nxt = applyMove(state, mv, state.currentPlayer);
            bestScore = min(bestScore, minimax(nxt, depth + 1, originalPlayer));
        }
    }

    memoMinimax[key] = bestScore;
    return bestScore;
}

static bool isGoalReaching(const Move& m, char player) {
    return isInGoalZone(m.toRow, m.toCol, player);
}
static bool isJumpMove(const Move& m, char player) {
    return (player == 'R' && abs(m.toCol - m.fromCol) == 2)
        || (player == 'G' && abs(m.toRow - m.fromRow) == 2);
}
static bool isBlockingMove(const GameState& before, const Move& m, char player) {
    char opp = getOpponent(player);
    if (opp == 'R' && m.toCol > 0 && before.board[m.toRow][m.toCol - 1] == opp) return true;
    if (opp == 'G' && m.toRow > 0 && before.board[m.toRow - 1][m.toCol] == opp) return true;
    return false;
}

Move findBestMove(const GameState& state, char originalPlayer) {
    // Clear memo before each top‑level call
    memoMinimax.clear();

    vector<Move> moves = getAllPossibleMoves(state, originalPlayer);
    int bestScore = numeric_limits<int>::min();
    vector<Move> candidates;

    // Evaluate each move
    for (auto& mv : moves) {
        GameState nxt = applyMove(state, mv, originalPlayer);
        // immediate win
        if (isWinningState(nxt, originalPlayer))
            return mv;

        int score = minimax(nxt, 1, originalPlayer);
        if (score > bestScore) {
            bestScore = score;
            candidates = { mv };
        }
        else if (score == bestScore) {
            candidates.push_back(mv);
        }
    }

    // Tiebreakers
    for (auto& mv : candidates) if (isGoalReaching(mv, originalPlayer)) return mv;
    for (auto& mv : candidates) if (isJumpMove(mv, originalPlayer))   return mv;
    for (auto& mv : candidates) if (isBlockingMove(state, mv, originalPlayer)) return mv;

    // Avoid giving opponent a jump, if possible
    vector<Move> safe;
    char opp = getOpponent(originalPlayer);
    for (auto& mv : candidates) {
        GameState nxt = applyMove(state, mv, originalPlayer);
        bool oppCanJump = false;
        for (auto& omv : getAllPossibleMoves(nxt, opp))
            if (isJumpMove(omv, opp)) { oppCanJump = true; break; }
        if (!oppCanJump) safe.push_back(mv);
    }
    if (!safe.empty()) return safe[0];

    // fallback
    return candidates.empty()
        ? Move{ -1,-1,-1,-1 }
    : candidates[0];
}

void printBoard(const vector<vector<char>>& board) {
    for (auto& row : board) {
        for (char c : row) cout << c << ' ';
        cout << '\n';
    }
}

vector<int> GetBestMove(vector<vector<char>> board, char turn) {
    GameState start{ board, turn };
    cout << "Current board:\n"; printBoard(start.board);
    cout << "Player: " << turn << "\n";

    if (isWinningState(start, turn)) {
        cout << "Already won!\n";
        return { 0 };
    }

    Move best = findBestMove(start, turn);
    cout << "Recommended move:\n";
    
    
    if (best.fromRow >= 0) {
        cout << "Move: (" << best.fromRow << ',' << best.fromCol << ") -> ("
            << best.toRow << ',' << best.toCol << ")\n";
        return { best.fromRow, best.fromCol ,best.toRow, best.toCol };
    }
    else {
        cout << "No legal Move!";
        return { -1 };
    }

    // Return string status if your GUI needs it
    //return "done";
}
