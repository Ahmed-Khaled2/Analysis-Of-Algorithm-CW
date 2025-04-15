#include <iostream>
#include <vector>
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

bool isWinningState(const GameState& state, char player) {
    int count = 0;
    int numRows = int(state.board.size());
    int numCols = int(state.board[0].size());

    if (player == 'G') {
        int targetRow = numRows - 1;
        for (int c = 0; c < numCols; c++) {
            if (state.board[targetRow][c] == 'G') count++;
        }
    }
    else {
        int targetCol = numCols - 1;
        for (int r = 0; r < numRows; r++) {
            if (state.board[r][targetCol] == 'R') count++;
        }
    }

    return count >= 3;
}

bool isLosingState(const GameState& state, char player) {
    return isWinningState(state, getOpponent(player));
}

vector<Move> getAllPossibleMoves(const GameState& state, char player) {
    vector<Move> moves;
    int numRows = int(state.board.size());
    int numCols = int(state.board[0].size());

    for (int r = 0; r < numRows; r++) {
        for (int c = 0; c < numCols; c++) {
            if (state.board[r][c] == player) {
                if (player == 'G') {
                    // Green moves DOWN
                    if (r + 1 < numRows && state.board[r + 1][c] == '.') {
                        moves.push_back({ r, c, r + 1, c });
                    }
                    // Jump over R
                    if (r + 2 < numRows && state.board[r + 1][c] == 'R' && state.board[r + 2][c] == '.') {
                        moves.push_back({ r, c, r + 2, c });
                    }
                }
                else if (player == 'R') {
                    // Red moves RIGHT
                    if (c + 1 < numCols && state.board[r][c + 1] == '.') {
                        moves.push_back({ r, c, r, c + 1 });
                    }
                    // Jump over G
                    if (c + 2 < numCols && state.board[r][c + 1] == 'G' && state.board[r][c + 2] == '.') {
                        moves.push_back({ r, c, r, c + 2 });
                    }
                }
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

string PlayAnyGame(const GameState& state, char player) {
    if (isWinningState(state, player)) return "good";
    if (isLosingState(state, player)) return "bad";

    for (const Move& move : getAllPossibleMoves(state, player)) {
        GameState newState = applyMove(state, move, player);
        string result = PlayAnyGame(newState, getOpponent(player));
        if (result == "bad") return "good";
    }

    return "bad";
}

Move findBestMove(const GameState& state, char player) {
    vector<Move> allMoves = getAllPossibleMoves(state, player);
    Move fallbackMove = { -1, -1, -1, -1 };  // if no move is found

    for (const Move& move : allMoves) {
        GameState newState = applyMove(state, move, player);
        string result = PlayAnyGame(newState, getOpponent(player));
        if (result == "bad") {
            return move; // Best move: leads to win
        }

        // Save fallback if none are good
        if (fallbackMove.fromRow == -1) {
            fallbackMove = move;
        }
    }

    // Return fallback move if no good move found
    return fallbackMove;
}

void printBoard(const vector<vector<char>>& board) {
    for (const auto& row : board) {
        for (char cell : row) {
            cout << cell << " ";
        }
        cout << endl;
    }
}

void printMove(const Move& move) {
    if (move.fromRow == -1) {
        cout << "No winning move found.\n";
    }
    else {
        cout << "Move from (" << move.fromRow << ", " << move.fromCol << ") to ("
            << move.toRow << ", " << move.toCol << ")\n";
    }
}

string GetBestMove(vector<vector<char>> board, char turn) {

    GameState initialState;
    initialState.board = board;
    initialState.currentPlayer = turn;

    cout << "Initial board:\n";
    printBoard(initialState.board);
    cout << "\nCurrent player: " << initialState.currentPlayer << "\n\n";

    Move bestMove = findBestMove(initialState, initialState.currentPlayer);

    cout << "Best move for player '" << initialState.currentPlayer << "':\n";
    printMove(bestMove);
    return "Hello";
}