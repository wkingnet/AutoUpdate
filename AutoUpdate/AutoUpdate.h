/**
*****************************************************************************
*  Copyright (C), 2010-2024, WKING. All rights reserved.
*
*  @author  wking
*  @website http://wkings.blog
*  @date    输入日期
*  @version 版本号
*
*****************************************************************************
*/
#pragma once
#include <iostream>
#include <Windows.h>
#include <wininet.h>
#include <format>
#include <tchar.h>
#include <vector>
#include <thread>
#include "tinyxml2.h"

#pragma comment(lib, "wininet.lib")

namespace AutoUpdate {
  using namespace std;
  using namespace tinyxml2;

  struct XML_FILE {
    wstring path;
    wstring exec;
    wstring unzip;
    bool overwrite; // 是否覆盖 0=不覆盖 1=覆盖
    unsigned long long size;  // 文件大小
    wstring CRC32;
  };

  inline std::wstring utf82unicode(const std::string& str) {
    const int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), wstrTo.data(), size_needed);
    return wstrTo;
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

  /**
   * \brief 下载并解析xml文件，判断是否有更新
   * \param xml_url 在线更新xml数据文件的URL
   * \return true=有更新 false=没有更新or无法下载or无法解析xml
   */
  inline bool is_update(const wchar_t* xml_url) {
    wstring exe_path;
    const auto exeFullPath = new wchar_t[MAXWORD];
    GetModuleFileName(nullptr, exeFullPath, MAXWORD);
    exe_path = exeFullPath;
    exe_path = exe_path.substr(0, exe_path.find_last_of('\\')) + L'\\';
    delete[] exeFullPath;
    tinyxml2::XMLDocument xmldoc;
    if (const HINTERNET hSession = InternetOpen(L"AutoUpdate_Updater", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0)) {
      if (const HINTERNET hRequest = InternetOpenUrl(hSession, xml_url, nullptr, 0, 0, 0)) {
        const auto szBuffer = new char[MAXWORD] {};
        DWORD dwBytesRead;
        InternetReadFile(hRequest, szBuffer, MAXWORD, &dwBytesRead);
        if (dwBytesRead)
          xmldoc.Parse(szBuffer);
        delete[] szBuffer;
        InternetCloseHandle(hRequest);
      }
      InternetCloseHandle(hSession);
    }
    if (xmldoc.ErrorID())
      return false;
    const XMLElement* xml_root = xmldoc.RootElement();
    if (!xml_root) {
      return false;
    }
    const XMLElement* xml_filelist = xml_root->FirstChildElement("filelist");
    if (!xml_filelist) {
      return false;
    }
    vector<XML_FILE> xml_files;
    for (const XMLElement* cur = xml_filelist->FirstChildElement("file"); cur; cur = cur->NextSiblingElement()) {
      XML_FILE xml_file{};
      // ReSharper disable once CppDeclarationHidesLocal
      const XMLElement* element;
      element = cur->FirstChildElement("path");
      if (element) {
        if (element->GetText())
          xml_file.path = utf82unicode(cur->FirstChildElement("path")->GetText());
      }
      element = cur->FirstChildElement("exec");
      if (element) {
        if (element->GetText())
          xml_file.exec = utf82unicode(cur->FirstChildElement("exec")->GetText());
      }
      element = cur->FirstChildElement("unzip");
      if (element) {
        if (element->GetText())
          xml_file.unzip = utf82unicode(cur->FirstChildElement("unzip")->GetText());
      }
      element = cur->FirstChildElement("overwrite");
      if (element) {
        if (element->GetText())
          cur->FirstChildElement("overwrite")->QueryBoolText(&xml_file.overwrite);
      }
      element = cur->FirstChildElement("size");
      if (element) {
        if (element->GetText())
          xml_file.size = cur->FirstChildElement("size")->Unsigned64Text();
      }
      element = cur->FirstChildElement("CRC32");
      if (element) {
        if (element->GetText())
          xml_file.CRC32 = utf82unicode(cur->FirstChildElement("CRC32")->GetText());
      }
      xml_files.emplace_back(xml_file);
    }
    for (const auto& [path, exec, unzip, overwrite, size, CRC32] : xml_files) {
      wstring filepath(exe_path + path);
      if (_waccess_s(filepath.data(), 0) == 0) {
        if (!overwrite)
          continue;
        const HANDLE hFile = CreateFile(filepath.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
          continue;
        const DWORD dwSize = GetFileSize(hFile, nullptr);
        const auto pFile = new BYTE[dwSize];
        wstring crc32;
        if (ReadFile(hFile, pFile, dwSize, nullptr, nullptr)) {
          crc32 = calc_crc32(pFile, dwSize);
          delete[] pFile;
        }
        else {
          delete[] pFile;
          continue;
        }
        if (!CloseHandle(hFile))
          continue;
        if (crc32 == CRC32)
          continue;
        return true;
      }
      return true;
    }
    return false;
  }

  /**
   * \brief 线程执行更新函数
   * \param hwnd 父窗口句柄
   * \param update_exe 更新程序的程序名。用于启动更新程序。
   * \param xml_url 在线更新的XML数据URL
   * \param notice 无需更新时是否显示结果消息框。true=显示, false=不显示
   */
  inline void thread_update(const HWND hwnd, const wchar_t* update_exe, const wchar_t* xml_url, const bool notice) {
    if (is_update(xml_url)) {
      if (MessageBox(hwnd, L"程序有新版本，是否更新？", L"检查更新", MB_ICONQUESTION | MB_OKCANCEL) == IDOK) {
        STARTUPINFO StartInfo{};
        PROCESS_INFORMATION ProInfo{};
        wstring url_tmp(L" ");
        url_tmp += xml_url;
        if (!CreateProcess(update_exe, url_tmp.data(), nullptr, nullptr, false, 0, nullptr, nullptr, &StartInfo, &ProInfo)) {
          MessageBox(hwnd, (L"启动自动更新程序出错，错误代码：" + std::to_wstring(GetLastError())).data(), L"检查更新", MB_ICONERROR | MB_OK);
        }
        ExitProcess(GetLastError());
      }
    }
    else if (notice) {
      MessageBox(hwnd, L"目前已经是最新版本", L"检查更新", MB_ICONINFORMATION | MB_OK);
    }
  }

  /**
 * \brief 自动更新模块的调用函数
 * \param hwnd 调用更新模块的窗口句柄，可为nullptr，用于显示MessageBox
 * \param update_exe 自动更新程序的文件名，用于启动更新程序。
 * \param xml_url 自动更新config程序生成的xml文件的URL
 * \param notice 无需更新时是否显示结果消息框。true=显示, false=不显示
 * \return 自动更新线程的线程ID
 */
  inline thread::id AutoUpdate(const HWND hwnd, const wchar_t* update_exe, const wchar_t* xml_url, const bool notice) {
    wstring wstr(update_exe);
    wstr += L".update";
    if (_waccess_s(wstr.data(), 0) == 0) {
      if (CopyFile(wstr.data(), update_exe, FALSE))
        DeleteFile(wstr.data());
    }
    jthread t(thread_update, hwnd, update_exe, xml_url, notice);
    t.detach();
    return t.get_id();
  }
}