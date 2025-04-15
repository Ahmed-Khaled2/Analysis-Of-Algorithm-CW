#pragma once
#include <vector>

namespace FSPG
{
    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;

    public ref class MyForm : public Form
    {
    public:
        MyForm(void);
    protected:
        ~MyForm();
    private:
        // Required designer variable.
        System::ComponentModel::Container^ components;

        // TableLayoutPanel for the board layout.
        TableLayoutPanel^ tableLayoutPanel1;

        // 2D array to store cell references.
        array<Control^, 2>^ cellControls;

        // Method to initialize all components
        void InitializeComponent(void);

        void piece_Click(Object^ sender, EventArgs^ e);

        String^ currentTurn; // Will be "red" or "green", not set initially.

        bool gameEnded = false; // Keeps track of whether the game has ended.

        void CheckWinCondition();

        std::vector<std::vector<char>> GetBoardState();
    };
}
