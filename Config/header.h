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

// 开启可视化效果
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace std;

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

/**
 * \brief Convert a wide Unicode string to an UTF8 string
 * \param wstr 输入的wstring字符串
 * \return string
 * \note 转换后VS显示乱码，无法比较字符串
 */
inline std::string unicode2utf8(const std::wstring& wstr) {
  const int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
  std::string strTo(size_needed, 0);
  WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), strTo.data(), size_needed, nullptr, nullptr);
  return strTo;
  //return boost::locale::conv::utf_to_utf<char>(wstr);
}

/*
@brief GetLastError()封装
@param 错误信息string
*/
inline void LastError() {
  // Retrieve the system error message for the last-error code

  LPVOID lpMsgBuf{};

  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |      // 自动分配缓存
    FORMAT_MESSAGE_FROM_SYSTEM |                // 系统消息
    FORMAT_MESSAGE_IGNORE_INSERTS,
    nullptr,
    GetLastError(),                             // 错误代码
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // 默认语言
    (LPWSTR)&lpMsgBuf,
    0, nullptr);

  // Display the error message and exit the process
  if (lpMsgBuf) {
    MessageBox(nullptr, (LPCWSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION);
  }
  else
    cout << "GetLastError: cannot find this error info.\n";
  LocalFree(lpMsgBuf);
}

inline wstring calc_crc32(const BYTE* ptr, DWORD Size) {
  DWORD crcTable[256], crcTmp1;

  // 动态生成CRC-32表
  for (int i = 0; i < 256; i++) {
    crcTmp1 = i;
    for (int j = 8; j > 0; j--) {
      if (crcTmp1 & 1) crcTmp1 = crcTmp1 >> 1 ^ 0xEDB88320L;
      else crcTmp1 >>= 1;
    }
    crcTable[i] = crcTmp1;
  }

  // 计算CRC32值
  DWORD crcTmp2 = 0xFFFFFFFF;
  while (Size--) {
    crcTmp2 = ((crcTmp2 >> 8) & 0x00FFFFFF) ^ crcTable[(crcTmp2 ^ (*ptr)) & 0xFF];
    ptr++;
  }
  crcTmp2 = crcTmp2 ^ 0xFFFFFFFF;
  char crc32[9];
  ignore = sprintf_s(crc32, "%X", crcTmp2);
  return wstring{ crc32, crc32 + strlen(crc32) };
}