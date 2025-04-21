// MyForm.cpp
#include "MyForm.h"
#include "GameSolver.h"
#include <Windows.h>
#include <iostream>

using namespace FSPG;
using namespace System;
using namespace System::Windows::Forms;
using namespace System::Drawing;

MyForm::MyForm(void)
{
    InitializeComponent();
    currentTurn = nullptr;
    gameEnded = false;

    AllocConsole();
    FILE* conOut = nullptr;
    freopen_s(&conOut, "CONOUT$", "w", stdout);
    std::cout << "=== Fake Sugar Pack Console ===\n";
}

MyForm::~MyForm(void)
{
    if (components) delete components;
}

void MyForm::InitializeComponent(void)
{
    components = gcnew ::System::ComponentModel::Container();
    auto resources = gcnew ::System::ComponentModel::ComponentResourceManager(MyForm::typeid);

    tableLayoutPanel1 = gcnew TableLayoutPanel();
    tableLayoutPanel1->Dock = DockStyle::Fill;
    tableLayoutPanel1->ColumnCount = 5;
    tableLayoutPanel1->RowCount = 5;
    for (int i = 0; i < 5; ++i) {
        tableLayoutPanel1->ColumnStyles->Add(
            gcnew ColumnStyle(SizeType::Percent, 20.0F));
        tableLayoutPanel1->RowStyles->Add(
            gcnew RowStyle(SizeType::Percent, 20.0F));
    }

    array<array<Color>^>^ colorMap = gcnew array<array<Color>^>(5) {
        gcnew array<Color>{ Color::Gray, Color::LightGreen, Color::LightGreen, Color::LightGreen, Color::Gray },
            gcnew array<Color>{ Color::Pink, Color::White, Color::White, Color::White, Color::Pink },
            gcnew array<Color>{ Color::Pink, Color::White, Color::White, Color::White, Color::Pink },
            gcnew array<Color>{ Color::Pink, Color::White, Color::White, Color::White, Color::Pink },
            gcnew array<Color>{ Color::Gray, Color::LightGreen, Color::LightGreen, Color::LightGreen, Color::Gray }
    };

    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 5; ++c) {
            Panel^ cell = gcnew Panel();
            cell->Dock = DockStyle::Fill;
            cell->Margin = ::System::Windows::Forms::Padding(0);
            cell->BorderStyle = BorderStyle::FixedSingle;
            cell->BackColor = colorMap[r][c];

            if (r == 0 && c >= 1 && c <= 3) {
                PictureBox^ pic = gcnew PictureBox();
                pic->Image = (Image^)resources->GetObject("Green");
                pic->SizeMode = PictureBoxSizeMode::StretchImage;
                pic->Dock = DockStyle::Fill;
                pic->Name = "green";
                pic->Tag = Drawing::Point(c, r);
                pic->Click += gcnew EventHandler(this, &MyForm::piece_Click);
                cell->Controls->Add(pic);
            }
            if (c == 0 && r >= 1 && r <= 3) {
                PictureBox^ pic = gcnew PictureBox();
                pic->Image = (Image^)resources->GetObject("Red");
                pic->SizeMode = PictureBoxSizeMode::StretchImage;
                pic->Dock = DockStyle::Fill;
                pic->Name = "red";
                pic->Tag = Drawing::Point(c, r);
                pic->Click += gcnew EventHandler(this, &MyForm::piece_Click);
                cell->Controls->Add(pic);
            }

            tableLayoutPanel1->Controls->Add(cell);
            tableLayoutPanel1->SetColumn(cell, c);
            tableLayoutPanel1->SetRow(cell, r);
        }
    }

    this->ClientSize = Drawing::Size(600, 600);
    this->Controls->Add(tableLayoutPanel1);
    this->Text = "Fake Sugar Pack";
}

void MyForm::piece_Click(Object^ sender, EventArgs^ e)
{
    if (gameEnded) return;

    // 1) Determine human vs AI
    PictureBox^ clicked = safe_cast<PictureBox^>(sender);
    String^ humanSide = clicked->Name;
    char       humanChar = (humanSide == "red" ? 'R' : 'G');
    String^ aiSide = (humanSide == "red" ? "green" : "red");
    char       aiChar = (aiSide == "red" ? 'R' : 'G');

    // 2) Init turn
    if (currentTurn == nullptr) currentTurn = humanSide;

    // 3) Not human's turn? ignore
    if (!currentTurn->Equals(humanSide)) return;

    // 4) Human legal moves?
    auto board0 = GetBoardState();
    auto hm = getAllPossibleMoves({ board0,humanChar }, humanChar);

    // 5) Skip human if no moves
    if (hm.empty()) {
        std::cout << "[HUMAN has no legal moves -> skip]\n";
        currentTurn = aiSide;
        tableLayoutPanel1->Refresh();
        Application::DoEvents();
    }
    else {
        // 6) Perform human move
        auto pt = safe_cast<Point>(clicked->Tag);
        int col = pt.X, row = pt.Y, dcol = (humanSide == "red"), drow = (humanSide == "green");
        bool moved = false;
        int nc = col + dcol, nr = row + drow;
        if (nc >= 0 && nc < 5 && nr >= 0 && nr < 5) {
            auto adj = tableLayoutPanel1->GetControlFromPosition(nc, nr);
            if (adj->Controls->Count == 0) {
                auto src = clicked->Parent; src->Controls->Remove(clicked);
                adj->Controls->Add(clicked); clicked->Tag = Point(nc, nr);
                moved = true;
            }
            else {
                int lc = nc + dcol, lr = nr + drow;
                if (lc >= 0 && lc < 5 && lr >= 0 && lr < 5) {
                    auto land = tableLayoutPanel1->GetControlFromPosition(lc, lr);
                    if (land->Controls->Count == 0) {
                        auto src = clicked->Parent; src->Controls->Remove(clicked);
                        land->Controls->Add(clicked); clicked->Tag = Point(lc, lr);
                        moved = true;
                    }
                }
            }
        }
        if (!moved) return;

        // 7) Check human win
        CheckWinCondition();
        if (gameEnded) { std::cout << "[HUMAN wins!]\n"; return; }

        // 8) Refresh & log
        tableLayoutPanel1->Refresh(); Application::DoEvents();
        std::cout << "\n[After HUMAN move]\n"; printBoard(GetBoardState());

        currentTurn = aiSide;
    }

    // === AI block ===
    tableLayoutPanel1->Refresh(); Application::DoEvents();
    auto board1 = GetBoardState();
    auto aiMv = GetBestMove(board1, aiChar);

    if (aiMv.size() == 4 && aiMv[0] != -1) {
        std::cout << "[AI chooses] (" << aiMv[0] << "," << aiMv[1] << ")->(" << aiMv[2] << "," << aiMv[3] << ")\n";
        auto from = tableLayoutPanel1->GetControlFromPosition(aiMv[1], aiMv[0]);
        if (from->Controls->Count > 0) {
            auto pic = safe_cast<PictureBox^>(from->Controls[0]);
            auto to = tableLayoutPanel1->GetControlFromPosition(aiMv[3], aiMv[2]);
            from->Controls->Remove(pic); to->Controls->Add(pic);
            pic->Tag = Point(aiMv[3], aiMv[2]);
            tableLayoutPanel1->Refresh(); Application::DoEvents();
        }
    }
    else {
        std::cout << "[AI has no legal moves -> skip]\n";
    }

    // 12) Check AI win
    CheckWinCondition();
    if (gameEnded) { std::cout << "[AI wins!]\n"; return; }

    // 13) Log final
    std::cout << "[After AI move]\n"; printBoard(GetBoardState());

    // 14) Back to human
    currentTurn = humanSide;
}

void MyForm::CheckWinCondition()
{
    int G = 0, R = 0;
    for (int i = 1; i <= 3; i++) {
        auto g = tableLayoutPanel1->GetControlFromPosition(i, 4);
        auto r = tableLayoutPanel1->GetControlFromPosition(4, i);
        if (g->Controls->Count > 0) G++;
        if (r->Controls->Count > 0) R++;
    }
    if (G == 3) { MessageBox::Show("Green wins!"); gameEnded = true; }
    else if (R == 3) { MessageBox::Show("Red wins!"); gameEnded = true; }
}

std::vector<std::vector<char>> MyForm::GetBoardState()
{
    std::vector<std::vector<char>> B(5, std::vector<char>(5, '.'));
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            auto cell = tableLayoutPanel1->GetControlFromPosition(c, r);
            if (cell->Controls->Count > 0) {
                auto pic = safe_cast<PictureBox^>(cell->Controls[0]);
                B[r][c] = pic->Name->Equals("green") ? 'G' : 'R';
            }
        }
    }
    return B;
}
