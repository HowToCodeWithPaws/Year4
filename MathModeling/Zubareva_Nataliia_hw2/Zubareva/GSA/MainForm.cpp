#include "MainForm.h"
#include <Windows.h>

using namespace GSA;

[STAThread]
void main(array<String^>^ args)
{
    Application::EnableVisualStyles();

    Application::SetCompatibleTextRenderingDefault(false);

    GSA::MainForm form;

    Application::Run(% form);

}