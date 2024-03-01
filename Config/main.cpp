﻿#include "header.h"
#include "resource.h"

using namespace std;

HINSTANCE g_hInst;
wstring update_path{};  // 更新目录
ITEMIDLIST* update_idl = nullptr;  // 更新目录的ITEMIDLIST

int WINAPI WinMain(_In_ const HINSTANCE hInstance,
                   _In_opt_  HINSTANCE hPrevInstance,
                   _In_  LPSTR lpCmdLine,
                   _In_  int nCmdShow) {
  g_hInst = hInstance;

  const HWND hdlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG_CONFIG), GetDesktopWindow(), ProcConfig);

  if (!hdlg)
    return 0;

  if constexpr (_DEBUG) {
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
  default:
    return FALSE;
  }
}

BOOL Cls_OnInitDialog(const HWND hwnd, HWND hwndFocus, LPARAM lParam) {
  // 调试用，指定更新目录，以及获取更新目录对应的IDL
  // https://winterdom.com/dev/ui/ishfolder/
  update_path = L"F:\\down\\gvo\\";
  LPSHELLFOLDER pShellFolder = nullptr;
  ignore = SHGetDesktopFolder(&pShellFolder);
  ignore = pShellFolder->ParseDisplayName(nullptr, nullptr, update_path.data(), nullptr, &update_idl, nullptr);

  SetWindowText(GetDlgItem(hwnd, IDC_EDIT_DIR), update_path.data());

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
  column.fmt = LVCFMT_CENTER; // 第一列始终左对齐，此选项为后面的列设置

  // 第1列
  column.pszText = (LPWSTR)L""; // 列标题
  column.cx = 23;//列宽
  column.iSubItem = 0;//子项索引，第一列无子项
  ListView_InsertColumn(hListview, 0, &column);
  // 第2列
  column.pszText = (LPWSTR)L"相对路径文件"; // 列标题
  column.cx = 400;//列宽
  column.iSubItem = 1;//子项索引
  ListView_InsertColumn(hListview, 1, &column);
  // 第3列
  column.pszText = (LPWSTR)L"执行"; // 列标题
  column.cx = 40;//列宽
  column.iSubItem = 2;//子项索引
  ListView_InsertColumn(hListview, 2, &column);
  // 第4列
  column.pszText = (LPWSTR)L"解压"; // 列标题
  column.cx = 40;//列宽
  column.iSubItem = 3;//子项索引
  ListView_InsertColumn(hListview, 3, &column);

  column.pszText = (LPWSTR)L"大小"; // 列标题
  column.cx = 120;//列宽
  column.iSubItem = 4;//子项索引
  ListView_InsertColumn(hListview, 4, &column);
  column.pszText = (LPWSTR)L"CRC32"; // 列标题
  column.cx = 80;//列宽
  column.iSubItem = 5;//子项索引
  ListView_InsertColumn(hListview, 5, &column);

  return TRUE;
}

void Cls_OnSysCommand(HWND hwnd, const UINT cmd, int x, int y) {
  if (cmd == SC_CLOSE) {
    PostQuitMessage(0);//退出
  }
}

void Cls_OnCommand(const HWND hwnd, const int id, HWND hwndCtl, UINT codeNotify) {
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

    // 使用SHBrowseForFolder函数获取文件夹的PIDL，再使用SHGetPathFromIDList函数获取文件夹的路径，存放在szFile
    update_idl = SHBrowseForFolder(&lpbi);
    if (update_idl) {
      SHGetPathFromIDList(update_idl, szFile);
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

        int item_count = ListView_GetItemCount(GetDlgItem(hwnd, IDC_LIST_FILES));
        LVITEM row = {}; // 创建item结构体
        row.mask = LVIF_TEXT | LVIF_STATE;
        row.pszText = (LPWSTR)L"";
        row.iItem = item_count;
        row.iSubItem = 0;
        ListView_InsertItem(GetDlgItem(hwnd, IDC_LIST_FILES), &row);
        row.pszText = filename.data();
        row.iItem = item_count;
        row.iSubItem = 1;
        ListView_SetItem(GetDlgItem(hwnd, IDC_LIST_FILES), &row);
        row.pszText = nullptr;
        row.iItem = item_count;
        row.iSubItem = 2;
        ListView_SetItem(GetDlgItem(hwnd, IDC_LIST_FILES), &row);
        row.pszText = nullptr;
        row.iItem = item_count;
        row.iSubItem = 3;
        ListView_SetItem(GetDlgItem(hwnd, IDC_LIST_FILES), &row);
        wstring tmp = std::to_wstring(dwSize);
        row.pszText = tmp.data();
        row.iItem = item_count;
        row.iSubItem = 4;
        ListView_SetItem(GetDlgItem(hwnd, IDC_LIST_FILES), &row);
        row.pszText = crc32.data();
        row.iItem = item_count;
        row.iSubItem = 5;
        ListView_SetItem(GetDlgItem(hwnd, IDC_LIST_FILES), &row);
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

      // 获取临时listview控件句柄
      HWND hList = GetDlgItem(hwnd, IDC_LIST_FILES);

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
          row.pszText = nullptr;
          row.iItem = item_count;
          row.iSubItem = 2;
          ListView_SetItem(hList, &row);
          row.pszText = nullptr;
          row.iItem = item_count;
          row.iSubItem = 3;
          ListView_SetItem(hList, &row);
          wstring tmp = std::to_wstring(dwSize);
          row.pszText = tmp.data();
          row.iItem = item_count;
          row.iSubItem = 4;
          ListView_SetItem(hList, &row);
          row.pszText = crc32.data();
          row.iItem = item_count;
          row.iSubItem = 5;
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

  }
}