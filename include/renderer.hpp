#pragma once

#include "vulkan/vulkan.hpp"
#include "SDL.h"
#include "SDL_vulkan.h"

//std
#include <stdexcept>
#include <vector>
#include <iostream>
#include <array>
#include <optional>
#include <fstream>
class Renderer final
{
public:
    static void Init(SDL_Window* window);
    static void Quit();
    static void CreatePipeline(vk::ShaderModule vertexShader, vk::ShaderModule frag);
    static vk::ShaderModule CreateShaderModule(const char* filename);

private:
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsIndices;
        std::optional<uint32_t> presentIndices;
    };

    struct SwapchainRequiredInfo
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        vk::Extent2D extent;
        vk::SurfaceFormatKHR format;
        vk::PresentModeKHR presentMode;
        uint32_t imageCount;
    };

    static QueueFamilyIndices queueIndices_;
    static SwapchainRequiredInfo requiredInfo_;

    static vk::Instance instance_;
    static vk::SurfaceKHR surface_;
    static vk::PhysicalDevice phyDevice_;
    static vk::Device device_;
    static vk::Queue graphicQueue_;
    static vk::Queue presentQueue_;
    static vk::SwapchainKHR swapchain_;
    static std::vector<vk::Image> images_;
    static std::vector<vk::ImageView> imageViews_;
    static vk::Pipeline pipeline_;
    static std::vector<vk::ShaderModule> shaderModules_;
    static vk::PipelineLayout layout_;
    static vk::RenderPass renderPass_;
    static std::vector<vk::Framebuffer> framebuffers_;

    static vk::Instance createInstance(const std::vector<const char*> extensions);
    static vk::SurfaceKHR createSurface(SDL_Window* window);
    static vk::PhysicalDevice pickupPhysicalDevice();
    static vk::Device createDevice();
    static vk::SwapchainKHR createSwapchain();
    static std::vector<vk::ImageView> createImageViews();
    static vk::PipelineLayout createLayout();
    static vk::RenderPass createRenderPass();
    static std::vector<vk::Framebuffer> createFramebuffers();
    

    static QueueFamilyIndices queuePhysicalDevice();
    static SwapchainRequiredInfo querySwapchainRequiredInfo(int w, int h);
};