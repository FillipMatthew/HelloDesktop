#pragma once

class CRenderer
{
public:
	CRenderer();

	bool Process(const int x, const int y, const int width, const int height);
	void Render(HDC hDC, const int x, const int y, const int width, const int height);

private:
	std::mt19937 m_generator { static_cast<std::mt19937::result_type>(std::chrono::system_clock::now().time_since_epoch().count()) };
	std::chrono::system_clock::time_point m_lastTime;
	std::unique_ptr<std::uint8_t[]> m_landscape;
	int m_lastX = 0;
	int m_lastY = 0;
	int m_lastWidth = 0;
	int m_lastHeight = 0;
};
