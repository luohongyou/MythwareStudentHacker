// KillerGUI.cpp : 定义主程序的入口点。

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

#define _BETA "14.0 beta 2"
#define PROGRAM_DISPLAY_NAME L"极域电子教室学生端破解程序14.0 beta 2"
#define PROGRAM_DISPLAY_NAME_A "极域电子教室学生端破解程序14.0 beta 2"

#include "framework.h"
#include "KillerGUI.h"
#include "Build.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <comdef.h>
#include <shlobj.h>
#include <atlconv.h>
#include <CommCtrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <thread>
#include <string>
#include <sstream>
#include <fstream>
#include <assert.h>

#pragma comment(lib , "Comctl32.lib")
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1 : 0)
#define WM_NOTIFYICON WM_USER + 0x01

using std::thread;
using std::string;
using std::stringstream;
using std::ifstream;
using std::ofstream;

// 全局变量
// 主程序
HINSTANCE hInst;                                // 当前实例
HWND hWnd;                                      // 主窗口句柄
HWND TrayhWnd;                                  // 托盘图标窗口句柄
NOTIFYICONDATA nid = { 0 };                     // 托盘图标数据
double DPI;                                     // 高DPI显示比例
bool IsWindowMode;                              // 是否为窗口模式
string TargetDir;                               // 是否为窗口模式
bool FirstClose;                                // 是否显示恢复窗口提示
string CompileInfo;                             // 编译信息

// 禁用黑屏肃静屏幕广播
bool isTeacherScreenKillerEnabled;              // 禁用黑屏肃静屏幕广播是否打开
bool isTeacherScreenShow;                       // 黑屏肃静屏幕广播是否显示
int isTeacherScreenBox;                         // 黑屏肃静屏幕广播是否再次提示

// 控件信息
HDC hdcStatic;

HWND CompileInfoHWND;
HFONT CompileInfoFont;
#define ID_COMINFO   20001

// 此代码模块中包含的函数的前向声明
void             CreateTray(HINSTANCE hInstance);
LRESULT CALLBACK TrayProc(HWND, UINT, WPARAM, LPARAM);
void             CreateMainWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void             InstallDlltoProcess(std::string ProcessName, std::string DllName);

void             APIHookChecker();
void             APIHookInstaller();
void             InstallFailDLL();

string           GetTargetDir();

void             ScreenKiller(HWND(*_GetHWND)(), LPCWSTR Title);
void             ScreenKillerBox(HWND RedHWND, LPCWSTR Title);
HWND             BlackScreen();
HWND             TeacherScreen();
HWND             StudentScreen();

int              CheckProcess(std::string s);
void             StopProcess(std::string s);
void             NTSDStopProcess(std::string s);
DWORD            WinExecAndWait32(const char* lpszAppPath, const char* lpParameters, const char* lpszDirectory, DWORD dwMilliseconds);

bool             KillStudentMain();
bool             RecoveryStudentMain();

string           GetCompileInfo();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	// 创建互斥体
	HANDLE hMutex = NULL;
	hMutex = CreateMutexA(NULL, FALSE, "MythwareKiller");
	if (hMutex != NULL)
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			ReleaseMutex(hMutex);
			hWnd = FindWindow(L"KILLERGUI_V2_MYTHWARE", L"");
			ShowWindow(::hWnd, SW_SHOW);
			SetForegroundWindow(::hWnd);
			return 0;
		}

	// 初始化工作目录
	char strModule[256];
	GetModuleFileNameA(NULL, strModule, 256);
	string a;
	a.assign(strModule);
	a.append("\\..\\");
	SetCurrentDirectoryA(a.c_str());

	// 获取极域信息
	TargetDir = GetTargetDir();

	// 载入设置
	IsWindowMode = 1;
	isTeacherScreenKillerEnabled = 0;
	isTeacherScreenBox = 1;
	FirstClose = 1;
	CompileInfo = GetCompileInfo();

	// 获取高DPI显示比例
	int _TDPI = GetDeviceCaps(GetDC(NULL), LOGPIXELSX);
	DPI = _TDPI / 96.0;

	// 创建托盘图标
	CreateTray(hInstance);
    // 创建主窗口
	CreateMainWindow(hInstance);

	// 提前启动功能
	thread AHC(APIHookChecker);
	AHC.detach();
	thread TSK1(ScreenKiller, BlackScreen, L"禁用黑屏肃静");
	TSK1.detach();
	thread TSK2(ScreenKiller, TeacherScreen, L"禁用屏幕广播");
	TSK2.detach();
	thread TSK3(ScreenKiller, StudentScreen, L"禁用学生演示");
	TSK3.detach();

	// 主消息循环
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KILLERGUI));
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


void CreateTray(HINSTANCE hInstance)
{
	// 注册窗口类
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = TrayProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KILLERGUI));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_KILLERGUI);
	wcex.lpszClassName = L"KILLERGUI_V2_MYTHWARE_TRAY";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassExW(&wcex);

	// 将实例句柄存储在全局变量中
	hInst = hInstance;

	TrayhWnd = CreateWindowW(L"KILLERGUI_V2_MYTHWARE_TRAY", L"TrayWindow", WS_POPUPWINDOW,
		0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);

	// 检查是否创建成功
	if (!TrayhWnd)
	{
		MessageBox(NULL, L"创建托盘图标窗口失败！", PROGRAM_DISPLAY_NAME, MB_OK | MB_ICONERROR);
		::exit(0);
	}

	// 刷新窗口
	ShowWindow(TrayhWnd, SW_HIDE);
	UpdateWindow(TrayhWnd);

	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = TrayhWnd;
	nid.uID = IDI_SMALL;
	nid.hIcon = ::LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALL));
	nid.uCallbackMessage = WM_NOTIFYICON;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	_tcscpy_s(nid.szTip, PROGRAM_DISPLAY_NAME);
	Shell_NotifyIcon(NIM_ADD, &nid);
}
LRESULT CALLBACK TrayProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 分析菜单选择
		switch (wmId)
		{
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_NOTIFYICON:
		if ((wParam == IDI_SMALL) && (lParam == WM_LBUTTONDOWN))
		{
			ShowWindow(::hWnd, SW_SHOW);
			SetForegroundWindow(::hWnd);
		}
		else if ((wParam == IDI_SMALL) && (lParam == WM_RBUTTONDOWN))
		{
			POINT pt;
			GetCursorPos(&pt);
			SetForegroundWindow(::hWnd);
			HMENU hMenu, hMenu1;
			hMenu = LoadMenu(hInst, MAKEINTRESOURCEW(IDC_NOTIFYICON));
			hMenu1 = GetSubMenu(hMenu, 0);
			WPARAM ReturnMsg;
			ReturnMsg = TrackPopupMenu(hMenu1, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, NULL, hWnd, NULL);
			if (ReturnMsg > 0)
			{
				SendMessage(::hWnd, WM_COMMAND, ReturnMsg, 0);
				ShowWindow(::hWnd, SW_SHOW);
				SetForegroundWindow(::hWnd);
			}
		}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此处添加使用 hdc 的任何绘图代码...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void CreateMainWindow(HINSTANCE hInstance)
{
	// 注册窗口类
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KILLERGUI));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = CreatePatternBrush((HBITMAP)LoadImage(NULL, L"Resources\\Background.bmp", IMAGE_BITMAP, 950 * DPI, 425 * DPI, LR_LOADFROMFILE));
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_KILLERGUI);
	wcex.lpszClassName = L"KILLERGUI_V2_MYTHWARE";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassExW(&wcex);

	// 将实例句柄存储在全局变量中
	hInst = hInstance;

	// 检查分辨率
	int scrWidth, scrHeight;
	scrWidth = (GetSystemMetrics(SM_CXSCREEN) - 960 * DPI) / 2;
	scrHeight = (GetSystemMetrics(SM_CYSCREEN) - 480 * DPI) / 2;
	if (scrWidth < 0 || scrHeight < 0)
	{
		USES_CONVERSION;
		int DPIPercent = DPI * 100;
		string sDPIPercent, sW, sH;
		stringstream tempIO, tempIO1, tempIO2;
		tempIO << DPIPercent;
		tempIO >> sDPIPercent;
		tempIO1 << 960 * DPI;
		tempIO1 >> sW;
		tempIO2 << 480 * DPI;
		tempIO2 >> sH;
		string Message = (string)""
			+ "对于显示比例设置为 " + sDPIPercent + "% 的计算机，本程序需要 " + sW + " × " + sH + " 或更高分辨率以达到最优效果。\n"
			+ "要解决此问题，可以尝试以下方法之一：\n"
			+ "  ● 增加分辨率\n"
			+ "  ● 减小显示比例\n";
		InitCommonControls();
		int nButtonPressed = 0;
		TASKDIALOGCONFIG config = { 0 };
		const TASKDIALOG_BUTTON buttons[] = {
			{ IDOK, L"忽略该问题，继续运行程序\n程序的某些模块可能会显示错误" },
			{ 100, L"退出程序\n稍后您可以重新打开程序" }
		};
		config.cbSize = sizeof(config);
		config.hInstance = hInst;
		config.dwFlags = TDF_USE_COMMAND_LINKS;
		config.dwCommonButtons = 0;
		config.pszMainIcon = TD_WARNING_ICON;
		config.pszWindowTitle = PROGRAM_DISPLAY_NAME;
		config.pszMainInstruction = L"检查您的显示设置";
		config.pszContent = A2W(Message.c_str());
		config.pButtons = buttons;
		config.cButtons = ARRAYSIZE(buttons);
		TaskDialogIndirect(&config, &nButtonPressed, NULL, NULL);
		switch (nButtonPressed)
		{
		case IDOK:
			break;
		case 100:
			::exit(0);
		default:
			break;
		}
	}

	// 创建主窗口
	if (IsWindowMode)
		hWnd = CreateWindowW(L"KILLERGUI_V2_MYTHWARE", L"", WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
			scrWidth, scrHeight, 960 * DPI, 480 * DPI, nullptr, nullptr, hInstance, nullptr);
	else
		hWnd = CreateWindowW(L"KILLERGUI_V2_MYTHWARE", L"", WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
			scrWidth, scrHeight, 960 * DPI, 56 * DPI, nullptr, nullptr, hInstance, nullptr);
	
	// 检查是否创建成功
	if (!hWnd)
	{
		MessageBox(NULL, L"创建主窗口失败！", PROGRAM_DISPLAY_NAME, MB_OK | MB_ICONERROR);
		::exit(0);
	}

	// 刷新窗口
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	// 创建控件
	CompileInfoFont = CreateFont(-15, -7, 0, 0, 400, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"微软雅黑");
	CompileInfoHWND = CreateWindowA("static", CompileInfo.c_str(),
		WS_CHILD | WS_VISIBLE | SS_RIGHT,
		960 * DPI - 470, 480 * DPI - 60 * DPI - 20, 450, 20,
		hWnd, (HMENU)ID_COMINFO, hInst, NULL
	);
	SendMessage(CompileInfoHWND, WM_SETFONT, (WPARAM)CompileInfoFont, NULL);

	// 功能选择菜单
	InitCommonControls();
	int nButtonPressed = 0;
	TASKDIALOGCONFIG config = { 0 };
	const TASKDIALOG_BUTTON buttons[] = {
		{ 100, L"启用推荐的设置\n包括开启隐藏屏幕广播提示、禁用强制关机等" },
		{ 200, L"强行结束极域电子教室\n教师端将显示您掉线的信息" },
		{ 300, L"不预先启用功能" }
	};
	config.cbSize = sizeof(config);
	config.hInstance = hInst;
	config.hwndParent = hWnd;
	config.dwFlags = TDF_USE_COMMAND_LINKS;
	config.dwCommonButtons = 0;
	config.pszMainIcon = TD_INFORMATION_ICON;
	config.pszWindowTitle = PROGRAM_DISPLAY_NAME;
	config.pszMainInstruction = L"欢迎使用 极域电子教室学生端破解程序";
	config.pszContent = L"您现在可以快捷地启用预设的功能。";
	config.pButtons = buttons;
	config.cButtons = ARRAYSIZE(buttons);
	TaskDialogIndirect(&config, &nButtonPressed, NULL, NULL);
	switch (nButtonPressed)
	{
	case 100:
		isTeacherScreenKillerEnabled = 1;
		ShellExecuteA(NULL, "runas", "Resources\\DisableShutdown.exe", " disablesilent", NULL, SW_HIDE);
		break;
	case 200:
		if (CheckProcess("StudentMain.exe"))
			if (KillStudentMain())
				MessageBox(hWnd, L"成功地结束了极域电子教室主进程！", PROGRAM_DISPLAY_NAME, MB_ICONINFORMATION | MB_OK);
			else
				MessageBox(hWnd, L"结束极域电子教室主进程失败！", PROGRAM_DISPLAY_NAME, MB_ICONERROR | MB_OK);
		else
			MessageBox(hWnd, L"极域电子教室没有运行！", PROGRAM_DISPLAY_NAME, MB_ICONERROR | MB_OK);
		break;
	default:
		break;
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CTLCOLORSTATIC:
		hdcStatic = (HDC)wParam;
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkMode(hdcStatic, TRANSPARENT);
		return (INT_PTR)GetStockObject(NULL_BRUSH);

	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		if (FirstClose)
		{
			nid.uFlags = NIF_INFO | NIF_GUID;
			nid.dwInfoFlags = 0x00000004;
			_tcscpy_s(nid.szInfo, L"程序将在后台继续运行，单击托盘图标恢复窗口。");
			_tcscpy_s(nid.szTip, PROGRAM_DISPLAY_NAME);
			lstrcpy(nid.szInfoTitle, PROGRAM_DISPLAY_NAME);
			nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALL));
			Shell_NotifyIcon(NIM_MODIFY, &nid);
			FirstClose = 0;
		}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择
            switch (wmId)
            {
			case ID_Window:
			{
				int tempY;
				tempY = (IsWindowMode ? (480 - 4) * DPI : (56 + 4) * DPI);
				for (int i = 1; i <= 35; i++)
				{
					tempY += (IsWindowMode ? -12 * DPI : 12 * DPI);
					SetWindowPos(hWnd, NULL, NULL, NULL, 960 * DPI, tempY, SWP_NOMOVE | SWP_NOZORDER);
				}
				IsWindowMode = !IsWindowMode;
			}
			break;
			case ID_TSKEn:
				if (isTeacherScreenKillerEnabled)
					MessageBox(hWnd, L"该功能已启用！", PROGRAM_DISPLAY_NAME, MB_OK | MB_ICONERROR);
				else
				{
					isTeacherScreenKillerEnabled = 1;
					MessageBox(hWnd, L"成功启用了禁用黑屏肃静屏幕广播！", PROGRAM_DISPLAY_NAME, MB_OK | MB_ICONINFORMATION);
				}
				break;
			case ID_TSKDis:
				if (!isTeacherScreenKillerEnabled)
					MessageBox(hWnd, L"该功能已禁用！", PROGRAM_DISPLAY_NAME, MB_OK | MB_ICONERROR);
				else
				{
					isTeacherScreenKillerEnabled = 0;
					MessageBox(hWnd, L"成功取消了禁用黑屏肃静屏幕广播！", PROGRAM_DISPLAY_NAME, MB_OK | MB_ICONINFORMATION);
				}
				break;
			case ID_KILLPROCESS:
				if (CheckProcess("StudentMain.exe"))
					if (KillStudentMain())
						MessageBox(hWnd, L"成功地结束了极域电子教室主进程！", PROGRAM_DISPLAY_NAME, MB_ICONINFORMATION | MB_OK);
					else
						MessageBox(hWnd, L"直接结束极域电子教室主进程失败！已尝试注入退出DLL！", PROGRAM_DISPLAY_NAME, MB_ICONWARNING | MB_OK);
				else
					MessageBox(hWnd, L"极域电子教室没有运行！", PROGRAM_DISPLAY_NAME, MB_ICONERROR | MB_OK);
				break;
			case ID_RECOVERY:
				if (TargetDir != "")
					if (!CheckProcess("StudentMain.exe"))
						if (RecoveryStudentMain())
							MessageBox(hWnd, L"成功地恢复了极域电子教室运行！", PROGRAM_DISPLAY_NAME, MB_ICONINFORMATION | MB_OK);
						else
							MessageBox(hWnd, L"恢复极域电子教室运行失败！", PROGRAM_DISPLAY_NAME, MB_ICONERROR | MB_OK);
					else
						MessageBox(hWnd, L"极域电子教室已经在运行！", PROGRAM_DISPLAY_NAME, MB_ICONERROR | MB_OK);
				else
					MessageBox(hWnd, L"未获取到有效的极域电子教室安装位置信息！", PROGRAM_DISPLAY_NAME, MB_OK | MB_ICONERROR);
				break;
			case ID_DISABLESHUTDOWN:
				ShellExecuteA(NULL, "runas", "Resources\\DisableShutdown.exe", " disable", NULL, SW_HIDE);
				break;
			case ID_ENABLESHUTDOWN:
				ShellExecuteA(NULL, "runas", "Resources\\DisableShutdown.exe", " enable", NULL, SW_HIDE);
				break;
			case ID_PASSWORD:
				ShellExecuteA(NULL, "open", "explorer", "Resources\\RemovePassword.reg", NULL, SW_SHOW);
				break;
			case ID_OPENADDRESS:
				if (TargetDir == "")
				{
					MessageBox(hWnd, L"未获取到有效的极域电子教室安装位置信息！", PROGRAM_DISPLAY_NAME, MB_OK | MB_ICONERROR);
					break;
				}
				ShellExecuteA(NULL, "open", "explorer", TargetDir.c_str(), NULL, SW_SHOW);
				break;
			case ID_RELOADHOOK:
				if (TargetDir == "")
				{
					MessageBox(hWnd, L"未获取到有效的极域电子教室安装位置信息！", PROGRAM_DISPLAY_NAME, MB_OK | MB_ICONERROR);
					break;
				}
				KillStudentMain();
				RecoveryStudentMain();
				break;
			case ID_KILLDLL:
				InstallFailDLL();
				break;
            case IDM_ABOUT:
			{
				InitCommonControls();
				TASKDIALOGCONFIG config = { 0 };
				config.cbSize = sizeof(config);
				config.hInstance = hInst;
				config.dwCommonButtons = 0;
				config.pszMainIcon = TD_INFORMATION_ICON;
				config.pszWindowTitle = PROGRAM_DISPLAY_NAME;
				config.hwndParent = hWnd;
				config.pszMainInstruction = PROGRAM_DISPLAY_NAME;
				config.pszContent = 
LR"(版权声明：
(C)2013-2022 ETO. All Rights Reserved.
(C)2018-2022 FreeSTD Inc. All Rights Reserved.
(C)2018-2022 Chengdu Zuosi Co.,Ltd. All Rights Reserved.
(C)2020-2022 Coral Studio. All Rights Reserved.
(C)2022 North A516 Treaty Organization. All Rights Reserved.
(C)2022 Sunny Orange Electronics Co.,Ltd. All Rights Reserved.)";
				TaskDialogIndirect(&config, NULL, NULL, NULL);
			}
            break;
			case ID_SHOW:
				ShowWindow(hWnd, SW_SHOW);
				SetForegroundWindow(hWnd);
				break;
            case IDM_EXIT:
			{
				InitCommonControls();
				int nButtonPressed = 0;
				TASKDIALOGCONFIG config = { 0 };
				const TASKDIALOG_BUTTON buttons[] = {
					{ IDOK, L"继续退出\n您将不会受到保护" },
					{ 100, L"暂不退出" }
				};
				config.cbSize = sizeof(config);
				config.hInstance = hInst;
				config.hwndParent = hWnd;
				config.dwFlags = TDF_USE_COMMAND_LINKS;
				config.dwCommonButtons = 0;
				config.pszMainIcon = TD_WARNING_ICON;
				config.pszWindowTitle = L"退出";
				config.pszMainInstruction = L"退出程序";
				config.pszContent = L"确实要退出 极域电子教室学生端破解程序？";
				config.pButtons = buttons;
				config.cButtons = ARRAYSIZE(buttons);
				TaskDialogIndirect(&config, &nButtonPressed, NULL, NULL);
				switch (nButtonPressed)
				{
				case IDOK:
					DestroyWindow(hWnd);
				case 100:
					break;
				default:
					break;
				}
			}
            break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = TrayhWnd;
		nid.uID = IDI_SMALL;
		Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int CheckProcess(std::string s)
{
	HANDLE hToken;
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
	TOKEN_PRIVILEGES tp;
	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	PROCESSENTRY32 pd;
	pd.dwSize = sizeof(pd);
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	BOOL choose = ::Process32First(hProcessSnap, &pd);
	int stdexe = 0;
	while (choose)
	{
		char szEXE[100];
		_bstr_t b(pd.szExeFile);
		strcpy_s(szEXE, b);
		if (!strcmp(szEXE, s.c_str()))
			++stdexe;
		choose = ::Process32Next(hProcessSnap, &pd);
	}
	return stdexe;
}
void StopProcess(std::string s)
{
	HANDLE hToken;
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
	TOKEN_PRIVILEGES tp;
	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	PROCESSENTRY32 pd;
	pd.dwSize = sizeof(pd);
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	BOOL choose = ::Process32First(hProcessSnap, &pd);
	while (choose)
	{
		char szEXE[100];
		_bstr_t b(pd.szExeFile);
		strcpy_s(szEXE, b);
		if (!strcmp(szEXE, s.c_str()))
		{
			HANDLE std = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pd.th32ProcessID);
			::TerminateProcess(std, 0);
		}
		choose = ::Process32Next(hProcessSnap, &pd);
	}
}
DWORD WinExecAndWait32(const char* lpszAppPath, const char* lpParameters, const char* lpszDirectory, DWORD dwMilliseconds)
{
	SHELLEXECUTEINFOA ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = lpszAppPath;
	ShExecInfo.lpParameters = lpParameters;
	ShExecInfo.lpDirectory = lpszDirectory;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteExA(&ShExecInfo);

	if (WaitForSingleObject(ShExecInfo.hProcess, dwMilliseconds) == WAIT_TIMEOUT)
	{
		TerminateProcess(ShExecInfo.hProcess, 0);
		return 0;
	}

	DWORD dwExitCode;
	BOOL bOK = GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode);
	assert(bOK);

	return dwExitCode;
}
void NTSDStopProcess(std::string s)
{
	DWORD PID;
	string sPID, Command;

	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);
		return;
	}
	char szEXE[100];
	do
	{
		_bstr_t b(pe32.szExeFile);
		strcpy_s(szEXE, b);
		if (!strcmp(s.c_str(), szEXE))
		{
			PID = pe32.th32ProcessID;
			stringstream tempIO;
			tempIO << PID;
			tempIO >> sPID;
			Command = " -c q -p " + sPID;
			WinExecAndWait32("Resources\\ntsd.exe", Command.c_str(), "", 10000);
		}
	} 
	while (Process32Next(hProcessSnap, &pe32));
	CloseHandle(hProcessSnap);
}
void InstallFailDLL()
{
	InstallDlltoProcess("StudentMain.exe", "APIKill.dll");
}
bool KillStudentMain()
{
	StopProcess("StudentMain.exe");
	Sleep(500);
	if (!CheckProcess("StudentMain.exe"))
		return true;

	NTSDStopProcess("StudentMain.exe");
	Sleep(500);
	if (!CheckProcess("StudentMain.exe"))
		return true;

	InstallFailDLL();
	return false;
}
bool RecoveryStudentMain()
{
	ShellExecuteA(NULL, "open", (TargetDir + "StudentMain.exe").c_str(), "", NULL, SW_SHOW);
	Sleep(500);
	return CheckProcess("StudentMain.exe");
}
void InstallDlltoProcess(std::string ProcessName, std::string DllName)
{
	std::string UserTempPath;
	char ttp[MAX_PATH];
	GetTempPathA(260, ttp);
	UserTempPath = ttp;
	const DWORD dwThreadSize = 5 * 1024;
	DWORD dwWriteBytes, dwProcessId;
	HANDLE hToken;
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
	TOKEN_PRIVILEGES tp;
	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	PROCESSENTRY32 pd;
	pd.dwSize = sizeof(pd);
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	BOOL choose = ::Process32First(hProcessSnap, &pd);
	while (choose)
	{
		char szEXE[100];
		_bstr_t b(pd.szExeFile);
		strcpy_s(szEXE, b);
		if (!strcmp(szEXE, ProcessName.c_str()))
		{
			dwProcessId = pd.th32ProcessID;

			HANDLE hTargetProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
			if (!hTargetProcess)
			{
				MessageBoxA(hWnd, ("错误：无法注入DLL：" + DllName + "，无法操作进程" + ProcessName + "!").c_str(), PROGRAM_DISPLAY_NAME_A, MB_ICONERROR | MB_OK);
				break;
			}
			void* pRemoteThread = VirtualAllocEx(hTargetProcess,
				0,
				dwThreadSize,
				MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			if (!pRemoteThread)
				break;
			char szDll[5120];
			memset(szDll, 0, 5120);
			CopyFileA(("Resources\\" + DllName).c_str(), (UserTempPath + DllName).c_str(), FALSE);
			strcpy_s(szDll, (UserTempPath + DllName).c_str());
			if (!WriteProcessMemory(hTargetProcess,
				pRemoteThread,
				(LPVOID)szDll,
				dwThreadSize,
				0))
			{
				MessageBoxA(hWnd, ("错误：无法注入DLL：" + DllName + "，无法执行WriteProcessMemory!").c_str(), PROGRAM_DISPLAY_NAME_A, MB_ICONERROR | MB_OK);
				break;
			}
			LPVOID pFunc = LoadLibraryA;
			HANDLE hRemoteThread = CreateRemoteThread(hTargetProcess,
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)pFunc,
				pRemoteThread,
				0,
				&dwWriteBytes);
			if (!hRemoteThread)
			{
				MessageBoxA(hWnd, ("错误：无法注入DLL：" + DllName + "，无法创建线程!").c_str(), PROGRAM_DISPLAY_NAME_A, MB_ICONERROR | MB_OK);
				break;
			}
			WaitForSingleObject(hRemoteThread, INFINITE);
			VirtualFreeEx(hTargetProcess, pRemoteThread, dwThreadSize, MEM_COMMIT);
			CloseHandle(hRemoteThread);
		}
		choose = ::Process32Next(hProcessSnap, &pd);
	}
	if (dwProcessId == 0)
	{
		MessageBoxA(hWnd, ("错误：无法注入DLL：" + DllName + "，未找到进程" + ProcessName + "!").c_str(), PROGRAM_DISPLAY_NAME_A, MB_ICONERROR | MB_OK);
		return;
	}
}
void APIHookChecker()
{
	bool isHook = 0;
	while (1)
	{
		if (isHook != CheckProcess("StudentMain.exe"))
			if (isHook)
				isHook = 0;
			else
			{
				isHook = 1;
				Sleep(500);
				APIHookInstaller();
			}
		Sleep(500);
	}
}
void APIHookInstaller()
{
	InstallDlltoProcess("StudentMain.exe", "APIHook.dll");
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

// 用于获得黑屏肃静屏幕广播窗口状态的函数
string ENUM_stdTitle;
string ENUM_SearchTitle;
bool ENUM_ans;
BOOL CALLBACK EnumWindowsProc(
	HWND hwnd,
	LPARAM lParam)
{
	char caption[200];
	memset(caption, 0, sizeof(caption));
	::GetWindowTextA(hwnd, caption, 200);
	if (strcmp(caption, ""))
	{
		ENUM_stdTitle = caption;
		if (ENUM_stdTitle.find("正在共享屏幕") != string::npos)
			ENUM_ans = true, ENUM_SearchTitle = ENUM_stdTitle;
	}
	return TRUE;
}
HWND BlackScreen()
{
	HWND _hWnd = ::FindWindow(NULL, L"BlackScreen Window");
	//if (_hWnd && !TeacherScreen() && !StudentScreen())
		return _hWnd;
	//else 
	//	return NULL;
}
HWND TeacherScreen()
{
	HWND _hWnd;
	_hWnd = ::FindWindow(NULL, L"屏幕广播");
	if (!_hWnd)
		_hWnd = ::FindWindow(NULL, L"屏幕演播室窗口");
	return _hWnd;
}
HWND StudentScreen()
{
	HWND hwnd = NULL;
	ENUM_stdTitle = "";
	ENUM_ans = false;
	EnumWindows(EnumWindowsProc, NULL);
	if (ENUM_ans)
		hwnd = ::FindWindowA(NULL, ENUM_SearchTitle.c_str());
	return hwnd;
}
void ScreenKiller(HWND(*_GetHWND)(), LPCWSTR Title)
{
	isTeacherScreenShow = 0;
	HWND FSHWND = NULL;
	while (1)
	{
		while (!FSHWND || !isTeacherScreenKillerEnabled)
		{
			FSHWND = _GetHWND();
			Sleep(100);
		}
		if (isTeacherScreenBox)
		{
			thread TSBox(ScreenKillerBox, FSHWND, Title);
			TSBox.detach();
		}
		isTeacherScreenShow = 1;
		while (FSHWND)
		{
			if (KEY_DOWN(VK_CONTROL) && KEY_DOWN(VK_SPACE))
			{
				if (isTeacherScreenShow)
				{
					ShowWindow(FSHWND, SW_MINIMIZE);
					ShowWindow(FSHWND, SW_HIDE);
					HWND temp = ::FindWindow(NULL, Title);
					if (temp)
						SendMessage(temp, TDM_CLICK_BUTTON, 100, 0);
						
				}
				else
				{
					ShowWindow(FSHWND, SW_SHOW);
					ShowWindow(FSHWND, SW_SHOWNORMAL);
				}
				isTeacherScreenShow = !isTeacherScreenShow;
				Sleep(100);
			}
			FSHWND = _GetHWND();
			Sleep(100);
		}
		HWND temp = ::FindWindow(NULL, Title);
		if (temp)
			SendMessage(temp, TDM_CLICK_BUTTON, 100, 0);
		isTeacherScreenShow = 0;
	}
}

void ScreenKillerBox(HWND RedHWND, LPCWSTR Title)
{
	InitCommonControls();
	int nButtonPressed = 0;
	TASKDIALOGCONFIG config = { 0 };
	const TASKDIALOG_BUTTON buttons[] = {
		{ IDOK, L"立即隐藏极域黑屏肃静屏幕广播\n您可以稍后再次按下“Ctrl + 空格”恢复" },
		{ 100, L"暂时忽略\n您可以稍后自行按下“Ctrl + 空格”隐藏" }
	};
	config.cbSize = sizeof(config);
	config.hInstance = hInst;
	config.hwndParent = RedHWND;
	config.dwFlags = TDF_USE_COMMAND_LINKS;
	config.dwCommonButtons = 0;
	config.pszMainIcon = TD_WARNING_ICON;
	config.pszWindowTitle = Title;
	config.pszMainInstruction = L"隐藏极域黑屏肃静屏幕广播";
	config.pszContent = L"您可以现在立即隐藏，或稍后按下键盘“Ctrl + 空格”隐藏";
	config.pButtons = buttons;
	config.pszVerificationText = L"不再显示该信息";
	config.cButtons = ARRAYSIZE(buttons);
	TaskDialogIndirect(&config, &nButtonPressed, NULL, &isTeacherScreenBox);
	isTeacherScreenBox = !isTeacherScreenBox;
	switch (nButtonPressed)
	{
	case IDOK:
		ShowWindow(RedHWND, SW_HIDE);
		isTeacherScreenShow = 0;
		break;
	case 100:
		break;
	default:
		break;
	}
}

string GetCompileInfo()
{
	string CompileInfo;
	const char* month_names[] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
	string timetemp;
	timetemp = __DATE__;
	if (timetemp[4] == ' ')
		timetemp[4] = '0';
	string year = timetemp.substr(9, 2);
	string day = timetemp.substr(4, 2);
	char month[5];
	string s_month = timetemp.substr(0, 3);
	int tmonth;
	for (int i = 0; i <= 11; i++)
		if (s_month == month_names[i])
		{
			tmonth = i + 1;
			break;
		}
	if (tmonth < 10)
		month[0] = '0', month[1] = tmonth + '0', month[2] = '\0';
	else
		month[0] = '1', month[1] = tmonth % 10 + '0', month[2] = '\0';
	timetemp = __TIME__;
	string hour = timetemp.substr(0, 2);
	string min = timetemp.substr(3, 2);
#ifdef _BETA
	CompileInfo = (string)"版本 " + _BETA + " Build " + PROGRAM_BUILD_S + ".coral_internal_beta." + year + month + day + "-" + hour + min;
#else
	CompileInfo = "版本 14.0 Build " + (string)PROGRAM_BUILD_S + ".v14_release." + year + month + day + "-" + hour + min;
#endif
	return CompileInfo;
}