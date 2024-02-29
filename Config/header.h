#pragma once
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#include <Shlobj.h>
#include <string>
#include <iostream>
#include <tchar.h>
#include <algorithm>


// 开启可视化效果
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

void LastError();
std::wstring calc_crc32(const BYTE* ptr, DWORD Size);

INT_PTR CALLBACK ProcConfig(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Cls_OnSysCommand(HWND hwnd, UINT cmd, int x, int y);
void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

/**
 * \brief Convert an wide Unicode string to ANSI string
 * \param wstr 输入的wstring字符串
 * \return string
 * \note 转换后末尾有\0终止符，比较字符串时注意。再加.c_str()可消除\0终止符
 */
inline std::string unicode2ansi(const std::wstring& wstr) {
  const int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.data(), -1, nullptr, 0, nullptr, nullptr);
  std::string strTo(size_needed, 0);
  WideCharToMultiByte(CP_ACP, 0, wstr.data(), (int)wstr.size(), strTo.data(), size_needed, nullptr, nullptr);
  return strTo;
}

// Convert an ANSI string to a wide Unicode String
inline std::wstring ansi2unicode(const std::string& str) {
  const int size_needed = MultiByteToWideChar(CP_ACP, 0, str.data(), (int)str.size(), nullptr, 0);
  std::wstring wstrTo(size_needed, 0);
  MultiByteToWideChar(CP_ACP, 0, str.data(), (int)str.size(), wstrTo.data(), size_needed);
  return wstrTo;
}

