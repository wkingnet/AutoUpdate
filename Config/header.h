#pragma once
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <Shlobj.h>
#include <ShlObj_core.h>
#include <string>
#include <tchar.h>
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#include "xini_file.h"
#include "tinyxml2.h"

// 开启可视化效果
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#ifdef _DEBUG
#define IS_DEBUG 1
#else
#define IS_DEBUG 0
#endif

INT_PTR CALLBACK ProcConfig(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Cls_OnSysCommand(HWND hwnd, UINT cmd, int x, int y);
void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);