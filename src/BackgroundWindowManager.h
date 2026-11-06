#pragma once

#include "Platform.h"

// Owns a window parented into the desktop (behind the icons) that the
// renderer draws into. Start/Stop create and destroy that window.
class BackgroundWindowManager
{
public:
	BackgroundWindowManager() = default;
	~BackgroundWindowManager();

	BackgroundWindowManager(const BackgroundWindowManager &) = delete;
	BackgroundWindowManager & operator=(const BackgroundWindowManager &) = delete;

	// screenRect: the display area (screen coordinates) the window covers.
	bool Start(const RECT & screenRect);
	void Stop();
	bool IsRunning() const;

	// Null when not running.
	HWND GetHandle() const;

	// Pumps this thread's messages. Returns false once WM_QUIT is received.
	bool PumpMessages();

	// Called with the paint DC whenever the window must be drawn.
	using PaintHandler = std::function<void(HDC)>;
	void SetPaintHandler(PaintHandler handler);

	// Schedules a repaint.
	void Invalidate();

	void Paint(HDC hdc);

private:
	HWND FindDesktopParent(HWND & insertAfter);
	void LogDesktopChildren() const;
	bool CreateBackgroundWindow(HWND parent, HWND insertAfter, const RECT & screenRect);

	HWND m_progMan = nullptr;
	HWND m_window = nullptr;
	PaintHandler m_paintHandler;
};
