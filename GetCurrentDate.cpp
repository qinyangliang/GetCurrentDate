// DateToClipboard.cpp
// 功能：按全局热键（如Ctrl+Shift+D）自动将当前日期复制到剪贴板
// 编译：Visual Studio 创建空项目，添加此文件，编译为EXE

#include <windows.h>
#include <string>
#include <ctime>
#include <iostream>

// 热键ID
#define HOTKEY_ID 1001

// 当前窗口句柄
HWND g_hWnd = NULL;

// 获取当前日期字符串（格式：YYYYMMDD，你可以修改）
std::wstring GetCurrentDate() {
	time_t now = time(0);
	struct tm tstruct;
	localtime_s(&tstruct, &now);

	wchar_t buf[11]; // YYYY.MM.DD + 终止符
	wcsftime(buf, 11, L"%Y.%m.%d", &tstruct);
	return std::wstring(buf);
}

// 将文本复制到剪贴板
bool CopyToClipboard(const std::wstring& text) {
	if (!OpenClipboard(g_hWnd)) {
		return false;
	}

	// 清空剪贴板
	EmptyClipboard();

	// 分配全局内存
	size_t size = (text.length() + 1) * sizeof(wchar_t);
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
	if (!hGlobal) {
		CloseClipboard();
		return false;
	}

	// 锁定内存并复制数据
	wchar_t* pGlobal = (wchar_t*)GlobalLock(hGlobal);
	wcscpy_s(pGlobal, text.length() + 1, text.c_str());
	GlobalUnlock(hGlobal);

	// 设置剪贴板数据
	if (SetClipboardData(CF_UNICODETEXT, hGlobal) == NULL) {
		GlobalFree(hGlobal);
		CloseClipboard();
		return false;
	}

	CloseClipboard();
	return true;
}

// 窗口过程
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE:
		// 注册热键：Ctrl + Shift + D
		if (!RegisterHotKey(hWnd, HOTKEY_ID, MOD_CONTROL | MOD_SHIFT, 'D')) {
			MessageBox(hWnd, L"热键注册失败！可能被其他程序占用。", L"错误", MB_ICONERROR);
		}
		break;

	case WM_HOTKEY:
		if (wParam == HOTKEY_ID) {
			// 热键触发，获取日期并复制到剪贴板
			std::wstring dateStr = GetCurrentDate();
			if (CopyToClipboard(dateStr)) {
				// 可选：播放声音提示
				MessageBeep(MB_OK);

				// 可选：显示提示（调试用，正式使用可注释掉）
				// std::wstring msg = L"已复制到剪贴板: " + dateStr;
				// MessageBox(hWnd, msg.c_str(), L"成功", MB_OK);
			}
		}
		break;

	case WM_DESTROY:
		UnregisterHotKey(hWnd, HOTKEY_ID);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// 主函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// 注册窗口类
	const wchar_t CLASS_NAME[] = L"DateToClipboardClass";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// 创建隐藏窗口（只需要一个消息泵）
	g_hWnd = CreateWindowEx(
		0, CLASS_NAME, L"日期复制工具", 0,
		0, 0, 0, 0,
		NULL, NULL, hInstance, NULL
		);

	if (!g_hWnd) {
		MessageBox(NULL, L"窗口创建失败", L"错误", MB_ICONERROR);
		return 1;
	}

	// 隐藏窗口（既然不需要界面）
	ShowWindow(g_hWnd, SW_HIDE);

	// 消息循环
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}