// Platform headers must precede `import std;` (GCC modules)
#include "Platform.h"

import std;

#include "App.h"
#include "BackgroundWindowManager.h"
#include "Log.h"
#include "Renderer.h"
#include "TrayIcon.h"

using namespace std;

static constexpr int IDLE_SLEEP_MS = 1;

struct MonitorInfo
{
	HMONITOR handle = nullptr;
	RECT rect = { };
	bool primary = false;
};

static vector<MonitorInfo> s_monitors;

BOOL EnumMonitorsCallback(HMONITOR hMonitor, HDC hdc, LPRECT rect, LPARAM lparam)
{
	if (nullptr == hMonitor)
		return TRUE;

	MONITORINFO info = { };
	info.cbSize = sizeof(info);
	const bool bPrimary = (TRUE == GetMonitorInfo(hMonitor, &info)) && (0 != (info.dwFlags & MONITORINFOF_PRIMARY));

	s_monitors.push_back(MonitorInfo { hMonitor, *rect, bPrimary });

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
		m_windowManager->SetPaintHandler([this](HDC hDC) { Render(hDC); });

		m_tray.reset(
		    new TrayIcon(TrayIcon::Handlers { [this] { return m_windowManager->IsRunning(); }, [this] { ToggleRenderer(); }, [] { PostQuitMessage(0); } }));
		if (false == m_tray->Create())
			Log("Failed to create tray icon");

		if (false == StartBackground())
			Log("Failed to start background window");

		return true;
	}

	void Uninitialise() override
	{
		m_bRunning.store(false, memory_order_relaxed);
		m_tray.reset();
		m_windowManager.reset();
	}

	void Run() override
	{
		while (m_bRunning.load(memory_order_relaxed))
		{
			if (false == m_windowManager->PumpMessages())
				break;

			if (m_windowManager->IsRunning() && ProcessRenderers())
				m_windowManager->Invalidate();

			this_thread::sleep_for(chrono::milliseconds(IDLE_SLEEP_MS));
		}
	}

protected:
	void ToggleRenderer()
	{
		if (m_windowManager->IsRunning())
			m_windowManager->Stop();
		else if (false == StartBackground())
			Log("Failed to start background window");
	}

	bool StartBackground()
	{
		RECT rect = { };
		if (false == GetMonitors() || false == SelectDefaultMonitor(rect))
			return false;

		return m_windowManager->Start(rect);
	}

	bool ProcessRenderers()
	{
		RECT rect = { };
		if (false == GetMonitors() || false == SelectDefaultMonitor(rect))
			return false;

		// Currently just render the default monitor
		return m_renderer->Process(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
	}

	void Render(HDC hDC)
	{
		if (nullptr == hDC)
			return;

		RECT rect = { };
		if (false == GetMonitors() || false == SelectDefaultMonitor(rect))
			return;

		// The window covers exactly the default monitor, so draw at its origin.
		m_renderer->Render(hDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top);
	}

	// Fetches all monitors into s_monitors.
	bool GetMonitors()
	{
		s_monitors.clear();
		const BOOL result = EnumDisplayMonitors(nullptr, nullptr, EnumMonitorsCallback, 0);

		return (FALSE != result) && (false == s_monitors.empty());
	}

	// Picks the primary monitor, falling back to the first one found.
	bool SelectDefaultMonitor(RECT & rect) const
	{
		if (true == s_monitors.empty())
			return false;

		rect = s_monitors.front().rect;
		for (const MonitorInfo & monitor : s_monitors)
		{
			if (true == monitor.primary)
			{
				rect = monitor.rect;
				break;
			}
		}

		return true;
	}

private:
	atomic<bool> m_bRunning = true;
	unique_ptr<BackgroundWindowManager> m_windowManager;
	unique_ptr<CRenderer> m_renderer;
	unique_ptr<TrayIcon> m_tray;
};

REGISTER_SERVICE(CHelloDesktop);
