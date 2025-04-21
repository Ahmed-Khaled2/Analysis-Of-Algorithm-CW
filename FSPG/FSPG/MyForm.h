// MyForm.h
#pragma once

#include <vector>

namespace FSPG {
    using namespace System;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;

    public ref class MyForm : public Form {
    public:
        MyForm(void);
    protected:
        ~MyForm();
    private:
        ::System::ComponentModel::IContainer^ components;
        TableLayoutPanel^ tableLayoutPanel1;
        String^ currentTurn;
        bool                           gameEnded;

        void InitializeComponent(void);
        void piece_Click(Object^ sender, EventArgs^ e);
        void CheckWinCondition();
        std::vector<std::vector<char>> GetBoardState();
    };
}
