#include <chrono>
#include <memory>
#include <random>
#include <sstream>
#include <thread>

#include "BackgroundWindowManager.h"
#include "LandscapeGenerator.h"
#include "Renderer.h"

using namespace std;

CRenderer::CRenderer()
{
}

bool CRenderer::Process(int x, int y, int width, int height)
{
	auto now = chrono::system_clock::now();
	if (m_lastTime + chrono::seconds(5) > now && width == m_lastWidth && height == m_lastHeight)
	{
		if (x != m_lastX || y != m_lastY) // Monitor needs an update but we don't need to regenerate anything
		{
			m_lastX = x;
			m_lastY = y;

			return true;
		}

		return false;
	}

	m_lastTime = now;
	m_lastX = x;
	m_lastY = y;
	m_lastWidth = width;
	m_lastHeight = height;

	vector<pair<Coord<int>, Coord<int>>> coordSets = { { { 0, static_cast<int>(0.7 * height) }, { width, static_cast<int>(0.64 * height) } },
		                                               { { 0, static_cast<int>(0.54 * height) }, { width, static_cast<int>(0.38 * height) } },
		                                               { { 0, static_cast<int>(0.36 * height) }, { width, static_cast<int>(0.16 * height) } },
		                                               { { static_cast<int>(0.25 * width), static_cast<int>(0) }, { width, static_cast<int>(0.4 * height) } } };

	for (auto & coordSet : coordSets)
	{
		coordSet.first.y += (m_generator() % 200) - 100;
		coordSet.second.y += (m_generator() % 200) - 100;
	}

	vector<Layer> layers;
	layers.push_back(midpointDisplacement(coordSets[0].first, coordSets[0].second, 1.2, 540, 8));
	layers.push_back(midpointDisplacement(coordSets[1].first, coordSets[1].second, 1, 259, 9));
	layers.push_back(midpointDisplacement(coordSets[2].first, coordSets[2].second, 1.2, 65, 12));
	layers.push_back(midpointDisplacement(coordSets[3].first, coordSets[3].second, 1.4, 43, 12));

	m_landscape = drawLayers(layers, width, height, "");

	return true;
}

void CRenderer::Render(HDC hDC, int x, int y, int width, int height)
{
	if (hDC == nullptr || m_landscape == nullptr || width != m_lastWidth || height != m_lastHeight)
		return;

	HBITMAP hBitmap = CreateBitmap(width, height, 1, 32, m_landscape.get());

	HDC hdcMem = CreateCompatibleDC(hDC);
	HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);
	BitBlt(hDC, x, y, width, height, hdcMem, 0, 0, SRCCOPY);
	SelectObject(hdcMem, oldBitmap);
	DeleteDC(hdcMem);

	DeleteObject(hBitmap);
}
