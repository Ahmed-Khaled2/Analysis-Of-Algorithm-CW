#include "MyForm.h"

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

void MyForm::InitializeComponent(void)
{
    this->components = gcnew System::ComponentModel::Container();
    this->tableLayoutPanel1 = gcnew TableLayoutPanel();

    this->tableLayoutPanel1->ColumnCount = 5;
    this->tableLayoutPanel1->RowCount = 5;
    this->tableLayoutPanel1->Dock = DockStyle::Fill;

    for (int col = 0; col < 5; col++){
        this->tableLayoutPanel1->ColumnStyles->Add(gcnew ColumnStyle(SizeType::Percent, 20.0F));
        this->tableLayoutPanel1->RowStyles->Add(gcnew RowStyle(SizeType::Percent, 20.0F));
    }

    array<array<Color>^>^ colorMapping = gcnew array<array<Color>^>(5);
    colorMapping[0] = gcnew array<Color>{ Color::Gray, Color::LightGreen, Color::LightGreen, Color::LightGreen, Color::Gray };
    colorMapping[1] = gcnew array<Color>{ Color::Pink, Color::White, Color::White, Color::White, Color::Pink };
    colorMapping[2] = gcnew array<Color>{ Color::Pink, Color::White, Color::White, Color::White, Color::Pink };
    colorMapping[3] = gcnew array<Color>{ Color::Pink, Color::White, Color::White, Color::White, Color::Pink };
    colorMapping[4] = gcnew array<Color>{ Color::Gray, Color::LightGreen, Color::LightGreen, Color::LightGreen, Color::Gray };

    for (int r = 0; r < 5; r++){
        for (int c = 0; c < 5; c++){
           
            Panel^ panelCell = gcnew Panel();
            panelCell->Dock = DockStyle::Fill;
            panelCell->Margin = System::Windows::Forms::Padding(0);
            panelCell->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            panelCell->BackColor = colorMapping[r][c];
            
            if (r == 2 && c == 2){
                PictureBox^ piecePic = gcnew PictureBox();
                
                piecePic->Image = Image::FromFile("C:\\Users\\gosta\\Desktop\\Screenshot 2025-04-14 130728.png");
                piecePic->SizeMode = PictureBoxSizeMode::StretchImage;
                piecePic->Dock = DockStyle::Fill;
                piecePic->Tag = Point(c, r);
                piecePic->Click += gcnew EventHandler(this, &MyForm::piece_Click);
                panelCell->Controls->Add(piecePic);
            }
            
            this->tableLayoutPanel1->Controls->Add(panelCell, c, r);
        }
    }

    this->ClientSize = Drawing::Size(600, 600);
    this->Controls->Add(this->tableLayoutPanel1);
    this->Text = L"MyForm (Simple Board)";
}

void MyForm::piece_Click(Object^ sender, EventArgs^ e)
{
    PictureBox^ piece = safe_cast<PictureBox^>(sender);
    // Retrieve current position stored in Tag; expect it to be a Point.
    Point curPos = safe_cast<Point>(piece->Tag);
    int curCol = curPos.X;
    int curRow = curPos.Y;

    // Calculate the "next" cell position.
    // For example, move right one cell, wrapping to the next row if necessary.
    int nextCol = curCol + 1;
    int nextRow = curRow;
    if (nextCol >= this->tableLayoutPanel1->ColumnCount)
    {
        nextCol = 0;
        nextRow++;
    }
    // Ensure we have not exceeded the board.
    if (nextRow >= this->tableLayoutPanel1->RowCount)
    {
        // Alternatively, you might loop back to the beginning.
        nextRow = 0;
    }

    // Obtain the control (Panel) at the next cell's position.
    Control^ nextCell = this->tableLayoutPanel1->GetControlFromPosition(nextCol, nextRow);
    if (nextCell != nullptr)
    {
        // Remove the piece from its current parent.
        Control^ currentCell = piece->Parent;
        currentCell->Controls->Remove(piece);

        // Add the piece to the next cell.
        nextCell->Controls->Add(piece);

        // Update the piece's tag with the new grid position.
        piece->Tag = Point(nextCol, nextRow);
    }
}

