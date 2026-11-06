#pragma once

#include "Platform.h"

class BackgroundWindowManager
{
public:
	BackgroundWindowManager();
	~BackgroundWindowManager();

	HWND GetHandle();

private:
	HWND m_progMan = nullptr;
	static HWND s_workerw;
};
