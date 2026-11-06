#pragma once

#include "Platform.h"

// Notification-area icon with a Start/Stop/Exit context menu.
class TrayIcon
{
public:
	struct Handlers
	{
		std::function<bool()> isRunning;
		std::function<void()> onToggle;
		std::function<void()> onExit;
	};

	explicit TrayIcon(Handlers handlers);
	~TrayIcon();

	TrayIcon(const TrayIcon &) = delete;
	TrayIcon & operator=(const TrayIcon &) = delete;

	bool Create();
	void Destroy();

	LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	bool AddIcon();
	void ShowContextMenu();

	Handlers m_handlers;
	HWND m_window = nullptr;
	UINT m_taskbarCreatedMessage = 0;
};
