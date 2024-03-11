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

// ReSharper disable CppClangTidyClangDiagnosticExtraSemiStmt
// ReSharper disable CppClangTidyBugproneImplicitWideningOfMultiplicationResult
#include "header.h"
#include "resource.h"

using namespace std;

// 定义更新临时目录的目录名
constexpr auto UPDATE_DIR = _T("update");

HWND g_hDialogUpdater;
wstring exe_path; // 程序运行路径 末尾有"\"
wstring update_path; // 程序操作的临时存放更新文件的目录 末尾有"\"
tinyxml2::XMLDocument xmldoc; // 在线xml对象
wstring update_time; // 在线xml文件的更新时间
wstring update_url; // 在线xml文件的更新URL
vector<XML_FILE> vecXmlfiles; // xml解析后的数据
DWORD g_interval = GetTickCount(); // libcurl显示回调函数里间隔刷新对话框计数

// 用于传递libcurl进度回调函数形参clientp的结构体
struct CURL_XFERINFODATA {
  CURL* curl;
  HWND hListview;
  size_t listview_idx; // Listview当前行索引数
  unsigned long long total_size; // 下载文件总大小
};

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                       _In_opt_ HINSTANCE hPrevInstance,
                       _In_ LPWSTR    lpCmdLine,
                       _In_ int       nShowCmd) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  if constexpr (IS_DEBUG) {
    AllocConsole();
    //system("chcp 65001");

    // wcout 显示中文
    std::wcout.imbue(locale(locale(), "", LC_CTYPE));

    std::ignore = freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    std::ignore = freopen_s((FILE**)stdout, "CONOUT$", "w", stderr);
    std::ignore = freopen_s((FILE**)stdout, "CONIN$", "r", stdin);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
    std::cin.clear();
    //cout << "cout  This works γηυ测试國ふそ▁☒☑■☾•♫¶¤φÇÏÅはにてヌホム㉢ㅆㅔㅃㅋㅝㅘㅎЩОёыфцэыъшмбгжф\n";
    cout << "cout test\n";
    cout << "cout 测试\n";
    wcout << L"wcout test\n";
    wcout << L"wcout 测试\n";
  }

  // 获取程序运行路径
  const auto exeFullPath = new wchar_t[MAXWORD];
  GetModuleFileName(nullptr, exeFullPath, MAXWORD);
  exe_path = exeFullPath;
  exe_path = exe_path.substr(0, exe_path.find_last_of('\\')) + L'\\';
  delete[] exeFullPath;
  wcout << "exe_path=" << exe_path << "\n";
  update_path = exe_path + UPDATE_DIR + _T("\\");

  // 获取命令行参数
  // https://blog.csdn.net/ypist/article/details/8138310
  int argCount; //命令行参数个数
  // 获取命令行参数保存在szArgList szArgList[0]是程序自己的程序名
  const LPWSTR* szArgList = CommandLineToArgvW(GetCommandLine(), &argCount);
  if (szArgList != nullptr) {
    if (argCount == 2)
      update_url = szArgList[1];
    else if (argCount < 2) {
      MessageBox(nullptr, L"请勿直接运行，程序退出", L"自动更新", MB_ICONERROR | MB_OK);
      return 0;
    }
    else if (argCount > 2) {
      MessageBox(nullptr, L"参数数量不符，程序退出", L"自动更新", MB_ICONERROR | MB_OK);
      return 0;
    }
    else {
      return 0;
    }
  }
  else {
    MessageBox(nullptr, L"命令行参数获取失败，程序退出", L"自动更新", MB_ICONERROR | MB_OK);
    return 0;
  }


  // 下载解析xml文件
  if (const HINTERNET hSession = InternetOpen(L"AutoUpdate_Updater", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0)) {
    if (const HINTERNET hRequest = InternetOpenUrl(hSession, update_url.data(), nullptr, 0, 0, 0)) {
      const auto szBuffer = new char[MAXWORD] {};  // 保存在线xml数据 这里有可能XML文件大于数组大小导致读取失败
      DWORD dwBytesRead;
      InternetReadFile(hRequest, szBuffer, MAXWORD, &dwBytesRead);
      // 读取成功则dwBytesRead!=0
      if (dwBytesRead) {
        xmldoc.Parse(szBuffer);
      }
      delete[] szBuffer;
      InternetCloseHandle(hRequest);
    }
    else
      wcerr << format(L"InternetOpenUrl={} 创建失败\n", update_url);
    InternetCloseHandle(hSession);
  }
  else
    wcerr << format(L"InternetOpen(WinInet) 创建失败\n");

  if (xmldoc.ErrorID()) {
    /*
    https://blog.csdn.net/qq_36518164/article/details/108809984
    return doc.ErrorID();//返回类型为 int
    //错误Id枚举类型
    enum XMLError {
      XML_SUCCESS = 0,
      XML_NO_ATTRIBUTE,//没有属性
      XML_WRONG_ATTRIBUTE_TYPE,//错误的属性类型
      XML_ERROR_FILE_NOT_FOUND,//找不到文件
      XML_ERROR_FILE_COULD_NOT_BE_OPENED,//文件无法打开
      XML_ERROR_FILE_READ_ERROR,//文件读取错误
      XML_ERROR_PARSING_ELEMENT,//element语法错误
      XML_ERROR_PARSING_ATTRIBUTE,//attribute语法错误
      XML_ERROR_PARSING_TEXT,//text语法错误
      XML_ERROR_PARSING_CDATA,//cdata语法错误
      XML_ERROR_PARSING_COMMENT,//comment语法错误
      XML_ERROR_PARSING_DECLARATION,//declarattion语法错误
      XML_ERROR_PARSING_UNKNOWN,//未知错误
      XML_ERROR_EMPTY_DOCUMENT,//空文档
      XML_ERROR_MISMATCHED_ELEMENT,//element元素不匹配
      XML_ERROR_PARSING,//解析错误
      XML_CAN_NOT_CONVERT_TEXT,//不能转换文本
      XML_NO_TEXT_NODE,//没有文本节点
      XML_ELEMENT_DEPTH_EXCEEDED,//超过了xml的element元素深度
      XML_ERROR_COUNT//错误计数
    };
    */
    const wstring wstr(L"XML文件解析失败，程序退出。错误ID=" + to_wstring(xmldoc.ErrorID()));
    wcerr << wstr;
    MessageBox(nullptr, wstr.data(), L"自动更新", MB_ICONERROR | MB_OK);
    return 0;
  }

  // 创建对话框，失败则退出
  g_hDialogUpdater = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG_UPDATER), GetDesktopWindow(), proc_updater);
  if (!g_hDialogUpdater)
    return 0;
  else
    ShowWindow(g_hDialogUpdater, SW_SHOW);

  // 初始化libcurl
  if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
    cerr << "init libcurl failed.\n";
    MessageBox(g_hDialogUpdater, L"libcurl初始化失败，程序退出", L"自动更新", MB_ICONERROR | MB_OK);
    return 0;
  }

  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // 释放curl
  curl_global_cleanup();

  return 0;
}


INT_PTR CALLBACK proc_updater(const HWND hDlg, const UINT msg, const WPARAM wParam, const LPARAM lParam) {
  switch (msg) {
    HANDLE_MSG(hDlg, WM_INITDIALOG, Cls_OnInitDialog);
    HANDLE_MSG(hDlg, WM_SYSCOMMAND, Cls_OnSysCommand);
    HANDLE_MSG(hDlg, WM_COMMAND, Cls_OnCommand);
  default:
    return FALSE;
  }
}

BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {
  using namespace tinyxml2;

  // 获取对话框信息并移动窗口到屏幕中间
  WINDOWINFO wininfo{};
  GetWindowInfo(hwnd, &wininfo);
  MoveWindow(hwnd, (GetSystemMetrics(SM_CXSCREEN) - wininfo.rcWindow.right) / 2, (GetSystemMetrics(SM_CYSCREEN) - wininfo.rcWindow.bottom) / 2, wininfo.rcWindow.right, wininfo.rcWindow.bottom, FALSE);

  // ListView
// 获取ListView控件的句柄
  const HWND hList = GetDlgItem(hwnd, IDC_LISTVIEW);

  // 设置扩展风格 选中高亮 复选框 网格线
  ListView_SetExtendedListViewStyle(hList, LVS_EX_GRIDLINES);

  // 设置ListView的列
  LVCOLUMN column = {};
  column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

  // 第0列
  column.fmt = LVCFMT_LEFT; // 左对齐
  column.pszText = (LPWSTR)L"相对路径文件"; // 列标题
  column.cx = 370;//列宽
  column.iSubItem = 0;//子项索引
  ListView_InsertColumn(hList, 0, &column);
  // 第1列
  column.fmt = LVCFMT_CENTER; // 居中对齐 下面都是居中对齐
  column.pszText = (LPWSTR)L"进度"; // 列标题
  column.cx = 50;//列宽
  column.iSubItem = 1;//子项索引
  ListView_InsertColumn(hList, 1, &column);

  // 定义根节点
  const XMLElement* xml_root = xmldoc.RootElement();

  // 获取config节点
  const XMLElement* xml_config = xml_root->FirstChildElement("config");
  const XMLElement* element;
  element = xml_config->FirstChildElement("time");
  if (element) {
    if (element->GetText())
      update_time = utf82unicode(xml_config->FirstChildElement("time")->GetText());
  }
  element = xml_config->FirstChildElement("url");
  if (element) {
    if (element->GetText())
      update_url = utf82unicode(xml_config->FirstChildElement("url")->GetText());
  }

  // 判断根节点下是否有filelist节点
  const XMLElement* xml_filelist = xml_root->FirstChildElement("filelist");
  if (!xml_filelist) {
    cout << "XML没有filelist节点";
    return false;
  }

  // 解析xml对象filelist元素，填充listview
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
    vecXmlfiles.emplace_back(xml_file);

    // 插入行
    int item_count = ListView_GetItemCount(hList);
    LVITEM row = {}; // 创建item结构体
    row.mask = LVIF_TEXT | LVIF_STATE;
    row.pszText = xml_file.path.data();
    row.iItem = item_count;
    row.iSubItem = 0;
    ListView_InsertItem(hList, &row);

    row.pszText = (LPWSTR)L"";  // 下载百分比开始都是空
    row.iItem = item_count;
    row.iSubItem = 1;
    ListView_SetItem(hList, &row);
  }

  // 如果是release模式，发送开始更新消息以便自动更新
  if constexpr (!IS_DEBUG)
    PostMessage(hwnd, WM_COMMAND, IDC_BUTTON_START, 0);

  return true;
}

void Cls_OnSysCommand(HWND hwnd, const UINT cmd, int x, int y) {
  if (cmd == SC_CLOSE) {
    PostQuitMessage(0);//退出
  }
}

void Cls_OnCommand(const HWND hwnd, const int id, HWND hwndCtl, UINT codeNotify) {
  if (id == IDC_BUTTON_START) {
    // 禁用开始更新按钮
    Button_Enable(GetDlgItem(hwnd, IDC_BUTTON_START), FALSE);

    // 如果没有更新目录则新建
    if (_waccess_s(update_path.data(), 0) == 2)
      CreateDirectory(update_path.data(), nullptr);

    thread t(AutoUpdate::start_update, GetDlgItem(hwnd, IDC_LISTVIEW), vecXmlfiles);

    // 分离线程，让线程自己执行，防止主程序卡死
    t.detach();
  }
}


void AutoUpdate::start_update(HWND hListview, const vector<XML_FILE>& xml_files) {
  using namespace AutoUpdate;

  // 创建curl easy interface
  CURL* curl = curl_easy_init();
  if (curl == nullptr) {
    cerr << "get a easy handle failed. func return.\n";
    return;
  }

  // 初始化变量和设置
  unsigned long long total_size{};  // 所有要下载的文件总大小，用于计算剩余时间
  FILE* fp{};
  char errbuf[CURL_ERROR_SIZE]; // 接收curl错误信息的buffer
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);  //设置错误缓冲区
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);  // 不验证证书
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, FALSE);  // 不验证POST
  curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);  // 当HTTP返回值大于等于400的时候，请求失败
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &proc_libcurl_write);  // 设置写入回调函数
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L); // 设置开启进度回调功能
  curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, proc_libcurl_progress);  // 设置进度回调函数

  // 先遍历一遍计算总文件大小
  for (const auto& xml_file : xml_files)
    total_size += xml_file.size;

  // 遍历
  for (size_t i = 0; i < xml_files.size(); i++) {
    wstring filepath(exe_path + xml_files[i].path); // 文件路径
    wstring filepath_update(update_path + xml_files[i].path); // 文件临时更新路径
    string urlpath(unicode2utf8(update_url + xml_files[i].path)); // URL路径
    urlpath = subreplace(urlpath, string("\\"), string("/")); // 转换URL中的\为/
    wcout << filepath;

    // 在循环开始处设置回调函数形参clientp，避免放到循环最后导致total_size计算出错
    CURL_XFERINFODATA clientp{ curl, hListview, i, total_size };
    // 设置进度回调函数形参clientp
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &clientp);
    total_size -= xml_files[i].size;

    // 如果相对文件路径包括目录则新建目录
    vector<wstring> split = split_string(xml_files[i].path, LR"(\)");
    // 大于1说明有分割，遍历且排除最后一个元素(也就是文件名)
    if (split.size() > 1) {
      wstring path(exe_path);
      wstring path1(update_path); // 给临时更新目录也新建
      for (auto it = split.begin(); it != split.end() - 1; ++it) {
        path += *it + LR"(\)";
        if (_waccess_s(path.data(), 0) == 2)
          CreateDirectory(path.data(), nullptr);
        path1 += *it + LR"(\)";
        if (_waccess_s(path1.data(), 0) == 2)
          CreateDirectory(path1.data(), nullptr);
      }
    }

    /* set the error buffer as empty before performing a request */
    errbuf[0] = 0;

    // 先判断文件是否存在，如存在则校检，校验一致跳过不下载。0=存在
    if (_waccess_s(filepath.data(), 0) == 0) {
      cout << " 文件存在";

      // 判断xml中该文件的overwrite属性，如果是1则覆盖更新，0则跳过
      if (!xml_files[i].overwrite) {
        cout << " 不覆盖，跳过\n";
        ListView_SetItemText(hListview, i, 1, (LPWSTR)L"100%");
        continue;
      }

      // 获取文件大小以及CRC32
      // https://www.cnblogs.com/LyShark/p/13656473.html
      const HANDLE hFile = CreateFile(filepath.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
      // 文件打开失败
      if (hFile == INVALID_HANDLE_VALUE) {
        cout << format("CreateFile failed {}\n", GetLastError());
        ListView_SetItemText(hListview, i, 1, (LPWSTR)L"失败");
        continue;
      }

      // 获取文件大小
      const DWORD dwSize = GetFileSize(hFile, nullptr);

      // 创建文件大小的字节指针，将文件读入指针，然后计算CRC32
      const auto pFile = new BYTE[dwSize];
      wstring crc32;

      // 读取文件并判断是否读取成功，成功则计算crc32，不成功继续下个循环
      if (ReadFile(hFile, pFile, dwSize, nullptr, nullptr)) {
        crc32 = calc_crc32(pFile, dwSize);
        delete[] pFile;
      }
      else {
        ListView_SetItemText(hListview, i, 1, (LPWSTR)L"失败");
        cout << format("ReadFile failed {}\n", GetLastError());
        delete[] pFile;
        continue;
      }

      // 关闭文件句柄，不成功继续下个循环
      if (!CloseHandle(hFile)) {
        ListView_SetItemText(hListview, i, 1, (LPWSTR)L"失败");
        cout << format("CloseHandle failed {}\n", errno);
        continue;
      }

      // 如果校检一致，更新Listview，继续下个循环
      if (crc32 == xml_files[i].CRC32) {
        cout << " 无需更新\n";
        ListView_SetItemText(hListview, i, 1, (LPWSTR)L"100%");
        continue;
      }
      else {
        cout << " 需要更新";
      }
    }
    else
      cout << " 文件不存在";

    // 打开临时更新文件，如果成功，则为零；如果失败，则为错误代码。
    if (_wfopen_s(&fp, filepath_update.data(), L"wb")) {
      char errmsg[200];
      if (!strerror_s(errmsg, 200, errno))  // NOLINT(bugprone-not-null-terminated-result)
        cerr << format("fopen_s failed {}\n", errmsg);
      ListView_SetItemText(hListview, i, 1, (LPWSTR)L"失败");
      continue;
    }

    cout << " 开始更新...";
    // 设置写入回调函数的形参user_p，这里传递文件指针
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    // 设置下载URL
    curl_easy_setopt(curl, CURLOPT_URL, urlpath.c_str());

    // 执行数据请求 因为是easy interface所以会阻塞
    // https://curl.se/libcurl/c/CURLOPT_ERRORBUFFER.html
    // 成功返回0；失败返回非零，且错误缓冲区中将显示可读错误消息。if the request did not complete correctly, show the error information. if no detailed error information was written to errbuf show the more generic information from curl_easy_strerror instead.
    const CURLcode res = curl_easy_perform(curl);

    // 关闭文件指针。如果已成功关闭流，则 fclose 返回 0。 _fcloseall 返回已关闭流的总数。 这两个函数都返回 EOF，表示出现错误。
    if (fclose(fp)) {
      char errmsg[200];
      if (!strerror_s(errmsg, 200, errno))  // NOLINT(bugprone-not-null-terminated-result)
        cerr << format("fclose failed {}\n", errmsg);
    }

    if (res != CURLE_OK) {
      cout << " 更新失败\n";
      cerr << "出错URL=";
      wcerr << update_url + xml_files[i].path << L"\n";
      const size_t len = strlen(errbuf);
      ignore = fprintf(stderr, "错误原因: libcurl: (%d) ", res);
      if (len)
        ignore = fprintf(stderr, "%s%s", errbuf, errbuf[len - 1] != '\n' ? "\n" : "");
      else
        ignore = fprintf(stderr, "%s\n", curl_easy_strerror(res));
      ListView_SetItemText(hListview, i, 1, (LPWSTR)L"失败");
    }
    else {
      cout << " 更新成功";

      // 如果是Updater.exe进程自己，则移动更新文件到源目录，并改名为Updater.exe.update，之后通过调用主程序AutoUpdate模块来更新
      wchar_t processFullName[_MAX_PATH]{};
      GetModuleFileName(nullptr, processFullName, _MAX_PATH); //进程完整路径
      if (processFullName == exe_path + xml_files[i].path) {
        cout << " 更新程序自己，移动并改名";
        // 重命名
        wstring filepath_new = filepath_update + L".update";
        if (_waccess_s(filepath_update.data(), 0) == 0) {
          if (CopyFile(filepath_update.data(), filepath_new.data(), FALSE))
            DeleteFile(filepath_update.data());

          // 修改要移动的文件名
          filepath_update = filepath_new;
          filepath += L".update";

          // 删除已存在的Updater.exe.update文件
          DeleteFile(filepath.data());
        }
      }
      // 如果不是更新进程自己
      else {
        // 删除原文件
        if (!DeleteFile(filepath.data())) {
          //cerr << " 删除文件失败\n失败原因:";
          //LastError();
          //ListView_SetItemText(hListview, i, 1, (LPWSTR)L"失败");
        }
      }

      // 移动更新临时文件
      if (!MoveFile(filepath_update.data(), filepath.data())) {
        cerr << " 移动文件失败\n失败原因:";
        LastError();
        ListView_SetItemText(hListview, i, 1, (LPWSTR)L"失败");
      }
      else {
        cout << " 移动成功\n";
        ListView_SetItemText(hListview, i, 1, (LPWSTR)L"100%");
      }
    }
  }

  // 清除easy interface
  curl_easy_cleanup(curl);

  // 设置下载速度显示为0
  SetDlgItemInt(g_hDialogUpdater, IDC_STATIC_DOWNSPEED, 0, false);

  // 递归删除更新目录
  remove_allfiles(update_path);

  // 遍历Listview所有下载进度列，判断文本是否都是100%，全为100%则更新完成，否则更新失败
  auto buffer = new wchar_t[100];
  bool all_done = true;
  for (int i = 0; i < ListView_GetItemCount(hListview); i++) {
    ListView_GetItemText(hListview, i, 1, buffer, 100);
    if (std::wcscmp(buffer, L"100%") != 0)
      all_done = false;
  }
  delete[] buffer;

  if (all_done) {
    cout << "更新完成\n";
    MessageBox(g_hDialogUpdater, L"更新完成，程序退出", L"自动更新", MB_ICONINFORMATION | MB_OK);
    exit(0);  // NOLINT(concurrency-mt-unsafe)
  }
  else {
    cout << "更新失败\n";
    MessageBox(g_hDialogUpdater, L"更新失败，请关闭主程序或者杀毒软件或者重启计算机后重试", L"自动更新", MB_ICONERROR | MB_OK);
    // 启用开始更新按钮
    Button_Enable(GetDlgItem(g_hDialogUpdater, IDC_BUTTON_START), TRUE);
  }
}

size_t AutoUpdate::proc_libcurl_write(const void* buffer, const size_t size, const size_t nmemb, void* user_p) {
  const auto fp = (FILE*)user_p;
  return fwrite(buffer, size, nmemb, fp);
}

size_t AutoUpdate::proc_libcurl_progress(void* clientp, const curl_off_t dltotal, const curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
  // https://blog.csdn.net/ixiaochouyu/article/details/47301005

  const auto arg1 = static_cast<CURL_XFERINFODATA*>(clientp);
  wchar_t remain_time[9] = L"Unknown";
  wstring unit(L"B/s");

  curl_off_t speed;

  //cout << format("dltotal={} dlnow={} 下载速度={} KBytes/sec\n",dltotal, dlnow, speed / 1000);
  curl_easy_getinfo(arg1->curl, CURLINFO_SPEED_DOWNLOAD_T, &speed);

  // 如果没有速度则直接返回
  if (!speed)
    return 0;

  // 如果间隔大于，则更新一次显示
  const DWORD tmp_time = GetTickCount();
  if (tmp_time - g_interval > 100) {
    g_interval = tmp_time;
    // Time remaining
    const int leftTime = static_cast<int>((arg1->total_size - dlnow) / speed);
    int hours = leftTime / 3600;
    int minutes = (leftTime - hours * 3600) / 60;
    int seconds = leftTime - hours * 3600 - minutes * 60;


#ifdef _WIN64
    ignore = swprintf_s(timeFormat, 9, L"%02d:%02d:%02d", hours, minutes, seconds);
#else
    ignore = swprintf_s(remain_time, L"%02d:%02d:%02d", hours, minutes, seconds);
#endif

    if (speed > 1024 * 1024 * 1024) {
      unit = L"GB/s";
      speed /= 1024 * 1024 * 1024;
    }
    else if (speed > 1024 * 1024) {
      unit = L"MB/s";
      speed /= 1024 * 1024;
    }
    else if (speed > 1024) {
      unit = L"kB/s";
      speed /= 1024;
    }

    // 进度显示
    wstring wstr(format(L"{:d}%", dlnow * 100 / dltotal));
    ListView_SetItemText(arg1->hListview, arg1->listview_idx, 1, wstr.data());

    SetDlgItemInt(g_hDialogUpdater, IDC_STATIC_DOWNSPEED, (UINT)speed, false);  // 设置下载速度显示
    SetDlgItemText(g_hDialogUpdater, IDC_STATIC_TIMELEFT, remain_time);  // 设置剩余时间显示
    SetDlgItemText(g_hDialogUpdater, IDC_STATIC_SPEED_UNIT, unit.data());  // 设置下载单位显示
  }

  return 0;
}

