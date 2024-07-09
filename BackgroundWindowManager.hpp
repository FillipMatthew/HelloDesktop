#pragma once

#include <Windows.h>

class BackgroundWindowManager
{
public:
	BackgroundWindowManager()
	{
		m_progMan = FindWindow("Progman", "Program Manager");
		SendMessage(m_progMan, 0x0034, 4, 0);
		PDWORD_PTR result = nullptr;
		SendMessageTimeout(m_progMan, 0x052C, 0, 0, SMTO_NORMAL, 1000, result);

		// We enumerate all Windows, until we find one, that has the
		// SHELLDLL_DefView as a child. If we found that window, we take its next
		// sibling and assign it to workerw.
		EnumWindows(
		    [](HWND tophandle, LPARAM topparamhandle)
		    {
			    HWND p = FindWindowEx(tophandle, nullptr, "SHELLDLL_DefView", nullptr);

			    if (p != nullptr)
			    {
				    // Gets the WorkerW Window after the current one.
				    s_workerw = FindWindowEx(nullptr, tophandle, "WorkerW", nullptr);
			    }

			    return TRUE;
		    },
		    0);
	}

	~BackgroundWindowManager()
	{
		DestroyWindow(s_workerw); // Does this even do anything?
		s_workerw = nullptr;
		SendMessage(m_progMan, 0x0034, 4, 0);
	}

	HWND GetHandle()
	{
		return s_workerw;
	}

private:
	HWND m_progMan = nullptr;
	static HWND s_workerw;
};
