// ReSharper disable CppClangTidyClangDiagnosticExtraSemiStmt
// ReSharper disable CppClangTidyPerformanceNoIntToPtr
#include "header.h"
#include "utils.h"
#include "resource.h"

using namespace std;

HINSTANCE g_hInst;
wstring exe_path; // 此程序运行所在目录
wstring update_path{};  // 更新目录
wstring update_url{};  // 更新URL
ITEMIDLIST* update_idl = nullptr;  // 保存更新目录文件夹的ITEMIDLIST

int WINAPI WinMain(_In_ const HINSTANCE hInstance,
                   _In_opt_  HINSTANCE hPrevInstance,
                   _In_  LPSTR lpCmdLine,
                   _In_  int nCmdShow) {
  g_hInst = hInstance;

  if constexpr (IS_DEBUG) {
    AllocConsole();
    //system("chcp 65001");

    // wcout 显示中文
    wcout.imbue(locale(locale(), "", LC_CTYPE));

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

  const HWND hdlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG_CONFIG), GetDesktopWindow(), ProcConfig);

  if (!hdlg)
    return 0;

  ShowWindow(hdlg, SW_SHOW);

  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}

INT_PTR CALLBACK ProcConfig(const HWND hDlg, const UINT msg, const WPARAM wParam, const LPARAM lParam) {
  switch (msg) {
    HANDLE_MSG(hDlg, WM_INITDIALOG, Cls_OnInitDialog);
    HANDLE_MSG(hDlg, WM_SYSCOMMAND, Cls_OnSysCommand);
    HANDLE_MSG(hDlg, WM_COMMAND, Cls_OnCommand);
  case WM_NOTIFY:
    Cls_OnNotify(hDlg, msg, wParam, lParam);
    break;
  default:;
  }
  return FALSE;
}

BOOL Cls_OnInitDialog(const HWND hwnd, HWND hwndFocus, LPARAM lParam) {
  /*
  // 调试用，指定更新目录，以及获取更新目录对应的IDL
  // https://winterdom.com/dev/ui/ishfolder/
  update_path = L"F:\\down\\gvo\\";
  LPSHELLFOLDER pShellFolder = nullptr;
  ignore = SHGetDesktopFolder(&pShellFolder);
  ignore = pShellFolder->ParseDisplayName(nullptr, nullptr, update_path.data(), nullptr, &update_idl, nullptr);
  SetWindowText(GetDlgItem(hwnd, IDC_EDIT_DIR), update_path.data());
  */

  // 获取对话框信息并移动窗口到屏幕中间
  WINDOWINFO wininfo{};
  GetWindowInfo(hwnd, &wininfo);
  MoveWindow(hwnd, (GetSystemMetrics(SM_CXSCREEN) - wininfo.rcWindow.right) / 2, (GetSystemMetrics(SM_CYSCREEN) - wininfo.rcWindow.bottom) / 2, wininfo.rcWindow.right, wininfo.rcWindow.bottom, FALSE);

  // ListView
  // 获取ListView控件的句柄
  const HWND hListview = GetDlgItem(hwnd, IDC_LIST_FILES);

  // 设置扩展风格 选中高亮 复选框 网格线
  ListView_SetExtendedListViewStyle(hListview, LVS_EX_AUTOCHECKSELECT | LVS_EX_AUTOSIZECOLUMNS | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES);

  // 设置ListView的列
  LVCOLUMN column = {};
  column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

  // 第1列
  column.pszText = (LPWSTR)L""; // 列标题
  column.cx = 23;//列宽
  column.iSubItem = 0;//子项索引，第一列无子项
  ListView_InsertColumn(hListview, 0, &column);
  // 第2列
  column.fmt = LVCFMT_LEFT; // 左对齐
  column.pszText = (LPWSTR)L"相对路径文件"; // 列标题
  column.cx = 400;//列宽
  column.iSubItem = 1;//子项索引
  ListView_InsertColumn(hListview, 1, &column);
  // 第3列
  column.fmt = LVCFMT_CENTER; // 居中对齐 下面都是居中对齐
  column.pszText = (LPWSTR)L"执行"; // 列标题
  column.cx = 40;//列宽
  column.iSubItem = 2;//子项索引
  ListView_InsertColumn(hListview, 2, &column);
  // 第4列
  column.pszText = (LPWSTR)L"解压"; // 列标题
  column.cx = 40;//列宽
  column.iSubItem = 3;//子项索引
  ListView_InsertColumn(hListview, 3, &column);

  column.pszText = (LPWSTR)L"覆盖"; // 列标题
  column.cx = 40;//列宽
  column.iSubItem = 4;//子项索引
  ListView_InsertColumn(hListview, 4, &column);

  column.pszText = (LPWSTR)L"大小"; // 列标题
  column.cx = 120;//列宽
  column.iSubItem = 5;//子项索引
  ListView_InsertColumn(hListview, 5, &column);

  column.pszText = (LPWSTR)L"CRC32"; // 列标题
  column.cx = 80;//列宽
  column.iSubItem = 6;//子项索引
  ListView_InsertColumn(hListview, 6, &column);

  // 如果存在config.cfg则自动加载
  const string config_path = unicode2utf8(exe_path) + "config.cfg";
  if (_access_s(config_path.c_str(), 0) == 0)
    PostMessage(hwnd, WM_COMMAND, IDC_BUTTON_LOAD, 0);

  return TRUE;
}

void Cls_OnSysCommand(HWND hwnd, const UINT cmd, int x, int y) {
  if (cmd == SC_CLOSE) {
    PostQuitMessage(0);//退出
  }
}

void Cls_OnCommand(const HWND hwnd, const int id, HWND hwndCtl, UINT codeNotify) {
  // 获取listview控件句柄
  HWND hList = GetDlgItem(hwnd, IDC_LIST_FILES);

  if (id == IDC_BUTTON_SELECT_PATH) {
    // https://stackoverflow.com/questions/1953339/how-to-get-full-path-from-shbrowseforfolder-function
    auto szFile = new wchar_t[MAXWORD]; // buffer for file name
    BROWSEINFO lpbi{};
    lpbi.hwndOwner = hwnd;
    lpbi.pidlRoot = nullptr;
    lpbi.pszDisplayName = szFile; // Address of a buffer to receive the display name of the folder selected by the user
    lpbi.lpszTitle = L"选择更新文件所在的目录"; // Title of the dialog
    lpbi.ulFlags = 0;
    lpbi.lpfn = nullptr;
    lpbi.lParam = 0;
    lpbi.iImage = -1;

    // 使用SHBrowseForFolder函数获取文件夹的PIDL，如果有PIDL，再使用SHGetPathFromIDList函数获取文件夹的路径，存放在szFile
    if (const auto idl = SHBrowseForFolder(&lpbi)) {
      SHGetPathFromIDList(idl, szFile);
      update_idl = idl;
      update_path = szFile;
      update_path += L"\\";
      SetWindowText(GetDlgItem(hwnd, IDC_EDIT_DIR), update_path.data());
    }

    delete[] szFile;
  }

  else if (id == IDC_BUTTON_ADDFILE) {
    auto szFile = new wchar_t[MAXWORD]; // buffer for file name
    OPENFILENAME ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrTitle = (LPWSTR)L"要添加到更新列表的文件";
    ofn.lpstrFile = szFile;
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = MAXWORD;
    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = update_path.data();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
      // 选择了文件
      wstring filename(szFile);
      // 如果文件路径中包括update_path，则把文件路径中的update_path部分替换为空
      if (filename.find(update_path) != std::string::npos) {
        filename.replace(filename.find(update_path), update_path.size(), L"");

        // https://blog.csdn.net/cpp_learner/article/details/123018360
        // 这是一个存储文件(夹)信息的结构体，其中有文件大小和创建时间、访问时间、修改时间等

        // 获取文件大小以及CRC32
        // https://www.cnblogs.com/LyShark/p/13656473.html
        const HANDLE hFile = CreateFile((update_path + filename).data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

        // 获取文件大小
        const DWORD dwSize = GetFileSize(hFile, nullptr);

        // 创建文件大小的字节指针，将文件读入指针，然后计算CRC32
        const auto pFile = new BYTE[dwSize];
        ignore = ReadFile(hFile, pFile, dwSize, nullptr, nullptr);
        wstring crc32 = calc_crc32(pFile, dwSize);

        int item_count = ListView_GetItemCount(hList);
        LVITEM row = {}; // 创建item结构体
        row.mask = LVIF_TEXT | LVIF_STATE;
        row.pszText = (LPWSTR)L"";
        row.iItem = item_count;
        row.iSubItem = 0;
        ListView_InsertItem(hList, &row);
        row.pszText = filename.data();
        row.iItem = item_count;
        row.iSubItem = 1;
        ListView_SetItem(hList, &row);
        row.pszText = nullptr;
        row.iItem = item_count;
        row.iSubItem = 2;
        ListView_SetItem(hList, &row);
        row.pszText = nullptr;
        row.iItem = item_count;
        row.iSubItem = 3;
        ListView_SetItem(hList, &row);
        row.pszText = (LPWSTR)L"1";
        row.iItem = item_count;
        row.iSubItem = 4;
        ListView_SetItem(hList, &row);
        wstring tmp = std::to_wstring(dwSize);
        row.pszText = tmp.data();
        row.iItem = item_count;
        row.iSubItem = 5;
        ListView_SetItem(hList, &row);
        row.pszText = crc32.data();
        row.iItem = item_count;
        row.iSubItem = 6;
        ListView_SetItem(hList, &row);
      }
      else {
        MessageBox(hwnd, _T("请添加更新目录下的文件"), nullptr, MB_ICONERROR | MB_OK);
      }
    }
    else
      cout << format("选择文件错误代码：{:x}\n", CommDlgExtendedError());

    delete[] szFile;
  }

  else if (id == IDC_BUTTON_ADDDIR) {
    auto szFile = new wchar_t[MAXWORD]; // buffer for file name
    BROWSEINFO lpbi{};
    lpbi.hwndOwner = hwnd;
    lpbi.pidlRoot = update_idl;
    lpbi.pszDisplayName = szFile; // Address of a buffer to receive the display name of the folder selected by the user
    lpbi.lpszTitle = L"选择要添加的目录"; // Title of the dialog
    lpbi.ulFlags = 0;
    lpbi.lpfn = nullptr;
    lpbi.lParam = 0;
    lpbi.iImage = -1;

    // 使用SHBrowseForFolder函数选择文件夹，返回IDL，使用SHGetPathFromIDList函数用IDL获取文件夹路径
    if (const LPITEMIDLIST lpItem = SHBrowseForFolder(&lpbi)) {
      SHGetPathFromIDList(lpItem, szFile);

      // 生成列表项前禁用重绘，以便加快窗口生成速度
      SendMessage(hList, WM_SETREDRAW, FALSE, 0);

      // 使用recursive_directory_iterator函数遍历(子)文件和文件夹
      wstring dir(szFile);
      for (const auto& dir_entry : filesystem::recursive_directory_iterator(szFile)) {
        // 如果是文件
        if (dir_entry.is_regular_file()) {
          const HANDLE hFile = CreateFile(dir_entry.path().wstring().data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

          // 获取文件大小
          const DWORD dwSize = GetFileSize(hFile, nullptr);

          // 创建文件大小的字节指针，将文件读入指针，然后计算CRC32
          const auto pFile = new BYTE[dwSize];
          ignore = ReadFile(hFile, pFile, dwSize, nullptr, nullptr);
          wstring crc32 = calc_crc32(pFile, dwSize);

          // 如果文件路径中包括update_path，则把文件路径中的update_path部分替换为空
          wstring filename(dir_entry.path().wstring());
          if (filename.find(update_path) != std::string::npos) {
            filename.replace(filename.find(update_path), update_path.size(), L"");
          }

          int item_count = ListView_GetItemCount(hList);
          LVITEM row = {}; // 创建item结构体
          row.mask = LVIF_TEXT | LVIF_STATE;
          row.pszText = (LPWSTR)L"";
          row.iItem = item_count;
          row.iSubItem = 0;
          ListView_InsertItem(hList, &row);
          row.pszText = filename.data();
          row.iItem = item_count;
          row.iSubItem = 1;
          ListView_SetItem(hList, &row);
          row.pszText = (LPWSTR)L"";
          row.iItem = item_count;
          row.iSubItem = 2;
          ListView_SetItem(hList, &row);
          row.pszText = (LPWSTR)L"";
          row.iItem = item_count;
          row.iSubItem = 3;
          ListView_SetItem(hList, &row);
          row.pszText = (LPWSTR)L"1";
          row.iItem = item_count;
          row.iSubItem = 4;
          ListView_SetItem(hList, &row);
          wstring tmp = std::to_wstring(dwSize);
          row.pszText = tmp.data();
          row.iItem = item_count;
          row.iSubItem = 5;
          ListView_SetItem(hList, &row);
          row.pszText = crc32.data();
          row.iItem = item_count;
          row.iSubItem = 6;
          ListView_SetItem(hList, &row);

          delete[] pFile;
        }
      }
      // 生成列表项后启用重绘，以便加快窗口生成速度
      SendMessage(hList, WM_SETREDRAW, TRUE, 0);
    }

    delete[] szFile;
  }

  if (id == IDC_BUTTON_DELETE) {
    vector<int> indexes;
    int iPos = ListView_GetNextItem(hList, -1, LVNI_SELECTED); // Get the first selected item

    // 先把所有选择的项保存到vector，不能直接删除会造成索引错乱
    while (iPos != -1) {
      indexes.emplace_back(iPos);
      iPos = ListView_GetNextItem(hList, iPos, LVNI_SELECTED);
    }

    // 降序排序，以便所有从后往前删除
    ranges::sort(indexes, greater());

    // 禁用重绘，以便加快窗口生成速度
    SendMessage(hList, WM_SETREDRAW, FALSE, 0);
    for (const auto& idx : indexes)
      ListView_DeleteItem(hList, idx);
    // 启用重绘，以便加快窗口生成速度
    SendMessage(hList, WM_SETREDRAW, TRUE, 0);
  }

  if (id == IDC_BUTTON_SAVE) {
    // 如果已存在config.cfg则删除
    if (_waccess_s(L"config.cfg", 0) == 0) {
      if (MessageBox(hwnd, L"已存在配置文件，是否覆盖？", L"提示", MB_ICONINFORMATION | MB_YESNO) == IDYES)
        DeleteFile(L"config.cfg");
      else
        return;
    }

    // 创建inifile对象
    string filename(unicode2utf8(exe_path) + "config.cfg");
    xini_file_t xini_file(filename);

    // 这种语句为写入变量到ini文件
    xini_file["config"]["dir"] = unicode2ansi(update_path).c_str();

    // 获取URL控件长度和内容
    int length = Edit_GetTextLength(GetDlgItem(hwnd, IDC_EDIT_URL)) + 1;
    update_url.resize(length);
    Edit_GetText(GetDlgItem(hwnd, IDC_EDIT_URL), update_url.data(), length);
    xini_file["config"]["url"] = unicode2ansi(update_url).c_str();

    // 保存listview内容，只保存相对路径、执行、解压、覆盖
    // 先获取数量并写入
    int item_count = ListView_GetItemCount(hList);
    xini_file["config"]["count"] = item_count;
    auto path = new wchar_t[MAXWORD];
    wchar_t exec[2]{}, unzip[2]{}, overwrite[2]{};
    for (int i = 0; i < item_count; i++) {
      string key("file" + to_string(i));

      ListView_GetItemText(hList, i, 1, path, MAXWORD);
      ListView_GetItemText(hList, i, 2, exec, 2);
      ListView_GetItemText(hList, i, 3, unzip, 2);
      ListView_GetItemText(hList, i, 4, overwrite, 2);

      xini_file[key]["path"] = unicode2ansi(path).c_str();
      //xini_file[key]["exec"] = std::wcstol(exec, nullptr, 10);
      //xini_file[key]["unzip"] = std::wcstol(unzip, nullptr, 10);
      xini_file[key]["exec"] = unicode2ansi(exec).c_str();
      xini_file[key]["unzip"] = unicode2ansi(unzip).c_str();
      xini_file[key]["overwrite"] = unicode2ansi(overwrite).c_str();
    }
    delete[] path;

    MessageBox(hwnd, L"保存配置完成，保存在程序目录的config.cfg文件", nullptr, MB_ICONINFORMATION | MB_OK);
  }

  if (id == IDC_BUTTON_LOAD) {
    string config_path = unicode2utf8(exe_path) + "config.cfg";
    // 如果不存在config.cfg则提示并退出
    if (_access_s(config_path.c_str(), 0) != 0) {
      MessageBox(hwnd, L"程序目录不存在config.cfg文件，无法读取配置", nullptr, MB_ICONERROR | MB_OK);
      return;
    }

    xini_file_t xini_file(config_path);
    update_path = ansi2unicode((const char*)xini_file["config"]["dir"]);
    update_url = ansi2unicode((const char*)xini_file["config"]["url"]);
    SetWindowText(GetDlgItem(hwnd, IDC_EDIT_DIR), update_path.data());
    SetWindowText(GetDlgItem(hwnd, IDC_EDIT_URL), update_url.data());

    // 设置添加目录的根路径
    LPSHELLFOLDER pShellFolder = nullptr;
    ignore = SHGetDesktopFolder(&pShellFolder);
    ignore = pShellFolder->ParseDisplayName(nullptr, nullptr, update_path.data(), nullptr, &update_idl, nullptr);

    // 如果Listview已有项目则清除
    if (ListView_GetItemCount(hList) > 0)
      ListView_DeleteAllItems(hList);

    // 循环读取config.cfg文件中的file?项，并添加到listview
    for (int i = 0; i < (int)xini_file["config"]["count"]; i++) {
      string key("file" + to_string(i));
      wstring value1(ansi2unicode((const char*)xini_file[key]["path"]));
      wstring value2(ansi2unicode((const char*)xini_file[key]["exec"]));
      wstring value3(ansi2unicode((const char*)xini_file[key]["unzip"]));
      wstring value4(ansi2unicode((const char*)xini_file[key]["overwrite"]));

      if constexpr (IS_DEBUG) {
        wcout << "path=" << exe_path + value1;
        wcout << " exec=" << value2;
        wcout << " unzip=" << value3;
        wcout << " overwrite=" << value4 << "\n";
      }

      const HANDLE hFile = CreateFile((update_path + value1).data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

      // 文件打开失败
      if (hFile == INVALID_HANDLE_VALUE) {
        cout << "CreateFile failed ";
        LastError();
        continue;
      }

      // 获取文件大小
      const DWORD dwSize = GetFileSize(hFile, nullptr);

      // 创建文件大小的字节指针，将文件读入指针，然后计算CRC32
      wstring crc32;
      const auto pFile = new BYTE[dwSize];
      if (ReadFile(hFile, pFile, dwSize, nullptr, nullptr)) {
        crc32 = calc_crc32(pFile, dwSize);
        delete[] pFile;
      }
      else {
        cout << format("ReadFile failed {}\n", GetLastError());
        LastError();
        delete[] pFile;
        continue;
      }

      // 关闭文件句柄
      if (!CloseHandle(hFile)) {
        cout << format("CloseHandle failed {}\n", GetLastError());
        LastError();
      }

      int item_count = ListView_GetItemCount(hList);
      // 插入Listview
      LVITEM row = {}; // 创建item结构体
      row.mask = LVIF_TEXT | LVIF_STATE;
      row.pszText = (LPWSTR)L"";
      row.iItem = item_count;
      row.iSubItem = 0;
      ListView_InsertItem(hList, &row);
      row.pszText = value1.data();
      row.iItem = item_count;
      row.iSubItem = 1;
      ListView_SetItem(hList, &row);
      row.pszText = value2.data();
      row.iItem = item_count;
      row.iSubItem = 2;
      ListView_SetItem(hList, &row);
      row.pszText = value3.data();
      row.iItem = item_count;
      row.iSubItem = 3;
      ListView_SetItem(hList, &row);
      row.pszText = value4.data();
      row.iItem = item_count;
      row.iSubItem = 4;
      ListView_SetItem(hList, &row);
      wstring tmp = std::to_wstring(dwSize);
      row.pszText = tmp.data();
      row.iItem = item_count;
      row.iSubItem = 5;
      ListView_SetItem(hList, &row);
      row.pszText = crc32.data();
      row.iItem = item_count;
      row.iSubItem = 6;
      ListView_SetItem(hList, &row);
    }
  }

  if (id == IDC_CHECK_EXEC) {
    vector<int> indexes;
    int iPos = ListView_GetNextItem(hList, -1, LVNI_SELECTED); // Get the first selected item
    // 先把所有选择的项保存到vector
    while (iPos != -1) {
      indexes.emplace_back(iPos);
      iPos = ListView_GetNextItem(hList, iPos, LVNI_SELECTED);
    }

    wchar_t exec[2]{};
    for (const auto& i : indexes) {
      ListView_GetItemText(hList, i, 2, exec, 2);  // NOLINT(clang-diagnostic-extra-semi-stmt)
      // 如果是空
      if (wcscmp(exec, L"") == 0) {
        ListView_SetItemText(hList, i, 2, (LPWSTR)L"1");// NOLINT(clang-diagnostic-extra-semi-stmt)
      }
      else {
        ListView_SetItemText(hList, i, 2, (LPWSTR)L"");// NOLINT(clang-diagnostic-extra-semi-stmt)
      }
    }
  }

  if (id == IDC_CHECK_UNZIP) {
    vector<int> indexes;
    int iPos = ListView_GetNextItem(hList, -1, LVNI_SELECTED); // Get the first selected item
    // 先把所有选择的项保存到vector
    while (iPos != -1) {
      indexes.emplace_back(iPos);
      iPos = ListView_GetNextItem(hList, iPos, LVNI_SELECTED);
    }

    wchar_t exec[2]{};
    for (const auto& i : indexes) {
      ListView_GetItemText(hList, i, 3, exec, 2);
      // 如果是空
      if (wcscmp(exec, L"") == 0) {
        ListView_SetItemText(hList, i, 3, (LPWSTR)L"1");
      }
      else {
        ListView_SetItemText(hList, i, 3, (LPWSTR)L"");
      }
    }
  }

  if (id == IDC_CHECK_OVERWRITE) {
    vector<int> indexes;
    int iPos = ListView_GetNextItem(hList, -1, LVNI_SELECTED); // Get the first selected item
    // 先把所有选择的项保存到vector
    while (iPos != -1) {
      indexes.emplace_back(iPos);
      iPos = ListView_GetNextItem(hList, iPos, LVNI_SELECTED);
    }

    wchar_t exec[2]{};
    for (const auto& i : indexes) {
      ListView_GetItemText(hList, i, 4, exec, 2);
      // 如果是空
      if (wcscmp(exec, L"") == 0) {
        ListView_SetItemText(hList, i, 4, (LPWSTR)L"1");
      }
      else {
        ListView_SetItemText(hList, i, 4, (LPWSTR)L"");
      }
    }
  }

  if (id == IDC_BUTTON_XML) {
    using namespace tinyxml2;

    tinyxml2::XMLDocument doc;
    XMLDeclaration* declaration = doc.NewDeclaration("xml version='1.0' encoding='UTF-8' standalone='yes'");


    XMLElement* root = doc.NewElement("Root");
    {
      XMLElement* config = doc.NewElement("config");

      // 生成时间元素
      XMLElement* create_time = doc.NewElement("time");
      tm t;   //tm结构指针
      time_t now;  //声明time_t类型变量
      ignore = time(&now);      //获取系统日期和时间保存在now
      ignore = localtime_s(&t, &now);   //将time_t格式的时间值now转换为tm结构保存在t，并针对本地时区进行更正
      char time_buf[100]{};
      ignore = std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &t);
      create_time->InsertEndChild(doc.NewText(time_buf));
      config->InsertEndChild(create_time);

      // 生成url元素
      XMLElement* url = doc.NewElement("url");
      url->InsertEndChild(doc.NewText(unicode2utf8(update_url).data()));
      config->InsertEndChild(url);

      // config插入到root
      root->InsertEndChild(config);
    }

    {
      XMLElement* filelist = doc.NewElement("filelist");

      // 遍历生成文件元素
      auto buf = new wchar_t[MAXWORD];
      string strtmp;
      for (int i = 0; i < ListView_GetItemCount(hList); i++) {
        XMLElement* file = doc.NewElement("file");

        ListView_GetItemText(hList, i, 1, buf, MAXWORD);
        strtmp = unicode2utf8(buf);
        XMLElement* path = doc.NewElement("path");
        path->InsertEndChild(doc.NewText(strtmp.data()));
        file->InsertEndChild(path);

        ListView_GetItemText(hList, i, 2, buf, MAXWORD);
        strtmp = unicode2utf8(buf);
        XMLElement* exec = doc.NewElement("exec");
        exec->InsertEndChild(doc.NewText(strtmp.data()));
        file->InsertEndChild(exec);

        ListView_GetItemText(hList, i, 3, buf, MAXWORD);
        strtmp = unicode2utf8(buf);
        XMLElement* unzip = doc.NewElement("unzip");
        unzip->InsertEndChild(doc.NewText(strtmp.data()));
        file->InsertEndChild(unzip);

        ListView_GetItemText(hList, i, 4, buf, MAXWORD);
        strtmp = unicode2utf8(buf);
        XMLElement* overwrite = doc.NewElement("overwrite");
        unzip->InsertEndChild(doc.NewText(strtmp.data()));
        file->InsertEndChild(overwrite);

        ListView_GetItemText(hList, i, 5, buf, MAXWORD);
        strtmp = unicode2utf8(buf);
        XMLElement* size = doc.NewElement("size");
        size->InsertEndChild(doc.NewText(strtmp.data()));
        file->InsertEndChild(size);

        ListView_GetItemText(hList, i, 6, buf, MAXWORD);
        strtmp = unicode2utf8(buf);
        XMLElement* CRC32 = doc.NewElement("CRC32");
        CRC32->InsertEndChild(doc.NewText(strtmp.data()));
        file->InsertEndChild(CRC32);

        filelist->InsertEndChild(file);
      }
      delete[] buf;

      // filelist插入到root
      root->InsertEndChild(filelist);
    }

    // 加入最顶层节点
    //doc.InsertEndChild(doc.NewDeclaration());
    doc.InsertEndChild(declaration);
    doc.InsertEndChild(root);

    string filename(unicode2utf8(exe_path) + "update.xml");
    doc.SaveFile(filename.c_str());

    if (doc.ErrorID()) {
      wstring wstr(L"文件保存错误, errorID=");
      wstr += to_wstring(doc.ErrorID());
      MessageBox(hwnd, wstr.data(), nullptr, MB_ICONERROR | MB_OK);
    }
    else
      MessageBox(hwnd, L"生成xml完成，保存在程序目录的update.xml文件", nullptr, MB_ICONINFORMATION | MB_OK);
  }
}

void Cls_OnNotify(HWND hDlg, UINT message, WPARAM wParam, const LPARAM lParam) {
  const auto hwnd = ((LPNMHDR)lParam)->hwndFrom;
  const auto msg = ((LPNMHDR)lParam)->code;
  const auto id = ((LPNMHDR)lParam)->idFrom;
  if (id == IDC_LIST_FILES) {
    if (msg == NM_CLICK) {
      const auto lpnmitem = (LPNMITEMACTIVATE)lParam;
      const auto buf = new wchar_t[2];

      // 获取执行、解压、覆盖列的状态，如果是1，则打钩
      // 获取执行列
      ListView_GetItemText(hwnd, lpnmitem->iItem, 2, buf, 2);
      if (std::wcscmp(buf, L"1") == 0)
        Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_EXEC), BST_CHECKED);
      else
        Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_EXEC), BST_UNCHECKED);

      // 获取解压列
      ListView_GetItemText(hwnd, lpnmitem->iItem, 3, buf, 2);
      if (std::wcscmp(buf, L"1") == 0)
        Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_UNZIP), BST_CHECKED);
      else
        Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_UNZIP), BST_UNCHECKED);

      // 获取覆盖列
      ListView_GetItemText(hwnd, lpnmitem->iItem, 4, buf, 2);
      if (std::wcscmp(buf, L"1") == 0)
        Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_OVERWRITE), BST_CHECKED);
      else
        Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_OVERWRITE), BST_UNCHECKED);

      delete[] buf;
    }
  }
}
