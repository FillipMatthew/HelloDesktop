#pragma once

class CRenderer
{
public:
	CRenderer();

	bool Process(int x, int y, int width, int height);
	void Render(HDC hDC, int x, int y, int width, int height);

private:
	std::mt19937 m_generator { std::chrono::system_clock::now().time_since_epoch().count() };
	std::chrono::system_clock::time_point m_lastTime;
	std::unique_ptr<uint8_t> m_landscape;
	int m_lastX = 0;
	int m_lastY = 0;
	int m_lastWidth = 0;
	int m_lastHeight = 0;
};
