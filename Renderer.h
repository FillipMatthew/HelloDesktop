#pragma once

class BackgroundWindowManager;

void RenderLoop(std::unique_ptr<BackgroundWindowManager> & windowManager, int x, int y, int width, int height);
