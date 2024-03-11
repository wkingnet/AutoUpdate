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

#include "AutoUpdate.h"

using namespace std;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                       _In_opt_ HINSTANCE hPrevInstance,
                       _In_ LPWSTR    lpCmdLine,
                       _In_ int       nShowCmd) {

  AutoUpdate::AutoUpdate(nullptr, L"Updater.exe", L"http://192.168.74.10/update.xml", true);
}