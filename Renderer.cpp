#include <memory>
#include <sstream>
#include <thread>

#include "BackgroundWindowManager.hpp"
#include "LandscapeGenerator.h"
#include "Renderer.h"

using namespace std;

void RenderLoop(unique_ptr<BackgroundWindowManager> & windowManager, int x, int y, int width, int height)
{
	mt19937 generator(chrono::system_clock::now().time_since_epoch().count());
	for (int i = 0; i < 5; ++i)
	{
		vector<pair<Coord<int>, Coord<int>>> coordSets
		    = { { { 0, static_cast<int>(0.7 * height) }, { width, static_cast<int>(0.64 * height) } },
			    { { 0, static_cast<int>(0.54 * height) }, { width, static_cast<int>(0.38 * height) } },
			    { { 0, static_cast<int>(0.36 * height) }, { width, static_cast<int>(0.16 * height) } },
			    { { static_cast<int>(0.25 * width), static_cast<int>(0) }, { width, static_cast<int>(0.4 * height) } } };

		for (auto & coordSet : coordSets)
		{
			coordSet.first.y += (generator() % 200) - 100;
			coordSet.second.y += (generator() % 200) - 100;
		}

		vector<Layer> layers;
		layers.push_back(midpointDisplacement(coordSets[0].first, coordSets[0].second, 1.2, 540, 8));
		layers.push_back(midpointDisplacement(coordSets[1].first, coordSets[1].second, 1, 259, 9));
		layers.push_back(midpointDisplacement(coordSets[2].first, coordSets[2].second, 1.2, 65, 12));
		layers.push_back(midpointDisplacement(coordSets[3].first, coordSets[3].second, 1.4, 43, 12));

		auto landscape = drawLayers(layers, width, height, "");

		HBITMAP hBitmap = CreateBitmap(width, height, 1, 32, landscape.get());

		HDC hdc = GetDC(windowManager->GetHandle());
		if (hdc != nullptr)
		{
			HDC hdcMem = CreateCompatibleDC(hdc);
			HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);
			BitBlt(hdc, x, y, width, height, hdcMem, 0, 0, SRCCOPY);
			SelectObject(hdcMem, oldBitmap);
			DeleteDC(hdcMem);
		}

		ReleaseDC(windowManager->GetHandle(), hdc);
		this_thread::sleep_for(5000ms);
		DeleteObject(hBitmap);
	}
}
