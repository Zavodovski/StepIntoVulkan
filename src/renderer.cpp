#include "renderer.hpp"

#define CHECK_NULL(expr) \
if(!(expr))\
{\
    throw std::runtime_error(#expr "is nullptr!");\
}

vk::Instance Renderer::instance_ = nullptr;
vk::SurfaceKHR Renderer::surface_ = nullptr;

void Renderer::Init(SDL_Window* window)
{
    //get extensions
    unsigned int count;
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
    std::vector<const char*> extensions(count);
    SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data());
    
    std::cout << "Enabled Extensions" << std::endl;
    for(auto& ext : extensions)
    {
        std::cout << "\t" << ext << std::endl;
    }

    //validation layers
    std::array<const char*, 1> layers{"VK_LAYER_KHRONOS_validation"};

    //get all layer names if you forget how to spell them
    // auto layerNames = vk::enumerateInstanceLayerProperties();
    // for(auto& layer : layerNames)
    //     std::cout << layer.layerName << std::endl;

    vk::InstanceCreateInfo info;
    info.setPEnabledExtensionNames(extensions);
    info.setPEnabledLayerNames(layers);
    instance_ = vk::createInstance(info);

    CHECK_NULL(instance_);

    VkSurfaceKHR surface;
    if(!SDL_Vulkan_CreateSurface(window, instance_, &surface))
    {
        throw std::runtime_error("surface creat failed");
    }
    surface_ = surface;
    CHECK_NULL(surface_);

    
}

void Renderer::Quit()
{
    instance_.destroySurfaceKHR(surface_);
    instance_.destroy();
}