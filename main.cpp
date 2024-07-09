#include <map>
#include <memory>
#include <thread>

#include "BackgroundWindowManager.hpp"
#include "Renderer.h"

using namespace std;

HWND BackgroundWindowManager::s_workerw = nullptr;

static map<HMONITOR, RECT> s_monitors;

BOOL EnumMonitorsCallback(HMONITOR hMonitor, HDC hdc, LPRECT rect, LPARAM lparam)
{
	if (hMonitor != nullptr)
		s_monitors[hMonitor] = *rect;

	return TRUE;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int argc = 0;
	LPWSTR * argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	//auto argv = CommandLineToArgv(lpCmdLine, &argc);
	if (argv == nullptr)
		return 1;

	int monitorID = stoi(wstring(argv[1]));

	unique_ptr<BackgroundWindowManager> windowManager(new BackgroundWindowManager());

	HDC hdc = GetDC(windowManager->GetHandle());
	BOOL result = EnumDisplayMonitors(hdc, nullptr, EnumMonitorsCallback, 0);
	ReleaseDC(windowManager->GetHandle(), hdc);

	if (!result || s_monitors.empty())
		return 1;

	auto iter = s_monitors.cbegin();
	for (int i = 0; i < monitorID && iter != s_monitors.cend(); ++i)
		++iter;

	if (iter == s_monitors.cend())
		iter = s_monitors.cbegin();

	RECT rect = iter->second;

	RenderLoop(windowManager, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);

	return 0;
}
