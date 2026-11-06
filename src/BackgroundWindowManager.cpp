// Platform headers must precede `import std;` (GCC modules)
#include "Platform.h"

import std;

#include "BackgroundWindowManager.h"
#include "Log.h"

namespace
{
constexpr UINT MSG_SPAWN_WORKERW = 0x052C;
constexpr WPARAM SPAWN_WORKERW_WPARAM = 0xD;
constexpr LPARAM SPAWN_WORKERW_LPARAM = 1;
constexpr UINT SPAWN_WORKERW_TIMEOUT_MS = 1000;
constexpr int CLASS_NAME_BUFFER_SIZE = 256;
constexpr BYTE OPAQUE_ALPHA = 255;
constexpr const char * WINDOW_CLASS_NAME = "HelloDesktopBackground";

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_ERASEBKGND:
		return 1;
	case WM_NCHITTEST:
		return HTTRANSPARENT;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		BackgroundWindowManager * manager = reinterpret_cast<BackgroundWindowManager *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (nullptr != manager)
			manager->Paint(hdc);

		EndPaint(hwnd, &ps);
		return 0;
	}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

std::string ToString(HWND hwnd)
{
	return std::format("{}", static_cast<const void *>(hwnd));
}
} // namespace

BackgroundWindowManager::~BackgroundWindowManager()
{
	Stop();
	UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(nullptr));
}

bool BackgroundWindowManager::Start(const RECT & screenRect)
{
	if (nullptr != m_window)
		return true;

	m_progMan = FindWindow("Progman", "Program Manager");
	if (nullptr == m_progMan)
	{
		Log("Progman window not found");
		return false;
	}

	DWORD_PTR result = 0;
	SendMessageTimeout(m_progMan, MSG_SPAWN_WORKERW, SPAWN_WORKERW_WPARAM, SPAWN_WORKERW_LPARAM, SMTO_NORMAL, SPAWN_WORKERW_TIMEOUT_MS, &result);

	HWND insertAfter = HWND_TOP;
	HWND parent = FindDesktopParent(insertAfter);
	if (nullptr == parent)
	{
		Log("No desktop parent window found");
		LogDesktopChildren();
		return false;
	}

	if (false == CreateBackgroundWindow(parent, insertAfter, screenRect))
		return false;

	Invalidate();
	return true;
}

void BackgroundWindowManager::Stop()
{
	if (nullptr != m_window)
	{
		DestroyWindow(m_window);
		m_window = nullptr;
	}
}

bool BackgroundWindowManager::IsRunning() const
{
	return nullptr != m_window;
}

HWND BackgroundWindowManager::GetHandle() const
{
	return m_window;
}

bool BackgroundWindowManager::PumpMessages()
{
	MSG msg = { };
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (WM_QUIT == msg.message)
			return false;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

void BackgroundWindowManager::SetPaintHandler(PaintHandler handler)
{
	m_paintHandler = std::move(handler);
}

void BackgroundWindowManager::Invalidate()
{
	if (nullptr != m_window)
		InvalidateRect(m_window, nullptr, FALSE);
}

void BackgroundWindowManager::Paint(HDC hdc)
{
	if (m_paintHandler)
		m_paintHandler(hdc);
}

// Returns the window our background window should be a child of, and sets
// insertAfter to the sibling it must sit directly below in Z-order.
HWND BackgroundWindowManager::FindDesktopParent(HWND & insertAfter)
{
	// Windows 11 24H2+: the icon view and a WorkerW are both children of
	// Progman. Our window must be a child of that WorkerW (parenting it
	// directly to Progman is not composited by DWM, even when layered).
	HWND defView = FindWindowEx(m_progMan, nullptr, "SHELLDLL_DefView", nullptr);
	if (nullptr != defView)
	{
		insertAfter = HWND_TOP;
		return FindWindowEx(m_progMan, nullptr, "WorkerW", nullptr);
	}

	// Legacy layout: find the top-level window holding SHELLDLL_DefView and
	// use the WorkerW that follows it.
	static HWND s_workerw = nullptr;
	s_workerw = nullptr;
	EnumWindows(
	    [](HWND tophandle, LPARAM)
	    {
		    if (nullptr != FindWindowEx(tophandle, nullptr, "SHELLDLL_DefView", nullptr))
			    s_workerw = FindWindowEx(nullptr, tophandle, "WorkerW", nullptr);

		    return TRUE;
	    },
	    0);

	insertAfter = HWND_TOP;
	return s_workerw;
}

void BackgroundWindowManager::LogDesktopChildren() const
{
	char className[CLASS_NAME_BUFFER_SIZE] = { };
	for (HWND child = GetWindow(m_progMan, GW_CHILD); nullptr != child; child = GetWindow(child, GW_HWNDNEXT))
	{
		GetClassName(child, className, CLASS_NAME_BUFFER_SIZE);
		Log("Progman child (top to bottom): " + ToString(child) + " " + className);
	}
}

bool BackgroundWindowManager::CreateBackgroundWindow(HWND parent, HWND insertAfter, const RECT & screenRect)
{
	HINSTANCE instance = GetModuleHandle(nullptr);

	WNDCLASS wc = { };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = instance;
	wc.lpszClassName = WINDOW_CLASS_NAME;
	if (0 == RegisterClass(&wc) && ERROR_CLASS_ALREADY_EXISTS != GetLastError())
	{
		Log("RegisterClass failed: " + std::to_string(GetLastError()));
		return false;
	}

	m_window
	    = CreateWindowEx(WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW, WINDOW_CLASS_NAME, "Hello Desktop", WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, instance, nullptr);
	if (nullptr == m_window)
	{
		Log("CreateWindowEx failed: " + std::to_string(GetLastError()));
		return false;
	}

	SetWindowLongPtr(m_window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	if (nullptr == SetParent(m_window, parent))
		Log("SetParent failed: " + std::to_string(GetLastError()));

	SetWindowLongPtr(m_window, GWL_STYLE, WS_CHILD | WS_VISIBLE);

	// Position relative to the parent's client area, covering only the chosen display.
	POINT parentOrigin = { 0, 0 };
	ClientToScreen(parent, &parentOrigin);
	const int width = screenRect.right - screenRect.left;
	const int height = screenRect.bottom - screenRect.top;
	SetWindowPos(m_window, insertAfter, screenRect.left - parentOrigin.x, screenRect.top - parentOrigin.y, width, height,
	             SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

	// Without WS_EX_LAYERED, DWM does not composite this window when it is a
	// child of the desktop WorkerW (Windows 11 25H2).
	SetWindowLongPtr(m_window, GWL_EXSTYLE, GetWindowLongPtr(m_window, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(m_window, 0, OPAQUE_ALPHA, LWA_ALPHA);

	return true;
}
