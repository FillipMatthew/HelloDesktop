// Platform headers must precede `import std;` (GCC modules)
#include "Platform.h"

import std;

#include "Log.h"
#include "TrayIcon.h"

namespace
{
constexpr const char * WINDOW_CLASS_NAME = "HelloDesktopTray";
constexpr const char * TOOLTIP = "Hello Desktop";
constexpr UINT WM_TRAY_ICON = WM_APP + 1;
constexpr UINT TRAY_ICON_ID = 1;
constexpr UINT MENU_ID_TOGGLE = 1;
constexpr UINT MENU_ID_EXIT = 2;

LRESULT CALLBACK TrayWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (WM_NCCREATE == msg)
	{
		CREATESTRUCT * create = reinterpret_cast<CREATESTRUCT *>(lParam);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
	}

	TrayIcon * self = reinterpret_cast<TrayIcon *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (nullptr != self)
		return self->HandleMessage(hwnd, msg, wParam, lParam);

	return DefWindowProc(hwnd, msg, wParam, lParam);
}
} // namespace

TrayIcon::TrayIcon(Handlers handlers)
    : m_handlers(std::move(handlers))
{
}

TrayIcon::~TrayIcon()
{
	Destroy();
}

bool TrayIcon::Create()
{
	HINSTANCE instance = GetModuleHandle(nullptr);

	WNDCLASS wc = { };
	wc.lpfnWndProc = TrayWindowProc;
	wc.hInstance = instance;
	wc.lpszClassName = WINDOW_CLASS_NAME;
	if (0 == RegisterClass(&wc) && ERROR_CLASS_ALREADY_EXISTS != GetLastError())
	{
		Log("Tray RegisterClass failed: " + std::to_string(GetLastError()));
		return false;
	}

	m_taskbarCreatedMessage = RegisterWindowMessage("TaskbarCreated");

	// Hidden top-level window (not message-only) so it receives TaskbarCreated.
	m_window = CreateWindowEx(0, WINDOW_CLASS_NAME, TOOLTIP, WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, instance, this);
	if (nullptr == m_window)
	{
		Log("Tray CreateWindowEx failed: " + std::to_string(GetLastError()));
		return false;
	}

	return AddIcon();
}

void TrayIcon::Destroy()
{
	if (nullptr == m_window)
		return;

	NOTIFYICONDATA nid = { };
	nid.cbSize = sizeof(nid);
	nid.hWnd = m_window;
	nid.uID = TRAY_ICON_ID;
	Shell_NotifyIcon(NIM_DELETE, &nid);

	DestroyWindow(m_window);
	m_window = nullptr;
	UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(nullptr));
}

bool TrayIcon::AddIcon()
{
	NOTIFYICONDATA nid = { };
	nid.cbSize = sizeof(nid);
	nid.hWnd = m_window;
	nid.uID = TRAY_ICON_ID;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_TRAY_ICON;
	nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	lstrcpyn(nid.szTip, TOOLTIP, sizeof(nid.szTip) / sizeof(nid.szTip[0]));

	if (FALSE == Shell_NotifyIcon(NIM_ADD, &nid))
	{
		Log("Shell_NotifyIcon(NIM_ADD) failed");
		return false;
	}

	return true;
}

void TrayIcon::ShowContextMenu()
{
	HMENU menu = CreatePopupMenu();
	if (nullptr == menu)
		return;

	const bool running = m_handlers.isRunning && m_handlers.isRunning();
	AppendMenu(menu, MF_STRING, MENU_ID_TOGGLE, running ? "Stop" : "Start");
	AppendMenu(menu, MF_SEPARATOR, 0, nullptr);
	AppendMenu(menu, MF_STRING, MENU_ID_EXIT, "Exit");

	POINT cursor = { };
	GetCursorPos(&cursor);

	// Required so the menu dismisses when the user clicks elsewhere.
	SetForegroundWindow(m_window);
	const UINT command = static_cast<UINT>(TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, cursor.x, cursor.y, 0, m_window, nullptr));
	PostMessage(m_window, WM_NULL, 0, 0);
	DestroyMenu(menu);

	if (MENU_ID_TOGGLE == command && m_handlers.onToggle)
		m_handlers.onToggle();
	else if (MENU_ID_EXIT == command && m_handlers.onExit)
		m_handlers.onExit();
}

LRESULT TrayIcon::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (WM_TRAY_ICON == msg)
	{
		const UINT mouseMessage = static_cast<UINT>(lParam);
		if (WM_RBUTTONUP == mouseMessage)
			ShowContextMenu();
		else if (WM_LBUTTONDBLCLK == mouseMessage && m_handlers.onToggle)
			m_handlers.onToggle();

		return 0;
	}

	// Explorer restarted: the icon must be re-added.
	if (0 != m_taskbarCreatedMessage && m_taskbarCreatedMessage == msg)
	{
		AddIcon();
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}
