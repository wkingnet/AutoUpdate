#include "header.h"
#include "resource.h"

HINSTANCE hgInst;
int WINAPI WinMain(_In_  HINSTANCE hInstance,
                   _In_opt_  HINSTANCE hPrevInstance,
                   _In_  LPSTR lpCmdLine,
                   _In_  int nCmdShow) {
  hgInst = hInstance;
  HWND hdlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG_CONFIG), GetDesktopWindow(), (DLGPROC)ProcConfig);
  if (!hdlg) {
    return 0;
  }
  ShowWindow(hdlg, SW_SHOW);
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return 0;
}

INT_PTR CALLBACK ProcConfig(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

  switch (msg) {
  case WM_INITDIALOG:
    return 0;
  case WM_SYSCOMMAND:
    if (wParam == SC_CLOSE) {
      PostQuitMessage(0);//退出
    }
    return 0;
  }
  return (INT_PTR)FALSE;
}