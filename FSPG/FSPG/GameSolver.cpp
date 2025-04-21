// GameSolver.cpp
#include "GameSolver.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <limits>
#include <algorithm>
#include <cmath>

using namespace std;

//-----------------------------------------------------------------------------
// Encode a game state to a string (for transposition keys)
//-----------------------------------------------------------------------------
string encodeGameState(const GameState& s, char orig) {
    stringstream ss;
    for (auto& row : s.board)
        for (char c : row) ss << c;
    ss << s.currentPlayer << orig;
    return ss.str();
}

//-----------------------------------------------------------------------------
// Basic game logic helpers
//-----------------------------------------------------------------------------
char getOpponent(char p) {
    return (p == 'R' ? 'G' : 'R');
}

bool isInGoalZone(int r, int c, char p) {
    if (p == 'R') return (r >= 1 && r <= 3 && c == 4);
    else          return (r == 4 && c >= 1 && c <= 3);
}

bool isWinningState(const GameState& s, char p) {
    int cnt = 0, n = (int)s.board.size();
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (s.board[i][j] == p && isInGoalZone(i, j, p))
                ++cnt;
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
            if (s.board[r][c] != p) continue;
            if (isInGoalZone(r, c, p)) continue;
            if (p == 'R') {
                if (c + 1 < n && s.board[r][c + 1] == '.')
                    moves.push_back({ r,c,r,c + 1 });
                if (c + 2 < n && s.board[r][c + 1] == 'G' && s.board[r][c + 2] == '.')
                    moves.push_back({ r,c,r,c + 2 });
            }
            else {
                if (r + 1 < n && s.board[r + 1][c] == '.')
                    moves.push_back({ r,c,r + 1,c });
                if (r + 2 < n && s.board[r + 1][c] == 'R' && s.board[r + 2][c] == '.')
                    moves.push_back({ r,c,r + 2,c });
            }
        }
    }
    return moves;
}

GameState applyMove(const GameState& s, const Move& m, char p) {
    GameState ns = s;
    ns.board[m.toRow][m.toCol] = p;
    ns.board[m.fromRow][m.fromCol] = '.';
    ns.currentPlayer = getOpponent(p);
    return ns;
}

//-----------------------------------------------------------------------------
// Helper predicates for evaluation
//-----------------------------------------------------------------------------
static bool isJumpMove(const Move& m, char player) {
    return (player == 'R' && abs(m.toCol - m.fromCol) == 2)
        || (player == 'G' && abs(m.toRow - m.fromRow) == 2);
}

static bool isGoalReaching(const Move& m, char player) {
    return isInGoalZone(m.toRow, m.toCol, player);
}

//-----------------------------------------------------------------------------
// Static evaluation: progress, goal, center, opponent threats
//-----------------------------------------------------------------------------
int evaluateState(const GameState& s, char player) {
    char opp = getOpponent(player);
    int score = 0;
    int n = (int)s.board.size();

    // own pieces
    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            if (s.board[r][c] == player) {
                score += (player == 'R' ? c : r);
                if (isInGoalZone(r, c, player)) score += 10;
                if (r == 2 && c == 2) score += 2;
            }
        }
    }
    // opponent pieces
    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            if (s.board[r][c] == opp) {
                score -= (opp == 'R' ? c : r);
                if (isInGoalZone(r, c, opp)) score -= 10;
                if (r == 2 && c == 2) score -= 2;
            }
        }
    }
    // immediate opponent jumps
    {
        GameState tmp = s;
        tmp.currentPlayer = opp;
        auto om = getAllPossibleMoves(tmp, opp);
        for (auto& m : om)
            if (isJumpMove(m, opp)) score -= 5;
    }
    // opponent goal threat
    {
        GameState tmp = s;
        tmp.currentPlayer = opp;
        auto om = getAllPossibleMoves(tmp, opp);
        for (auto& m : om) {
            if (isGoalReaching(m, opp)) { score -= 20; break; }
        }
    }
    return score;
}

//-----------------------------------------------------------------------------
// Transposition table
//-----------------------------------------------------------------------------
static unordered_map<string, int> transTable;

//-----------------------------------------------------------------------------
// Alpha–Beta search with depth‑limit + ordering + TT
//-----------------------------------------------------------------------------
static int alphaBeta_impl(const GameState& state,
    int depth,
    char originalPlayer,
    int alpha,
    int beta)
{
    if (isWinningState(state, originalPlayer))
        return 1000 - depth;
    if (isLosingState(state, originalPlayer))
        return -1000 + depth;

    const int MAX_DEPTH = 8;
    if (depth >= MAX_DEPTH)
        return evaluateState(state, originalPlayer);

    string key = encodeGameState(state, originalPlayer)
        + "|" + to_string(depth)
        + "|" + to_string(alpha)
        + "|" + to_string(beta);
    auto it = transTable.find(key);
    if (it != transTable.end())
        return it->second;

    bool maximizing = (state.currentPlayer == originalPlayer);
    int bestVal = maximizing
        ? numeric_limits<int>::min()
        : numeric_limits<int>::max();

    auto moves = getAllPossibleMoves(state, state.currentPlayer);
    sort(moves.begin(), moves.end(), [&](auto& a, auto& b) {
        bool ga = isGoalReaching(a, state.currentPlayer);
        bool gb = isGoalReaching(b, state.currentPlayer);
        if (ga != gb) return ga;
        int da = abs(a.toRow - a.fromRow) + abs(a.toCol - a.fromCol);
        int db = abs(b.toRow - b.fromRow) + abs(b.toCol - b.fromCol);
        return da > db;
        });

    for (auto& mv : moves) {
        GameState nxt = applyMove(state, mv, state.currentPlayer);
        int val = alphaBeta_impl(nxt, depth + 1, originalPlayer, alpha, beta);
        if (maximizing) {
            if (val > bestVal) bestVal = val;
            if (val > alpha)  alpha = val;
        }
        else {
            if (val < bestVal) bestVal = val;
            if (val < beta)    beta = val;
        }
        if (beta <= alpha) break;
    }

    transTable[key] = bestVal;
    return bestVal;
}

int alphaBeta(const GameState& state,
    int depth,
    char originalPlayer,
    int alpha,
    int beta)
{
    transTable.clear();
    return alphaBeta_impl(state, depth, originalPlayer, alpha, beta);
}

//-----------------------------------------------------------------------------
// Find best move at root
//-----------------------------------------------------------------------------
Move findBestMove(const GameState& state, char originalPlayer) {
    transTable.clear();
    auto moves = getAllPossibleMoves(state, originalPlayer);
    int bestScore = numeric_limits<int>::min();
    Move bestMove{ -1,-1,-1,-1 };

    for (auto& mv : moves) {
        GameState nxt = applyMove(state, mv, originalPlayer);
        if (isWinningState(nxt, originalPlayer))
            return mv;
        int score = alphaBeta(nxt, 1, originalPlayer,
            numeric_limits<int>::min(),
            numeric_limits<int>::max());
        if (score > bestScore) {
            bestScore = score;
            bestMove = mv;
        }
    }
    return bestMove;
}

//-----------------------------------------------------------------------------
// Utilities & C‑API
//-----------------------------------------------------------------------------
void printBoard(const vector<vector<char>>& B) {
    for (auto& row : B) {
        for (char c : row) cout << c << ' ';
        cout << '\n';
    }
}

vector<int> GetBestMove(vector<vector<char>> board, char turn) {
    GameState s{ board, turn };
    if (isWinningState(s, turn)) return { 0 };
    Move mv = findBestMove(s, turn);
    if (mv.fromRow < 0) return { -1 };
    return { mv.fromRow, mv.fromCol, mv.toRow, mv.toCol };
}
