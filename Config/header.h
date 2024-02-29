#pragma once
#include <Windows.h>
#include <Windowsx.h>


// 开启可视化效果
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

INT_PTR CALLBACK ProcConfig(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);