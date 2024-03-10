#pragma once
#include <vector>
#include <format>
#include <iostream>
#include <fstream>
#include <tchar.h>
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
#include <wininet.h>
#include <Shlobj.h>
#include <ShlObj_core.h>
#include <thread>
#include <cerrno>

#include "utils.h"
#include "tinyxml2.h"

#define CURL_STATICLIB  // libcurl需要
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "Ws2_32.lib")  // libcurl需要
#pragma comment(lib, "Wldap32.lib")  // libcurl需要
#pragma comment(lib, "Crypt32.lib")  // libcurl需要
#pragma comment(lib, "Normaliz.lib")  // libcurl需要
#include "curl/curl.h"
#ifdef _WIN64
#ifdef _DEBUG
#pragma comment(lib, "curl/libcurl-vc-x64-debug-static-ipv6-sspi-schannel.lib")
#else
#pragma comment(lib, "curl/libcurl-vc-x64-release-static-ipv6-sspi-schannel.lib")
#endif
#else
#ifdef _DEBUG
#pragma comment(lib, "curl/libcurl-vc-x86-debug-static-ipv6-sspi-schannel.lib")
#else
#pragma comment(lib, "curl/libcurl-vc-x86-release-static-ipv6-sspi-schannel.lib")
#endif
#endif

// 开启可视化效果
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#ifdef _DEBUG
#define IS_DEBUG 1
#else
#define IS_DEBUG 0
#endif

struct XML_FILE {
  wstring path;
  wstring exec;
  wstring unzip;
  bool overwrite; // 是否覆盖 0=不覆盖 1=覆盖
  unsigned long long size;  // 文件大小
  wstring CRC32;
};

INT_PTR CALLBACK proc_updater(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Cls_OnSysCommand(HWND hwnd, UINT cmd, int x, int y);
void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

/**
 * \brief 为防止对话框界面失去响应，通过新建线程执行此函数执行下载更新过程
 * \param hListview Listview句柄
 * \param xml_files xml解析后的数据vector
 * \return 下载完成返回true，失败返回false
 * \note 函数流程是：解析出来的xml文件内容，遍历读取每个文件的CRC32，判断是否需要更新，如果需要更新则下载云端文件到临时目录，下载完成后替换。最后判断所有文件的更新结果。
 */
void start_update(HWND hListview, const vector<XML_FILE>& xml_files);

/**
 * \brief libcurl接收到数据时的回调函数
 * \param buffer 接收到的数据所在缓冲区
 * \param size 数据长度
 * \param nmemb 数据片数量
 * \param user_p 用户自定义指针
 * \return 获取的数据长度
 */
size_t proc_libcurl_write(const void* buffer, size_t size, size_t nmemb, void* user_p);

/**
 * \brief libcurl接收到数据时的进度回调函数
 * \param clientp 用户自定义参数，通过设置CURLOPT_XFERINFODATA属性来传递
 * \param dltotal 需要下载的总字节数
 * \param dlnow 已经下载的字节数
 * \param ultotal 将要上传的字节数
 * \param ulnow 已经上传的字节数
 * \note 如果仅下载，ultotal和ulnow是0；如果仅上传，dltotal和dlnow是0。
 * \return 0=正常；非0值将会中断传输，错误代码是 CURLE_ABORTED_BY_CALLBACK
 */
size_t proc_libcurl_progress(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);