#pragma once
#include "MyForm.h"
#include "GameSolver.h"
#include <iostream>

using namespace FSPG;
using namespace System;
using namespace System::Windows::Forms;
using namespace System::Drawing;

MyForm::MyForm(void){
    InitializeComponent();
}

MyForm::~MyForm(void){
    if (components) delete components;
}

void MyForm::InitializeComponent(void){
    currentTurn = nullptr;
    gameEnded = false;
    components = gcnew::System::ComponentModel::Container();
    auto resources = gcnew::System::ComponentModel::ComponentResourceManager(MyForm::typeid);

    tableLayoutPanel1 = gcnew TableLayoutPanel();
    tableLayoutPanel1->ColumnCount = 5;
    tableLayoutPanel1->RowCount = 5;
    tableLayoutPanel1->Dock = DockStyle::Fill;
    tableLayoutPanel1->Margin = ::Padding(0);
    tableLayoutPanel1->Padding = ::Padding(0);

    for (int i = 0; i < 5; ++i) {
        tableLayoutPanel1->ColumnStyles->Add(gcnew ColumnStyle(SizeType::Percent, 20.0F));
        tableLayoutPanel1->RowStyles->Add(gcnew RowStyle(SizeType::Percent, 20.0F));
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

    Panel^ gridContainer = gcnew Panel();
    gridContainer->Size = Drawing::Size(600, 600);
    gridContainer->MinimumSize = Drawing::Size(600, 600);
    gridContainer->MaximumSize = Drawing::Size(600, 600);
    gridContainer->Dock = DockStyle::Fill;
    gridContainer->Margin = ::Padding(0);
    gridContainer->Padding = ::Padding(0);
    gridContainer->Controls->Add(tableLayoutPanel1);

    TableLayoutPanel^ sidebar = gcnew TableLayoutPanel();
    sidebar->Dock = DockStyle::Fill;
    sidebar->ColumnCount = 1;
    sidebar->RowCount = 3;
    sidebar->RowStyles->Add(gcnew RowStyle(SizeType::Absolute, 60));
    sidebar->RowStyles->Add(gcnew RowStyle(SizeType::Percent, 100));  
    sidebar->RowStyles->Add(gcnew RowStyle(SizeType::Absolute, 24)); 
    sidebar->BackColor = Color::LightGray;
    sidebar->Margin = ::Padding(0);
    sidebar->Padding = ::Padding(0);

    Button^ restartButton = gcnew Button();
    restartButton->Text = "Restart Game";
    restartButton->Dock = DockStyle::Fill;
    restartButton->Font = gcnew System::Drawing::Font("Segoe UI", 12, FontStyle::Bold);
    restartButton->Click += gcnew EventHandler(this, &MyForm::restartButton_Click);

    Panel^ logPanel = gcnew Panel();
    logPanel->Dock = DockStyle::Fill;
    logPanel->AutoScroll = true;
    logPanel->BackColor = Color::White;
    logPanel->Height = 300;
    logPanel->Dock = DockStyle::Top;

    logLabel = gcnew Label();
    logLabel->AutoSize = true;
    logLabel->Font = gcnew Drawing::Font("Consolas", 10);
    logLabel->Text = "=== Fake Sugar Pack Console ===\r\nPick any color to start the game.\n";
    logPanel->Controls->Add(logLabel);
    
    statusLabel = gcnew Label();
    statusLabel->Text = "Current Turn: None";
    statusLabel->Dock = DockStyle::Fill;
    statusLabel->TextAlign = ContentAlignment::MiddleLeft;
    statusLabel->BackColor = Color::White;
    statusLabel->Font = gcnew Drawing::Font("Segoe UI", 12, FontStyle::Regular);
    sidebar->Controls->Add(restartButton, 0, 0);
    sidebar->Controls->Add(logPanel, 0, 1);
    sidebar->Controls->Add(statusLabel, 0, 2);
    
    TableLayoutPanel^ mainLayout = gcnew TableLayoutPanel();
    mainLayout->Dock = DockStyle::Fill;
    mainLayout->ColumnCount = 2;
    mainLayout->RowCount = 1;
    mainLayout->ColumnStyles->Add(gcnew ColumnStyle(SizeType::AutoSize));         
    mainLayout->ColumnStyles->Add(gcnew ColumnStyle(SizeType::Percent, 100.0F)); 
    mainLayout->Controls->Add(gridContainer, 0, 0);
    mainLayout->Controls->Add(sidebar, 1, 0);

    this->ClientSize = Drawing::Size(1000, 600);
    this->Controls->Add(mainLayout);
    this->Text = "Fake Sugar Pack";
}

void MyForm::restartButton_Click(Object^ sender, EventArgs^ e) {
    this->Controls->Clear();
    InitializeComponent();
}

void MyForm::piece_Click(Object^ sender, EventArgs^ e){
    if (gameEnded) return;
    PictureBox^ clicked = safe_cast<PictureBox^>(sender);
    String^ humanSide = clicked->Name;
    String^ aiSide;
    char humanChar, aiChar;
    if (humanSide == "red") {
        humanChar = 'R'; aiSide = "green"; aiChar = 'G';
    }
    else {
        humanChar = 'G'; aiSide = "red"; aiChar = 'R';
    }

    if (currentTurn == nullptr) {
        currentTurn = humanSide;
    }

    if (!currentTurn->Equals(humanSide)) {
        return;
    }

    auto board0 = GetBoardState();
    auto hm = getAllPossibleMoves({ board0,humanChar }, humanChar);

    if (hm.empty()) {
        logLabel->Text += "[HUMAN has no legal moves -> skip]\n";
        currentTurn = aiSide;
    }
    else {
        statusLabel->Text = String::Format("Current Turn: {0}", System::Char(humanChar));
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
        if (!moved) {
            return;
        }

        CheckWinCondition();
        if (gameEnded) {
            logLabel->Text += "[HUMAN wins!]\n";
            return;
        }
        currentTurn = aiSide;
    }

    auto board1 = GetBoardState();
    auto aiMv = GetBestMove(board1, aiChar);

    if (aiMv.size() == 4 && aiMv[0] != -1) {
        logLabel->Text += String::Format("[AI chooses] ({0},{1}) -> ({2},{3})\r\n", aiMv[0], aiMv[1], aiMv[2], aiMv[3]);
        auto from = tableLayoutPanel1->GetControlFromPosition(aiMv[1], aiMv[0]);
        if (from->Controls->Count > 0) {
            auto pic = safe_cast<PictureBox^>(from->Controls[0]);
            auto to = tableLayoutPanel1->GetControlFromPosition(aiMv[3], aiMv[2]);
            from->Controls->Remove(pic); to->Controls->Add(pic);
            pic->Tag = Point(aiMv[3], aiMv[2]);
        }
    }
    else {
        logLabel->Text += "[AI has no legal moves -> skip]\n";
    }

    CheckWinCondition();
    if (gameEnded) {
        logLabel->Text += "[AI wins!]\n";
        return;
    }

    currentTurn = humanSide;
}

void MyForm::CheckWinCondition(){
    int G = 0, R = 0;
    for (int i = 1; i <= 3; i++) {
        auto g = tableLayoutPanel1->GetControlFromPosition(i, 4);
        auto r = tableLayoutPanel1->GetControlFromPosition(4, i);
        if (g->Controls->Count > 0) G++;
        if (r->Controls->Count > 0) R++;
    }
    if (G == 3) {
        gameEnded = true;
    }
    else if (R == 3) {
        gameEnded = true;
    }
}

std::vector<std::vector<char>> MyForm::GetBoardState(){
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
