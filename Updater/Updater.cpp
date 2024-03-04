#include <iostream>
#include <tchar.h>
#include <Windows.h>

using namespace std;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                       _In_opt_ HINSTANCE hPrevInstance,
                       _In_ LPWSTR    lpCmdLine,
                       _In_ int       nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  if constexpr (_DEBUG) {
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

  LPWSTR* szArgList;
  int argCount;

  szArgList = CommandLineToArgvW(GetCommandLine(), &argCount);
  if (szArgList == NULL) {
    MessageBox(NULL, L"Unable to parse command line", L"Error", MB_OK);
    return 10;
  }

  for (int i = 0; i < argCount; i++) {
    MessageBox(NULL, szArgList[i], L"Arglist contents", MB_OK);
  }

  LocalFree(szArgList);

  return 0;
}