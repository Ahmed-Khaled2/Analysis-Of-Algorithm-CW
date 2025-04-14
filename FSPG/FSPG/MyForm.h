#pragma once

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

        void MyForm::piece_Click(Object^ sender, EventArgs^ e);
    };
}
