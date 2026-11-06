// Juan Gallostra's 2D Procedural Landscape generator in C++

#pragma once

import std;

#include "Drawing.h"

using Layer = std::vector<Coord<int>>;

void insertSorted(std::vector<Coord<int>> & vec, const Coord<int> & item)
{
	std::vector<Coord<int>>::const_iterator iter = vec.cbegin();
	while (vec.cend() != iter)
	{
		if (iter->x <= item.x)
			++iter;
		else
			break;
	}

	vec.insert(iter, item);
}

std::vector<Coord<int>> midpointDisplacement(Coord<int> start, Coord<int> end, int roughness, int verticalDisplacement = 0, int num_of_iterations = 16)
{
	if (0 == verticalDisplacement)
		verticalDisplacement = (start.y + end.y) / 2;

	// Create a random number generator with the random seed
	std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());

	std::vector points { start, end };
	for (int iteration = 0; iteration < num_of_iterations; ++iteration)
	{
		std::vector<Coord<int>> points2 { points.cbegin(), points.cend() };
		for (size_t i = 0; i < points2.size() - 1; ++i)
		{
			Coord<int> midPoint = { (points2[i].x + points2[i + 1].x) / 2, (points2[i].y + points2[i + 1].y) / 2 };
			midPoint.y += (0 == (generator() % 2)) ? -verticalDisplacement : verticalDisplacement;
			insertSorted(points, midPoint);
		}

		verticalDisplacement *= std::pow(2, -roughness);
	}

	return points;
}

std::unique_ptr<std::uint8_t[]> drawLayers(const std::vector<Layer> layers, int width, int height, const std::string & colourPaletteKeyword)
{
	std::vector<Colour> colours;
	// if (colourPaletteKeyword)
	// {
	// }

	// Default colour palette
	// if (colours.empty() || colourMap.size() < (layers.size() + 1))
	// {
	colours = { Colour { 224, 157, 195 }, Colour { 204, 98, 158 }, Colour { 138, 79, 130 }, Colour { 99, 28, 68 },
	            Colour { 82, 7, 49 },     Colour { 38, 3, 23 },    Colour { 163, 203, 240 } };
	// }

	if (colours.size() < (layers.size() + 1))
		throw std::runtime_error("Num of colours should be bigger than the num of layers");

	std::vector<Layer> finalLayers;
	for (const Layer & layer : layers)
	{
		Layer sampledLayer;
		for (size_t i = 0; i < (layer.size() - 1); ++i)
		{
			sampledLayer.push_back(layer[i]);
			if (layer[i + 1].x - layer[i].x > 1)
			{
				double m = static_cast<double>(layer[i + 1].y - layer[i].y) / (layer[i + 1].x - layer[i].x);
				double n = layer[i].y - m * layer[i].x;

				for (int j = layer[i].x + 1; j < layer[i + 1].x; ++j)
					sampledLayer.push_back({ j, static_cast<int>(std::round(m * j + n)) });
			}
		}

		finalLayers.push_back(sampledLayer);
	}

	size_t bufferSize = width * height * 4;
	std::unique_ptr<std::uint8_t[]> buffer = std::make_unique<std::uint8_t[]>(bufferSize);
	Colour backgroundColour = colours[colours.size() - 1];
	for (size_t i = 0; i < bufferSize; i += 4)
	{
		std::uint8_t * pixel = buffer.get() + i;
		// GDI 32-bit bitmaps are BGRA.
		pixel[0] = backgroundColour.b;
		pixel[1] = backgroundColour.g;
		pixel[2] = backgroundColour.r;
		pixel[3] = 255;
	}

	drawCircle(buffer.get(), width, 75, 75, 50, Colour { 255, 255, 255 });

	for (size_t i = 0; i < finalLayers.size(); ++i)
	{
		const Layer & layer = finalLayers[i];
		for (size_t j = 0; j < layer.size(); ++j)
		{
			Coord<int> coord = layer[j];
			if (0 > coord.y)
				coord.y = 0;

			if (coord.y >= height)
				coord.y = height - 1;

			drawLine(buffer.get(), width, coord.x, (height - 1) - coord.y, coord.x, height - 1, colours[i]);
		}
	}

	return buffer;
}
