#pragma once

#include "vulkan/vulkan.hpp"
#include "SDL.h"
#include "SDL_vulkan.h"

//std
#include <stdexcept>
#include <vector>
#include <iostream>
#include <array>
class Renderer final
{
public:
    static void Init(SDL_Window* window);
    static void Quit();
private:
    static vk::Instance instance_;
    static vk::SurfaceKHR surface_;
};