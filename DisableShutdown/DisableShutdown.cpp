// DisableShutdown.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。

// 版权声明：
// Copyright (C) 2013-2022 ETO.                              All Rights Reserved.
// Copyright (C) 2018-2022 FreeSTD Inc.                      All Rights Reserved.
// Copyright (C) 2018-2022 Chengdu Zuosi Co.,Ltd.            All Rights Reserved.
// Copyright (C) 2020-2022 Coral Studio.                     All Rights Reserved.
// Copyright (C) 2022      North A516 Treaty Organization.   All Rights Reserved.
// Copyright (C) 2022      Sunny Orange Electronics Co.,Ltd. All Rights Reserved.

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <Windows.h>
#include <string>
using namespace std;

bool IsDisable, IsSuccessfulHint;
string TargetDir;
string GetTargetDir();

int main(int argc, char* argv[])
{
	IsSuccessfulHint = 1;
    if (argc != 2)
        return 0;
	if (!strcmp(argv[1], "disable"))
		IsDisable = 1;
	else if (!strcmp(argv[1], "enable"))
		IsDisable = 0;
	else if (!strcmp(argv[1], "disablesilent"))
		IsDisable = 1, IsSuccessfulHint = 0;
    else
        return 0;

	HWND hWnd = FindWindow(L"KILLERGUI_V2_MYTHWARE", L"");
    
	TargetDir = GetTargetDir();
	if (TargetDir == "")
	{
		MessageBox(hWnd, L"未获取到有效的极域电子教室安装位置信息！", L"极域电子教室学生端破解程序", MB_OK | MB_ICONERROR);
		return 0;
	}

	string Origin = TargetDir + "Shutdown.exe";
	string Edited = TargetDir + "__Shutdown.exe";

	if (IsDisable)
		if (!rename(Origin.c_str(), Edited.c_str()))
		{
			if (IsSuccessfulHint)
				MessageBox(hWnd, L"成功禁用了教师端强制关机！", L"极域电子教室学生端破解程序", MB_OK | MB_ICONINFORMATION);
		}
		else
			MessageBox(hWnd, L"禁用教师端强制关机失败！", L"极域电子教室学生端破解程序", MB_OK | MB_ICONERROR);
	else
		if (!rename(Edited.c_str(), Origin.c_str()))
			MessageBox(hWnd, L"成功恢复了教师端强制关机！", L"极域电子教室学生端破解程序", MB_OK | MB_ICONINFORMATION);
		else
			MessageBox(hWnd, L"恢复教师端强制关机失败！", L"极域电子教室学生端破解程序", MB_OK | MB_ICONERROR);

    return 0;
}

string GetTargetDir()
{
	bool IsFail = 0;
	std::string sKeyValue = "";
	HKEY hKey;
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\TopDomain\\e-Learning Class V6.0", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		IsFail = 1;
	}
	char chValue[256] = { 0 };
	DWORD dwSzType = REG_SZ;
	DWORD dwSize = sizeof(chValue);
	if (RegQueryValueExA(hKey, "TargetDirectory", 0, &dwSzType, (LPBYTE)&chValue, &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		IsFail = 1;
	}
	RegCloseKey(hKey);
	string Dir = chValue;
	if (!IsFail)
		return Dir;

	IsFail = 0;
	memset(chValue, 0, sizeof(chValue));
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\TopDomain\\e-Learning Class Standard\\1.00\\", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		IsFail = 1;
	}
	dwSzType = REG_SZ;
	dwSize = sizeof(chValue);
	if (RegQueryValueExA(hKey, "TargetDirectory", 0, &dwSzType, (LPBYTE)&chValue, &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		IsFail = 1;
	}
	RegCloseKey(hKey);
	Dir = chValue;
	if (!IsFail)
		return Dir;

	return "";
}