#pragma once

// Include after `import std;` and "Platform.h".

// The exe is a GUI-subsystem app (no console). When launched from a console,
// attach to it so Log() output is visible. Redirected stderr is left alone.
inline void AttachParentConsole()
{
	if (INVALID_HANDLE_VALUE != GetStdHandle(STD_ERROR_HANDLE) && nullptr != GetStdHandle(STD_ERROR_HANDLE))
		return;

	if (FALSE == AttachConsole(ATTACH_PARENT_PROCESS))
		return;

	HANDLE console = CreateFile("CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	if (INVALID_HANDLE_VALUE != console)
		SetStdHandle(STD_ERROR_HANDLE, console);
}

inline void Log(const std::string & message)
{
	const std::string line = "[HelloDesktop] " + message + "\n";

	HANDLE output = GetStdHandle(STD_ERROR_HANDLE);
	if (INVALID_HANDLE_VALUE != output && nullptr != output)
	{
		DWORD written = 0;
		WriteFile(output, line.c_str(), static_cast<DWORD>(line.size()), &written, nullptr);
	}

	OutputDebugString(line.c_str());
}
