#include "MyForm.h"
#include "GameSolver.h"
#include <vector>
#include <iostream>
#include <string>
#include <sstream>

using namespace FSPG;
using namespace System;
using namespace System::Windows::Forms;
using namespace System::Drawing;

MyForm::MyForm(void){
    InitializeComponent();
}

MyForm::~MyForm(void){
    if (components){
        delete components;
    }
}

void MyForm::InitializeComponent(void){
    System::ComponentModel::ComponentResourceManager^ resources =
        (gcnew System::ComponentModel::ComponentResourceManager(MyForm::typeid));

    currentTurn = nullptr;

    this->components = gcnew System::ComponentModel::Container();
    this->tableLayoutPanel1 = gcnew TableLayoutPanel();
    this->tableLayoutPanel1->Dock = DockStyle::Fill;

    for (int col = 0; col < 5; col++) {
        this->tableLayoutPanel1->ColumnStyles->Add(gcnew ColumnStyle(SizeType::Percent, 20.0F));
        this->tableLayoutPanel1->RowStyles->Add(gcnew RowStyle(SizeType::Percent, 20.0F));
    }

    array<array<Color>^>^ colorMapping = gcnew array<array<Color>^>(5);
    colorMapping[0] = gcnew array<Color>{ Color::Gray, Color::LightGreen, Color::LightGreen, Color::LightGreen, Color::Gray };
    colorMapping[1] = gcnew array<Color>{ Color::Pink, Color::White, Color::White, Color::White, Color::Pink };
    colorMapping[2] = gcnew array<Color>{ Color::Pink, Color::White, Color::White, Color::White, Color::Pink };
    colorMapping[3] = gcnew array<Color>{ Color::Pink, Color::White, Color::White, Color::White, Color::Pink };
    colorMapping[4] = gcnew array<Color>{ Color::Gray, Color::LightGreen, Color::LightGreen, Color::LightGreen, Color::Gray };

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            Panel^ panelCell = gcnew Panel();
            panelCell->Dock = DockStyle::Fill;
            panelCell->Margin = System::Windows::Forms::Padding(0);
            panelCell->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            panelCell->BackColor = colorMapping[r][c];
            if (r == 0 && c >= 1 && c <= 3) {
                PictureBox^ piecePic = gcnew PictureBox();
                piecePic->Image = (System::Drawing::Image^)resources->GetObject("Green");
                piecePic->SizeMode = PictureBoxSizeMode::StretchImage;
                piecePic->Dock = DockStyle::Fill;
                piecePic->Name = "green";
                piecePic->Tag = Point(c, r);
                piecePic->Click += gcnew EventHandler(this, &MyForm::piece_Click);
                panelCell->Controls->Add(piecePic);
            }
            if (c == 0 && r >= 1 && r <= 3) {
                PictureBox^ piecePic = gcnew PictureBox();
                piecePic->Image = (System::Drawing::Image^)resources->GetObject("Red");
                piecePic->SizeMode = PictureBoxSizeMode::StretchImage;
                piecePic->Dock = DockStyle::Fill;
                piecePic->Name = "red";
                piecePic->Tag = Point(c, r);
                piecePic->Click += gcnew EventHandler(this, &MyForm::piece_Click);
                panelCell->Controls->Add(piecePic);
            }
            this->tableLayoutPanel1->Controls->Add(panelCell, c, r);
        }
    }

    this->ClientSize = Drawing::Size(600, 600);
    this->Controls->Add(this->tableLayoutPanel1);
    this->Text = L"MyForm (Board with Two Pieces)";
}

void MyForm::piece_Click(Object^ sender, EventArgs^ e){
    PictureBox^ piece = safe_cast<PictureBox^>(sender);
    String^ pieceType = piece->Name; // "green" or "red"
    char LetterTurn;

    // If no turn has been set yet, let the first click decide the starting turn.
    if (currentTurn == nullptr) {
        currentTurn = pieceType;
        // Optionally, update UI to reflect that the starting turn is set.
    }

    // Only proceed if it is this piece's turn.
    if (!pieceType->Equals(currentTurn)) {
        // Optionally, provide feedback (such as a message) that it's not this piece's turn.
        return;
    }

    // Retrieve the current position stored in Tag.
    Point curPos = safe_cast<Point>(piece->Tag);
    int curCol = curPos.X;
    int curRow = curPos.Y;

    // Determine movement direction based on piece type.
    int dCol = 0, dRow = 0;
    if (pieceType->Equals("green")) {
        // Green pieces only move downward.
        dRow = 1;
    }
    else if (pieceType->Equals("red")) {
        // Red pieces only move to the right.
        dCol = 1;
    }

    // First, try a normal move to the adjacent cell in the allowed direction.
    int nextCol = curCol + dCol;
    int nextRow = curRow + dRow;

    // Check if the adjacent cell is within bounds.
    if (nextCol < 0 || nextCol >= 5 || nextRow < 0 || nextRow >= 5) {
        return; // Move not possible - out of bounds.
    }

    Control^ adjacentCell = this->tableLayoutPanel1->GetControlFromPosition(nextCol, nextRow);
    if (adjacentCell->Controls->Count == 0){
        // The adjacent cell is empty; perform a normal move.
        Control^ currentCell = piece->Parent;
        currentCell->Controls->Remove(piece);
        adjacentCell->Controls->Add(piece);
        piece->Tag = Point(nextCol, nextRow);

        CheckWinCondition();
    }
    else{
        // Attempt to jump over the piece in the adjacent cell.
        int landingCol = nextCol + dCol;
        int landingRow = nextRow + dRow;

        // Check if the landing cell is within bounds.
        if (landingCol < 0 || landingCol >= 5 ||
            landingRow < 0 || landingRow >= 5)
        {
            return; // Cannot jump if landing cell is out of bounds.
        }

        Control^ landingCell = this->tableLayoutPanel1->GetControlFromPosition(landingCol, landingRow);
        if (landingCell->Controls->Count == 0)
        {
            // Allowed jump: the landing cell is empty.
            Control^ currentCell = piece->Parent;
            currentCell->Controls->Remove(piece);
            landingCell->Controls->Add(piece);
            piece->Tag = Point(landingCol, landingRow);

            // Check for winner
            CheckWinCondition();
        }
        else{
            // Landing cell is occupied; invalid move.
            return;
        }
    }

    // Swap the turn after a successful move.
    if (currentTurn->Equals("red")) {
        LetterTurn = 'G';
    }
    else {
        LetterTurn = 'R';
    }

    // Optionally, update any UI elements (like a label) to reflect the new turn.

    std::vector<int> ComputerMove = GetBestMove(GetBoardState(), LetterTurn);
    if (ComputerMove[0] == -1) {
        std::cout << "No legal move!";
        // skip turn
    }
    else {

        Control^ fromCell = tableLayoutPanel1->GetControlFromPosition(ComputerMove[1], ComputerMove[0]);
        PictureBox^ piece = safe_cast<PictureBox^>(fromCell->Controls[0]);
        Control^ toCell = tableLayoutPanel1->GetControlFromPosition(ComputerMove[3], ComputerMove[2]);

        fromCell->Controls->Remove(piece);
        toCell->Controls->Add(piece);
        piece->Tag = Point(ComputerMove[3], ComputerMove[2]);

        CheckWinCondition();
    }

    GameState humanState{ GetBoardState(), LetterTurn };
    std::vector<::Move> legal = getAllPossibleMoves(humanState, LetterTurn);

    if (legal.empty()) {
        String^ Color;
        if (LetterTurn == 'G') {
            Color = "green's";
        }
        else {
            Color = "red's";
        }
        MessageBox::Show("No legal moves available — skipping " + Color + " turn.");
    }
}

void MyForm::CheckWinCondition(){
    int Gcount = 0, Rcount = 0;

    for (int i = 1; i <= 3; i++) {
        Control^ Green = this->tableLayoutPanel1->GetControlFromPosition(i, 4);
        Control^ Red = this->tableLayoutPanel1->GetControlFromPosition(4, i);
        if (Green->Controls->Count > 0) {
            Gcount++;
        }
        if (Red->Controls->Count > 0) {
            Rcount++;
        }
    }

    if (Gcount == 3){
        MessageBox::Show("Green wins!");
    }
    else if (Rcount == 3){
        MessageBox::Show("Red wins!");
    }
}

std::vector<std::vector<char>> MyForm::GetBoardState() {
    std::vector<std::vector<char>> board(5, std::vector<char>(5, 0));

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            Control^ cell = this->tableLayoutPanel1->GetControlFromPosition(c, r);
            if (cell->Controls->Count > 0) {
                PictureBox^ piece = safe_cast<PictureBox^>(cell->Controls[0]);
                if (piece->Name->Equals("green"))
                    board[r][c] = 'G';
                else if (piece->Name->Equals("red"))
                    board[r][c] = 'R';
            }
            else {
                board[r][c] = '.';
            }
        }
    }
    return board;
}