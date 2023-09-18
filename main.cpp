#include "LandscapeGenerator.h"

#include <windows.h>

#include <memory>
#include <sstream>
#include <thread>

using namespace std;

class BackgroundWindow
{
public:
	BackgroundWindow()
	{
		m_progMan = FindWindow("Progman", "Program Manager");
		SendMessage(m_progMan, 0x0034, 4, 0);
		PDWORD_PTR result = nullptr;
		SendMessageTimeout(m_progMan, 0x052C, 0, 0, SMTO_NORMAL, 1000, result);

		// We enumerate all Windows, until we find one, that has the SHELLDLL_DefView as a child.
		// If we found that window, we take its next sibling and assign it to workerw.
		EnumWindows([](HWND tophandle, LPARAM topparamhandle)
					{
        HWND p = FindWindowEx(tophandle, nullptr, "SHELLDLL_DefView", nullptr);

        if (p != nullptr)
        {
            // Gets the WorkerW Window after the current one.
            s_workerw = FindWindowEx(nullptr, tophandle, "WorkerW", nullptr);
        }

        return TRUE; },
					0);
	}

	~BackgroundWindow()
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

HWND BackgroundWindow::s_workerw = nullptr;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int width = 1920;
	int height = 1080;

	std::unique_ptr<BackgroundWindow> window(new BackgroundWindow());
	std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
	for (int i = 0; i < 5; ++i)
	{
		std::vector<std::pair<Coord<int>, Coord<int>>> coordSets = {
			{{0, static_cast<int>(0.7 * height)}, {width, static_cast<int>(0.64 * height)}},
			{{0, static_cast<int>(0.54 * height)}, {width, static_cast<int>(0.38 * height)}},
			{{0, static_cast<int>(0.36 * height)}, {width, static_cast<int>(0.16 * height)}},
			{{static_cast<int>(0.25 * width), static_cast<int>(0)}, {width, static_cast<int>(0.4 * height)}}};

		for (auto &coordSet : coordSets)
		{
			coordSet.first.y += (generator() % 200) - 100;
			coordSet.second.y += (generator() % 200) - 100;
		}

		std::vector<Layer> layers;
		layers.push_back(midpointDisplacement(coordSets[0].first, coordSets[0].second, 1.2, 540, 8));
		layers.push_back(midpointDisplacement(coordSets[1].first, coordSets[1].second, 1, 259, 9));
		layers.push_back(midpointDisplacement(coordSets[2].first, coordSets[2].second, 1.2, 65, 12));
		layers.push_back(midpointDisplacement(coordSets[3].first, coordSets[3].second, 1.4, 43, 12));

		auto landscape = drawLayers(layers, width, height, "");

		HBITMAP hBitmap = CreateBitmap(width, height, 1, 32, landscape.get());

		HDC hdc = GetDC(window->GetHandle());
		if (hdc != nullptr)
		{
			HDC hdcMem = CreateCompatibleDC(hdc);
			HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);
			BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);
			SelectObject(hdcMem, oldBitmap);
			DeleteDC(hdcMem);
		}

		ReleaseDC(window->GetHandle(), hdc);
		this_thread::sleep_for(5000ms);
		DeleteObject(hBitmap);
	}

	return 0;
}
