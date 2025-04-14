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
    currentTurn = nullptr;  // Reset current turn
    gameEnded = false;      // Reset game state

    this->components = gcnew System::ComponentModel::Container();
    this->tableLayoutPanel1 = gcnew TableLayoutPanel();

    // Set up a 5x5 grid.
    this->tableLayoutPanel1->ColumnCount = 5;
    this->tableLayoutPanel1->RowCount = 5;
    this->tableLayoutPanel1->Dock = DockStyle::Fill;

    for (int col = 0; col < 5; col++) {
        this->tableLayoutPanel1->ColumnStyles->Add(gcnew ColumnStyle(SizeType::Percent, 20.0F));
        this->tableLayoutPanel1->RowStyles->Add(gcnew RowStyle(SizeType::Percent, 20.0F));
    }

    // (Optional) Define your color mapping to style the board.
    array<array<Color>^>^ colorMapping = gcnew array<array<Color>^>(5);
    colorMapping[0] = gcnew array<Color>{ Color::Gray, Color::LightGreen, Color::LightGreen, Color::LightGreen, Color::Gray };
    colorMapping[1] = gcnew array<Color>{ Color::Pink, Color::White, Color::White, Color::White, Color::Pink };
    colorMapping[2] = gcnew array<Color>{ Color::Pink, Color::White, Color::White, Color::White, Color::Pink };
    colorMapping[3] = gcnew array<Color>{ Color::Pink, Color::White, Color::White, Color::White, Color::Pink };
    colorMapping[4] = gcnew array<Color>{ Color::Gray, Color::LightGreen, Color::LightGreen, Color::LightGreen, Color::Gray };

    // Create cells and place pieces according to new rules.
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            Panel^ panelCell = gcnew Panel();
            panelCell->Dock = DockStyle::Fill;
            panelCell->Margin = System::Windows::Forms::Padding(0);
            panelCell->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            panelCell->BackColor = colorMapping[r][c];

            // Place green pieces in the top row (row 0) and columns 1, 2, 3.
            if (r == 0 && c >= 1 && c <= 3) {
                PictureBox^ piecePic = gcnew PictureBox();
                piecePic->Image = Image::FromFile("C:\\Users\\hassa\\OneDrive\\Desktop\\Green.png");
                piecePic->SizeMode = PictureBoxSizeMode::StretchImage;
                piecePic->Dock = DockStyle::Fill;
                // Use the Name property to store the piece type.
                piecePic->Name = "green";
                // Store the grid position in the Tag.
                piecePic->Tag = Point(c, r);
                piecePic->Click += gcnew EventHandler(this, &MyForm::piece_Click);
                panelCell->Controls->Add(piecePic);
            }

            // Place red pieces in the first column (column 0) and rows 1, 2, 3.
            if (c == 0 && r >= 1 && r <= 3) {
                PictureBox^ piecePic = gcnew PictureBox();
                piecePic->Image = Image::FromFile("C:\\Users\\hassa\\OneDrive\\Desktop\\Red.png");
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

void MyForm::piece_Click(Object^ sender, EventArgs^ e)
{
    PictureBox^ piece = safe_cast<PictureBox^>(sender);
    String^ pieceType = piece->Name; // "green" or "red"

    // If no turn has been set yet, let the first click decide the starting turn.
    if (currentTurn == nullptr)
    {
        currentTurn = pieceType;
        // Optionally, update UI to reflect that the starting turn is set.
    }

    // Only proceed if it is this piece's turn.
    if (!pieceType->Equals(currentTurn))
    {
        // Optionally, provide feedback (such as a message) that it's not this piece's turn.
        return;
    }

    // Retrieve the current position stored in Tag.
    Point curPos = safe_cast<Point>(piece->Tag);
    int curCol = curPos.X;
    int curRow = curPos.Y;

    // Determine movement direction based on piece type.
    int dCol = 0, dRow = 0;
    if (pieceType->Equals("green"))
    {
        // Green pieces only move downward.
        dRow = 1;
    }
    else if (pieceType->Equals("red"))
    {
        // Red pieces only move to the right.
        dCol = 1;
    }

    // First, try a normal move to the adjacent cell in the allowed direction.
    int nextCol = curCol + dCol;
    int nextRow = curRow + dRow;

    // Check if the adjacent cell is within bounds.
    if (nextCol < 0 || nextCol >= this->tableLayoutPanel1->ColumnCount ||
        nextRow < 0 || nextRow >= this->tableLayoutPanel1->RowCount)
    {
        return; // Move not possible - out of bounds.
    }

    Control^ adjacentCell = this->tableLayoutPanel1->GetControlFromPosition(nextCol, nextRow);
    if (adjacentCell->Controls->Count == 0)
    {
        // The adjacent cell is empty; perform a normal move.
        Control^ currentCell = piece->Parent;
        currentCell->Controls->Remove(piece);
        adjacentCell->Controls->Add(piece);
        piece->Tag = Point(nextCol, nextRow);

        CheckWinCondition();
    }
    else
    {
        // Attempt to jump over the piece in the adjacent cell.
        int landingCol = nextCol + dCol;
        int landingRow = nextRow + dRow;

        // Check if the landing cell is within bounds.
        if (landingCol < 0 || landingCol >= this->tableLayoutPanel1->ColumnCount ||
            landingRow < 0 || landingRow >= this->tableLayoutPanel1->RowCount)
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
        else
        {
            // Landing cell is occupied; invalid move.
            return;
        }
    }

    // Swap the turn after a successful move.
    if (currentTurn->Equals("red"))
        currentTurn = "green";
    else
        currentTurn = "red";

    // Optionally, update any UI elements (like a label) to reflect the new turn.
}

void FSPG::MyForm::CheckWinCondition()
{
    int greenCount = 0;
    int redCount = 0;

    int totalRows = this->tableLayoutPanel1->RowCount;
    int totalCols = this->tableLayoutPanel1->ColumnCount;

    for (int r = 0; r < totalRows; r++)
    {
        for (int c = 0; c < totalCols; c++)
        {
            Control^ cell = this->tableLayoutPanel1->GetControlFromPosition(c, r);
            for each (Control ^ ctrl in cell->Controls)
            {
                PictureBox^ piece = dynamic_cast<PictureBox^>(ctrl);
                if (piece != nullptr)
                {
                    Point pos = safe_cast<Point>(piece->Tag);
                    int pieceCol = pos.X;
                    int pieceRow = pos.Y;

                    if (piece->Name->Equals("green") && pieceRow == totalRows - 1)
                        greenCount++;

                    if (piece->Name->Equals("red") && pieceCol == totalCols - 1)
                        redCount++;
                }
            }
        }
    }

    if (greenCount == 3 && !gameEnded)
    {
        gameEnded = true;
        MessageBox::Show("Green wins!");
    }
    else if (redCount == 3 && !gameEnded)
    {
        gameEnded = true;
        MessageBox::Show("Red wins!");
    }
}




