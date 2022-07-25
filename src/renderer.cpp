#include "renderer.hpp"

#define CHECK_NULL(expr) \
if(!(expr))\
{\
    throw std::runtime_error(#expr "is nullptr!");\
}

vk::Instance Renderer::instance_ = nullptr;
vk::SurfaceKHR Renderer::surface_ = nullptr;
vk::PhysicalDevice Renderer::phyDevice_ = nullptr;
Renderer::QueueFamilyIndices Renderer::queueIndices_;
vk::Device Renderer::device_ = nullptr;
vk::Queue Renderer::graphicQueue_ = nullptr;
vk::Queue Renderer::presentQueue_ = nullptr;

void Renderer::Init(SDL_Window* window)
{
    //get extensions
    unsigned int count;
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
    std::vector<const char*> extensions(count);
    SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data());
    
    // std::cout << "Enabled Extensions" << std::endl;
    // for(auto& ext : extensions)
    // {
    //     std::cout << "\t" << ext << std::endl;
    // }

    instance_ = createInstance(extensions);
    CHECK_NULL(instance_);
    
    surface_ = createSurface(window);
    CHECK_NULL(surface_);

    phyDevice_ = pickupPhysicalDevice();
    CHECK_NULL(phyDevice_);

    std::cout << "Pickup Device Name " << phyDevice_.getProperties().deviceName << std::endl;

    queueIndices_ = queuePhysicalDevice();

    device_ = createDevice();
    CHECK_NULL(device_);

    graphicQueue_ = device_.getQueue(queueIndices_.graphicsIndices.value(), 0);
    presentQueue_ = device_.getQueue(queueIndices_.presentIndices.value(), 0);
    CHECK_NULL(graphicQueue_);
    CHECK_NULL(presentQueue_);
}

vk::Instance Renderer::createInstance(const std::vector<const char*> extensions)
{
    //validation layers
    std::array<const char*, 1> layers{"VK_LAYER_KHRONOS_validation"};

    //get all layer names if you forget how to spell them
    // auto layerNames = vk::enumerateInstanceLayerProperties();
    // for(auto& layer : layerNames)
    //     std::cout << layer.layerName << std::endl;

    vk::InstanceCreateInfo info;
    info.setPEnabledExtensionNames(extensions);
    info.setPEnabledLayerNames(layers);

    return vk::createInstance(info);
}

vk::SurfaceKHR Renderer::createSurface(SDL_Window* window)
{
    VkSurfaceKHR surface;
    if(!SDL_Vulkan_CreateSurface(window, instance_, &surface))
    {
        throw std::runtime_error("surface creat failed");
    }
    return surface;
}

Renderer::QueueFamilyIndices Renderer::queuePhysicalDevice()
{
    Renderer::QueueFamilyIndices indices;
    auto families = phyDevice_.getQueueFamilyProperties();
    uint32_t idx = 0;
    for(auto family : families)
    {
        if(family.queueFlags | vk::QueueFlagBits::eGraphics)
        {
            indices.graphicsIndices = idx;
        }
        if(phyDevice_.getSurfaceSupportKHR(idx, surface_))
        {
            indices.presentIndices = idx;
        }
        if(indices.graphicsIndices && indices.presentIndices) break;
        idx ++;
    }
    return indices;
}

vk::Device Renderer::createDevice()
{
    std::vector<vk::DeviceQueueCreateInfo> queueinfos;

    if(queueIndices_.graphicsIndices.value() == queueIndices_.presentIndices.value())
    {
        vk::DeviceQueueCreateInfo info;
        float priority = 1.0;
        info.setQueuePriorities(priority);
        info.setQueueFamilyIndex(queueIndices_.graphicsIndices.value());
        info.setQueueCount(1);
        queueinfos.push_back(info);
    }
    else
    {
        vk::DeviceQueueCreateInfo info1;
        float priority = 1.0;
        info1.setQueuePriorities(priority);
        info1.setQueueFamilyIndex(queueIndices_.graphicsIndices.value());
        info1.setQueueCount(1);

        vk::DeviceQueueCreateInfo info2;
        info2.setQueuePriorities(priority);
        info2.setQueueFamilyIndex(queueIndices_.presentIndices.value());
        info2.setQueueCount(1);

        queueinfos.push_back(info1);
        queueinfos.push_back(info2);
    }
    
    vk::DeviceCreateInfo info;
    info.setQueueCreateInfos(queueinfos);

    return phyDevice_.createDevice(info);
}

vk::PhysicalDevice Renderer::pickupPhysicalDevice()
{
    auto physicalDevices = instance_.enumeratePhysicalDevices();
    // for(auto& device : physicalDevices)     //query the property and features of all GPU FYI
    // {
    //     auto property = device.getProperties();
    //     auto features = device.getFeatures();
    //     std::cout << "Device Name " << property.deviceName << std::endl;
    // }
    return physicalDevices[0];
}

void Renderer::Quit()
{
    device_.destroy();
    instance_.destroySurfaceKHR(surface_);
    instance_.destroy();
}