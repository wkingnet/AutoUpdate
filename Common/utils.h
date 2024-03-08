#pragma once
#include <iostream>
#include <string>
#include <tchar.h>
#define NOMINMAX
#include <Windows.h>

using namespace std;  // NOLINT(clang-diagnostic-header-hygiene)

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

// Convert an UTF8 string to a wide Unicode String
inline std::wstring utf82unicode(const std::string& str) {
  const int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);
  std::wstring wstrTo(size_needed, 0);
  MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
  return wstrTo;
}

/**
 * \brief Convert an wide Unicode string to ANSI string
 * \param wstr 输入的wstring字符串
 * \return string
 * \note 转换后末尾有\0终止符，比较字符串时注意。再加.c_str()可消除\0终止符
 */
inline std::string unicode2ansi(const std::wstring& wstr) {
  const int size_needed = WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, nullptr, 0, nullptr, nullptr);
  std::string strTo(size_needed, 0);
  WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
  return strTo;
}

// Convert an ANSI string to a wide Unicode String
inline std::wstring ansi2unicode(const std::string& str) {
  const int size_needed = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), nullptr, 0);
  std::wstring wstrTo(size_needed, 0);
  MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
  return wstrTo;
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

// 分割字符串，返回分割后的子串
inline vector<string> split_string(const string& str, const string& split) {
  if (str.empty())
    return {};
  vector<string> res{};

  //在字符串末尾也加入分隔符，方便截取最后一段
  string strs = str + split;
  size_t pos = strs.find(split);
  const size_t step = split.size();

  // 若找不到内容则字符串搜索函数返回 npos
  while (pos != strs.npos) {
    string temp = strs.substr(0, pos);
    res.emplace_back(temp);
    //去掉已分割的字符串,在剩下的字符串中进行分割
    strs = strs.substr(pos + step, strs.size());
    pos = strs.find(split);
  }
  return res;
}

// 分割字符串，返回分割后的子串
inline vector<wstring> split_string(const wstring& str, const wstring& split) {
  if (str.empty())
    return {};
  vector<wstring> res{};

  //在字符串末尾也加入分隔符，方便截取最后一段
  wstring strs = str + split;
  size_t pos = strs.find(split);
  const size_t step = split.size();

  // 若找不到内容则字符串搜索函数返回 npos
  while (pos != strs.npos) {
    wstring temp = strs.substr(0, pos);
    res.emplace_back(temp);
    //去掉已分割的字符串,在剩下的字符串中进行分割
    strs = strs.substr(pos + step, strs.size());
    pos = strs.find(split);
  }
  return res;
}

// 分割字符串，返回分割后的子串
inline vector<string> split_string(const string& str, const char& split) {
  if (str.empty())
    return {};
  vector<string> res{};

  //在字符串末尾也加入分隔符，方便截取最后一段
  string strs = str + split;
  size_t pos = strs.find(split);

  // 若找不到内容则字符串搜索函数返回 npos
  while (pos != strs.npos) {
    string temp = strs.substr(0, pos);
    res.emplace_back(temp);
    //去掉已分割的字符串,在剩下的字符串中进行分割
    strs = strs.substr(pos + 1, strs.size());
    pos = strs.find(split);
  }
  return res;
}

// 分割字符串，返回分割后的子串
inline vector<wstring> split_string(const wstring& str, const wchar_t& split) {
  if (str.empty())
    return {};
  vector<wstring> res{};

  //在字符串末尾也加入分隔符，方便截取最后一段
  wstring strs = str + split;
  size_t pos = strs.find(split);

  // 若找不到内容则字符串搜索函数返回 npos
  while (pos != strs.npos) {
    wstring temp = strs.substr(0, pos);
    res.emplace_back(temp);
    //去掉已分割的字符串,在剩下的字符串中进行分割
    strs = strs.substr(pos + 1, strs.size());
    pos = strs.find(split);
  }
  return res;
}

/**
 * \brief 字符串替换函数
 * \param resource_str 源字符串
 * \param sub_str 被替换子串
 * \param new_str 替换子串
 * \return 替换后的字符串
 */
template <typename T = string>
T subreplace(const T& resource_str, const T& sub_str, const T& new_str) {
  T dst_str = resource_str;
  size_t pos;
  while ((pos = dst_str.find(sub_str)) != T::npos)   //替换所有指定子串
  {
    dst_str.replace(pos, sub_str.length(), new_str);
  }
  return dst_str;
}