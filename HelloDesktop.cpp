#include <map>
#include <memory>
#include <random>
#include <string>
#include <thread>

#include "App.h"
#include "BackgroundWindowManager.h"
#include "Renderer.h"

using namespace std;

static map<HMONITOR, RECT> s_monitors;

BOOL EnumMonitorsCallback(HMONITOR hMonitor, HDC hdc, LPRECT rect, LPARAM lparam)
{
	if (hMonitor != nullptr)
		s_monitors[hMonitor] = *rect;

	return TRUE;
}

class CHelloDesktop : public IApp
{
public:
	CHelloDesktop()
	{
	}

	string GetName() const override
	{
		return "Hello Desktop";
	}

	bool Initialise() override
	{
		m_windowManager.reset(new BackgroundWindowManager());
		m_renderer.reset(new CRenderer());

		return true;
	}

	void Uninitialise() override
	{
		m_bRunning.store(false, memory_order_relaxed);
	}

	void Run() override
	{
		//MSG messageInfo = { 0 };

		while (m_bRunning.load(memory_order_relaxed))
		{
			bool bMustRender = ProcessRenderers();

			/*
			if (::PeekMessage(&messageInfo,
			                  0, // any window
			                  0, // all messages
			                  0, // all messages
			                  PM_REMOVE))
			{
				if (messageInfo.message == WM_QUIT)
				{
					break; // WM_QUIT retrieved so stop looping
				}

				::DispatchMessage(&messageInfo);

				continue;
			}
			*/

			if (bMustRender)
			{
				HDC hDC = GetDC(m_windowManager->GetHandle());
				Render(hDC);
				ReleaseDC(m_windowManager->GetHandle(), hDC);
			}

			this_thread::sleep_for(chrono::milliseconds(1));
		}

		// Return the WM_QUIT exit code.
		//return static_cast<int>(messageInfo.wParam);
	}

protected:
	bool ProcessRenderers()
	{
		HDC hDC = GetDC(m_windowManager->GetHandle());
		if (hDC == nullptr)
			return false;

		if (!GetMonitors(hDC))
			return false;

		bool bMustRender = false;
		int i = 0;
		for (auto iter = s_monitors.cbegin(); iter != s_monitors.cend(); ++iter, ++i)
		{
			if (i == 0) // Currently just render main window
			{
				RECT rect = iter->second;
				if (m_renderer->Process(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top))
					bMustRender = true;
			}
		}

		return bMustRender;
	}

	void Render(HDC hDC)
	{
		if (hDC == nullptr)
			return;

		if (!GetMonitors(hDC))
			return;

		int i = 0;
		for (auto iter = s_monitors.cbegin(); iter != s_monitors.cend(); ++iter, ++i)
		{
			if (i == 0) // Currently just render main window
			{
				RECT rect = iter->second;
				m_renderer->Render(hDC, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
			}
		}
	}

	bool GetMonitors(HDC hDC)
	{
		if (hDC == nullptr)
			return false;

		s_monitors.clear();
		BOOL result = EnumDisplayMonitors(hDC, nullptr, EnumMonitorsCallback, 0);

		if (!result || s_monitors.empty())
			return false;

		return true;
	}

private:
	atomic<bool> m_bRunning = true;
	unique_ptr<BackgroundWindowManager> m_windowManager;
	unique_ptr<CRenderer> m_renderer;
};

REGISTER_SERVICE(CHelloDesktop);
