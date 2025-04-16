#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <limits>
using namespace std;

struct Move {
    int fromRow, fromCol;
    int toRow, toCol;
};

struct GameState {
    vector<vector<char>> board;
    char currentPlayer;
};

char getOpponent(char player) {
    return player == 'R' ? 'G' : 'R';
}

bool isInGoalZone(int row, int col, char player) {
    if (player == 'R')
        return (row >= 1 && row <= 3 && col == 4);
    if (player == 'G')
        return (row == 4 && col >= 1 && col <= 3);
    return false;
}

bool isWinningState(const GameState& state, char player) {
    int count = 0;
    for (int r = 0; r < state.board.size(); ++r)
        for (int c = 0; c < state.board[0].size(); ++c)
            if (state.board[r][c] == player && isInGoalZone(r, c, player))
                count++;
    return count >= 3;
}

bool isLosingState(const GameState& state, char player) {
    return isWinningState(state, getOpponent(player));
}

vector<Move> getAllPossibleMoves(const GameState& state, char player) {
    vector<Move> moves;
    int rows = state.board.size();
    int cols = state.board[0].size();

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (state.board[r][c] != player)
                continue;
            if (isInGoalZone(r, c, player))
                continue;

            if (player == 'G') {
                if (r + 1 < rows && (state.board[r + 1][c] == '.' || state.board[r + 1][c] == '*'))
                    moves.push_back({ r, c, r + 1, c });
                if (r + 2 < rows && state.board[r + 1][c] == 'R' && (state.board[r + 2][c] == '.' || state.board[r + 2][c] == '*'))
                    moves.push_back({ r, c, r + 2, c });
            }
            else if (player == 'R') {
                if (c + 1 < cols && (state.board[r][c + 1] == '.' || state.board[r][c + 1] == '*'))
                    moves.push_back({ r, c, r, c + 1 });
                if (c + 2 < cols && state.board[r][c + 1] == 'G' && (state.board[r][c + 2] == '.' || state.board[r][c + 2] == '*'))
                    moves.push_back({ r, c, r, c + 2 });
            }
        }
    }
    return moves;
}

GameState applyMove(const GameState& state, const Move& move, char player) {
    GameState newState = state;
    newState.board[move.toRow][move.toCol] = player;
    newState.board[move.fromRow][move.fromCol] = '.';
    newState.currentPlayer = getOpponent(player);
    return newState;
}

string encodeGameState(const GameState& state, char originalPlayer) {
    stringstream ss;
    for (const auto& row : state.board)
        for (char cell : row)
            ss << cell;
    ss << state.currentPlayer << originalPlayer;
    return ss.str();
}

unordered_map<string, int> memoMinimax;

int minimax(const GameState& state, int depth, char originalPlayer) {
    if (isWinningState(state, originalPlayer))
        return 1000 - depth;
    if (isLosingState(state, originalPlayer))
        return -1000 + depth;

    vector<Move> moves = getAllPossibleMoves(state, state.currentPlayer);
    if (moves.empty())
        return -1000 + depth;

    string key = encodeGameState(state, originalPlayer);
    if (memoMinimax.find(key) != memoMinimax.end())
        return memoMinimax[key];

    int bestScore;
    if (state.currentPlayer == originalPlayer) {
        bestScore = numeric_limits<int>::min();
        for (const Move& move : moves) {
            GameState nextState = applyMove(state, move, state.currentPlayer);
            int score = minimax(nextState, depth + 1, originalPlayer);
            bestScore = max(bestScore, score);
        }
    }
    else {
        bestScore = numeric_limits<int>::max();
        for (const Move& move : moves) {
            GameState nextState = applyMove(state, move, state.currentPlayer);
            int score = minimax(nextState, depth + 1, originalPlayer);
            bestScore = min(bestScore, score);
        }
    }

    memoMinimax[key] = bestScore;
    return bestScore;
}

// Tiebreaker helpers
bool isGoalReaching(const Move& move, char player) {
    return isInGoalZone(move.toRow, move.toCol, player);
}

bool isJumpMove(const Move& move, char player) {
    return (player == 'G' && abs(move.toRow - move.fromRow) == 2) ||
        (player == 'R' && abs(move.toCol - move.fromCol) == 2);
}

bool isBlockingMove(const GameState& before, const Move& move, char player) {
    char opponent = getOpponent(player);
    int r = move.toRow;
    int c = move.toCol;
    if (opponent == 'G' && r > 0 && before.board[r - 1][c] == opponent)
        return true;
    if (opponent == 'R' && c > 0 && before.board[r][c - 1] == opponent)
        return true;
    return false;
}

Move findBestMove(const GameState& state, char originalPlayer) {
    vector<Move> moves = getAllPossibleMoves(state, originalPlayer);
    int bestScore = numeric_limits<int>::min();
    vector<Move> bestMoves;

    memoMinimax.clear();

    for (const Move& move : moves) {
        GameState nextState = applyMove(state, move, originalPlayer);
        if (isWinningState(nextState, originalPlayer)) {
            cout << "Immediate win detected.\n";
            return move;
        }
        int score = minimax(nextState, 1, originalPlayer);
        cout << "Evaluated move (" << move.fromRow << "," << move.fromCol
            << ") -> (" << move.toRow << "," << move.toCol << ") score: " << score << "\n";

        if (score > bestScore) {
            bestScore = score;
            bestMoves = { move };
        }
        else if (score == bestScore) {
            bestMoves.push_back(move);
        }
    }

    // Tiebreaker: prefer goal-reaching moves.
    for (const auto& move : bestMoves)
        if (isGoalReaching(move, originalPlayer)) return move;

    // Tiebreaker: prefer jump moves.
    for (const auto& move : bestMoves)
        if (isJumpMove(move, originalPlayer)) return move;

    // Tiebreaker: prefer blocking moves.
    for (const auto& move : bestMoves)
        if (isBlockingMove(state, move, originalPlayer)) return move;

    // New tie-breaker: avoid moves that allow a jump opportunity for the opponent.
    vector<Move> safeMoves;
    char opponent = getOpponent(originalPlayer);
    for (const auto& move : bestMoves) {
        GameState nextState = applyMove(state, move, originalPlayer);
        vector<Move> oppMoves = getAllPossibleMoves(nextState, opponent);
        bool opponentHasJump = false;
        for (const auto& opMove : oppMoves) {
            if (isJumpMove(opMove, opponent)) {
                opponentHasJump = true;
                break;
            }
        }
        if (!opponentHasJump) {
            safeMoves.push_back(move);
        }
    }
    if (!safeMoves.empty())
        return safeMoves[0];

    return bestMoves[0]; // fallback if no safe moves are found
}

void printBoard(const vector<vector<char>>& board) {
    for (const auto& row : board) {
        for (char cell : row)
            cout << cell << " ";
        cout << endl;
    }
}

void printMove(const Move& move) {
    if (move.fromRow == -1)
        cout << "No legal moves.\n";
    else
        cout << "Move: (" << move.fromRow << "," << move.fromCol
        << ") -> (" << move.toRow << "," << move.toCol << ")\n";
}

string GetBestMove(vector<vector<char>> board, char turn) {
    GameState initialState;
    initialState.board = board;
    initialState.currentPlayer = turn;

    cout << "Initial Board:\n";
    printBoard(initialState.board);
    cout << "Current Player: " << turn << "\n";

    if (isWinningState(initialState, turn)) {
        cout << "Player '" << turn << "' already won!\n";
        return "done";
    }

    Move bestMove = findBestMove(initialState, turn);
    cout << "\nBest Move:\n";
    printMove(bestMove);

    return "done";
}
