#pragma once

#include <cmath>
#include <cstdint>
#include <ranges>
#include <span>

template <typename T> struct Coord
{
	T x;
	T y;
};

struct Colour
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

void drawLine(uint8_t * buffer, int bufferWidth, int x1, int y1, int x2, int y2, const Colour & colour)
{
	int dx = std::abs(x2 - x1);
	int dy = std::abs(y2 - y1);
	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;
	int err = dx - dy;

	while (true)
	{
		uint8_t * pixelView = &buffer[((y1 * bufferWidth + x1) * 4)];
		pixelView[0] = colour.r;
		pixelView[1] = colour.g;
		pixelView[2] = colour.b;
		pixelView[3] = 255;

		if (x1 == x2 && y1 == y2)
			break;

		int e2 = 2 * err;
		if (e2 > -dy)
		{
			err -= dy;
			x1 += sx;
		}

		if (e2 < dx)
		{
			err += dx;
			y1 += sy;
		}
	}
}

void drawCircle(uint8_t * buffer, int bufferWidth, int centerX, int centerY, int radius, const Colour & colour)
{
	int N = 2 * radius + 1;
	for (int i = 0; i < N; ++i)
	{
		for (int j = 0; j < N; ++j)
		{
			int x = i - radius;
			int y = j - radius;
			if (x * x + y * y <= radius * radius + 1)
			{
				uint8_t * pixelView = buffer + ((centerY + y) * bufferWidth + (centerX + x)) * 4;
				pixelView[0] = colour.r;
				pixelView[1] = colour.g;
				pixelView[2] = colour.b;
				pixelView[3] = 255;
			}
		}
	}
}
