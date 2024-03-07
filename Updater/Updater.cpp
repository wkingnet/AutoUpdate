// ReSharper disable CppClangTidyClangDiagnosticExtraSemiStmt

#include "header.h"
#include "resource.h"

using namespace std;

HWND g_hDialogUpdater;
wstring exe_path; // 程序运行路径 末尾有"\"
tinyxml2::XMLDocument xmldoc; // 在线xml对象
wstring update_time; // 在线xml文件的更新时间
wstring update_url; // 在线xml文件的更新URL
vector<XML_FILE> vecXmlfiles; // xml解析后的数据

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
  wcout << "exe_path=" << exe_path << endl;

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
      wcout << format(L"InternetOpenUrl={} 创建失败\n", update_url);
    InternetCloseHandle(hSession);
  }
  else
    wcout << format(L"InternetOpen(WinInet) 创建失败\n");

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
    wcout << wstr;
    MessageBox(nullptr, wstr.data(), L"自动更新", MB_ICONERROR | MB_OK);
    return 0;
  }

  // 创建对话框，失败则退出
  if (g_hDialogUpdater = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG_UPDATER), GetDesktopWindow(), proc_updater); !g_hDialogUpdater)
    return 0;
  ShowWindow(g_hDialogUpdater, SW_SHOW);

  // 初始化libcurl
  if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
    cerr << "init libcurl failed.\n";
    MessageBox(nullptr, L"libcurl初始化失败，程序退出", L"自动更新", MB_ICONERROR | MB_OK);
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
  if (xml_config->FirstChildElement("time")->GetText())
    update_time = utf82unicode(xml_config->FirstChildElement("time")->GetText());
  if (xml_config->FirstChildElement("url")->GetText())
    update_url = utf82unicode(xml_config->FirstChildElement("url")->GetText());

  // 判断根节点下是否有filelist节点
  const XMLElement* xml_filelist = xml_root->FirstChildElement("filelist");
  if (!xml_filelist) {
    cout << "XML没有filelist节点";
    return false;
  }

  // 解析xml对象filelist元素，填充listview
  for (const XMLElement* cur = xml_filelist->FirstChildElement("file"); cur; cur = cur->NextSiblingElement()) {
    XML_FILE xml_file{};
    if (cur->FirstChildElement("path")->GetText())
      xml_file.path = utf82unicode(cur->FirstChildElement("path")->GetText());
    if (cur->FirstChildElement("exec")->GetText())
      xml_file.exec = utf82unicode(cur->FirstChildElement("exec")->GetText());
    if (cur->FirstChildElement("unzip")->GetText())
      xml_file.unzip = utf82unicode(cur->FirstChildElement("unzip")->GetText());
    if (cur->FirstChildElement("size")->GetText())
      xml_file.size = utf82unicode(cur->FirstChildElement("size")->GetText());
    if (cur->FirstChildElement("CRC32")->GetText())
      xml_file.CRC32 = utf82unicode(cur->FirstChildElement("CRC32")->GetText());
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
  return true;
}

void Cls_OnSysCommand(HWND hwnd, UINT cmd, int x, int y) {
  if (cmd == SC_CLOSE) {
    PostQuitMessage(0);//退出
  }
}

void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
  if (id == IDC_BUTTON_START) {
    thread t(start_update, GetDlgItem(hwnd, IDC_LISTVIEW), vecXmlfiles);

    // 分离线程，让线程自己执行，防止主程序卡死
    t.detach();
  }
}


void start_update(HWND hListview, const vector<XML_FILE>& xml_files) {

  // 创建curl easy interface
  CURL* curl = curl_easy_init();
  if (curl == nullptr) {
    cerr << "get a easy handle failed. func return.\n";
    return;
  }

  // 初始化变量和设置
  FILE* fp{};
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);  // 不验证证书
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, FALSE);  // 不验证POST
  curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);  // 当HTTP返回值大于等于400的时候，请求失败
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &proc_libcurl_write);  // 设置写入回调函数
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);  // 设置写入回调函数的形参user_p，这里传递文件指针
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L); // 设置开启进度回调功能
  curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, proc_libcurl_progress);  // 设置进度回调函数
  curl_easy_setopt(curl, CURLOPT_XFERINFODATA, nullptr);  // 设置进度回调函数形参clientp

  // 遍历
  for (size_t i = 0; i < xml_files.size(); i++) {
    // 先校检文件，如果本地文件与在线文件数据一致，则跳过下载
    wstring filepath(exe_path + xml_files[i].path); // 文件路径

    // 先判断文件是否存在，如存在则校检、下载
    if (_waccess_s(filepath.data(), 0) == 0) {
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
        cout << format("ReadFile failed {}\n", GetLastError());
        delete[] pFile;
        continue;
      }

      // 关闭文件句柄，不成功继续下个循环
      if (!CloseHandle(hFile)) {
        cout << format("CloseHandle failed {}\n", GetLastError());
        continue;
      }

      // 如果校检一致，更新Listview
      if (crc32 == xml_files[i].CRC32) {
        ListView_SetItemText(hListview, i, 1, (LPWSTR)L"100%");
      }
      // 不一致，下载更新
      else {
        wcout << format(L"本地CRC32={} 在线CRC32={}\n", crc32, xml_files[i].CRC32);
      }
    }
    else {
      wcout << filepath << endl;
    }
  }
  curl_easy_perform(curl);  // 执行数据请求 因为是easy interface所以会阻塞


  //fclose(fp);
  curl_easy_cleanup(curl);  // 清除easy interface
}

size_t proc_libcurl_write(const void* buffer, const size_t size, const size_t nmemb, void* user_p) {
  //const auto fp = (FILE*)user_p;
  //return fwrite(buffer, size, nmemb, fp);
  return size * nmemb;
}

size_t proc_libcurl_progress(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
  return 0;
}

