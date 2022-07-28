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
vk::SwapchainKHR Renderer::swapchain_ = nullptr;
Renderer::SwapchainRequiredInfo Renderer::requiredInfo_;
std::vector<vk::Image> Renderer::images_;
std::vector<vk::ImageView> Renderer::imageViews_;
vk::Pipeline Renderer::pipeline_ = nullptr;
std::vector<vk::ShaderModule> Renderer::shaderModules_;
vk::PipelineLayout Renderer::layout_ = nullptr;
vk::RenderPass Renderer::renderPass_ = nullptr;
std::vector<vk::Framebuffer> Renderer::framebuffers_;


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

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    requiredInfo_ = querySwapchainRequiredInfo(w, h);

    swapchain_ = createSwapchain();
    CHECK_NULL(swapchain_);

    images_ = device_.getSwapchainImagesKHR(swapchain_);
    imageViews_ = createImageViews();

    layout_ = createLayout();
    CHECK_NULL(layout_);

    renderPass_ = createRenderPass();
    CHECK_NULL(renderPass_);

    framebuffers_ = createFramebuffers();
    for(auto& framebuffer : framebuffers_)
    {
        CHECK_NULL(framebuffer);
    }
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

    std::array<const char*, 1> extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    
    vk::DeviceCreateInfo info;
    info.setPEnabledExtensionNames(extensions);
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

vk::SwapchainKHR Renderer::createSwapchain()
{
    vk::SwapchainCreateInfoKHR info;
    info.setImageColorSpace(requiredInfo_.format.colorSpace);
    info.setImageFormat(requiredInfo_.format.format);
    info.setMinImageCount(requiredInfo_.imageCount);
    info.setImageExtent(requiredInfo_.extent);
    info.setPresentMode(requiredInfo_.presentMode);
    info.setPreTransform(requiredInfo_.capabilities.currentTransform);
    
    if(queueIndices_.graphicsIndices.value() == queueIndices_.presentIndices.value())
    {
        info.setQueueFamilyIndices(queueIndices_.graphicsIndices.value());
        info.setImageSharingMode(vk::SharingMode::eExclusive);
    }
    else
    {
        std::array<uint32_t, 2> indices{queueIndices_.graphicsIndices.value(),
                                        queueIndices_.presentIndices.value()};
        info.setQueueFamilyIndices(indices);
        info.setImageSharingMode(vk::SharingMode::eConcurrent);
    }

    info.setClipped(true);
    info.setSurface(surface_);
    info.setImageArrayLayers(1);
    info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

    return device_.createSwapchainKHR(info);
    

}

Renderer::SwapchainRequiredInfo Renderer::querySwapchainRequiredInfo(int w, int h)
{
    SwapchainRequiredInfo info;
    info.capabilities = phyDevice_.getSurfaceCapabilitiesKHR(surface_);
    auto formats = phyDevice_.getSurfaceFormatsKHR(surface_);
    info.format = formats[0];
    for(auto& format : formats)
    {
        if(format.format == vk::Format::eR8G8B8A8Srgb || format.format == vk::Format::eB8G8R8A8Srgb)
        {
            info.format = format;
        }
    }

    info.extent.width = std::clamp<uint32_t>(w, info.capabilities.minImageExtent.width, info.capabilities.maxImageExtent.width);
    info.extent.height = std::clamp<uint32_t>(h, info.capabilities.minImageExtent.height, info.capabilities.maxImageExtent.height);

    info.imageCount = std::clamp<uint32_t>(2, info.capabilities.minImageCount, info.capabilities.maxImageCount);

    auto presentModes = phyDevice_.getSurfacePresentModesKHR(surface_);
    info.presentMode = vk::PresentModeKHR::eFifo;
    for(auto& present : presentModes)
    {
        if(present == vk::PresentModeKHR::eMailbox)
        {
            info.presentMode = present;
        }
    }

    return info;
}

std::vector<vk::ImageView> Renderer::createImageViews()
{
    std::vector<vk::ImageView> views(images_.size());
    for(int i = 0; i < views.size(); i ++)
    {
        vk::ImageViewCreateInfo info;
        info.setImage(images_[i]);
        info.setFormat(requiredInfo_.format.format);
        info.setViewType(vk::ImageViewType::e2D);
        vk::ImageSubresourceRange range;
        range.setBaseMipLevel(0);
        range.setLevelCount(1);
        range.setLayerCount(1);
        range.setBaseArrayLayer(0);
        range.setAspectMask(vk::ImageAspectFlagBits::eColor);
        info.setSubresourceRange(range);
        vk::ComponentMapping mapping;
        info.setComponents(mapping);

        views[i] = device_.createImageView(info);
    }
    return views;
}

void Renderer::Quit()
{
    for(auto& framebuffer : framebuffers_)
    {
        device_.destroyFramebuffer(framebuffer);
    }
    device_.destroyRenderPass(renderPass_);
    device_.destroyPipelineLayout(layout_);
    device_.destroyPipeline(pipeline_);
    for(auto& shader : shaderModules_)
    {
        device_.destroyShaderModule(shader);
    }
    for(auto& view : imageViews_)
    {
        device_.destroyImageView(view);
    }
    device_.destroySwapchainKHR(swapchain_);
    device_.destroy();
    instance_.destroySurfaceKHR(surface_);
    instance_.destroy();
}

void Renderer::CreatePipeline(vk::ShaderModule vertexShader, vk::ShaderModule fragShader)
{
    vk::GraphicsPipelineCreateInfo info;

    //Shader configurations
    std::array<vk::PipelineShaderStageCreateInfo, 2> stageInfos;
    stageInfos[0].setModule(vertexShader)
                 .setStage(vk::ShaderStageFlagBits::eVertex)
                 .setPName("main");
    stageInfos[1].setModule(fragShader)
                 .setStage(vk::ShaderStageFlagBits::eFragment)
                 .setPName("main");
    info.setStages(stageInfos);
    
    //Vertex Input
    vk::PipelineVertexInputStateCreateInfo vertexInput;
    info.setPVertexInputState(&vertexInput);

    //Input Assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAsm;
    inputAsm.setPrimitiveRestartEnable(false)
            .setTopology(vk::PrimitiveTopology::eTriangleList);
    info.setPInputAssemblyState(&inputAsm);

    //layout
    info.setLayout(layout_);

    //Viewport and Scissor
    vk::PipelineViewportStateCreateInfo viewportState;
    vk::Viewport viewport(0, 0, requiredInfo_.extent.width, requiredInfo_.extent.height, 0.0f, 1.0f);
    
    vk::Rect2D scissor({0, 0}, requiredInfo_.extent);
    viewportState.setViewports(viewport)
                 .setScissors(scissor);
    
    info.setPViewportState(&viewportState);

    //Rasterization
    vk::PipelineRasterizationStateCreateInfo rastInfo;
    rastInfo.setRasterizerDiscardEnable(false)
            .setDepthClampEnable(false)
            .setDepthBiasClamp(false)
            .setLineWidth(1)
            .setCullMode(vk::CullModeFlagBits::eNone)
            .setPolygonMode(vk::PolygonMode::eFill);
    
    info.setPRasterizationState(&rastInfo);

    //Multisample
    vk::PipelineMultisampleStateCreateInfo multisample;
    multisample.setSampleShadingEnable(false)
               .setRasterizationSamples(vk::SampleCountFlagBits::e1);
    info.setPMultisampleState(&multisample);

    //DepthStencil
    info.setPDepthStencilState(nullptr);

    //Color Blend
    vk::PipelineColorBlendStateCreateInfo colorBlend;
    vk::PipelineColorBlendAttachmentState attBlendState;
    attBlendState.setColorWriteMask(vk::ColorComponentFlagBits::eR |
                                    vk::ColorComponentFlagBits::eG |
                                    vk::ColorComponentFlagBits::eB |
                                    vk::ColorComponentFlagBits::eA);
    colorBlend.setLogicOpEnable(false)
              .setAttachments(attBlendState);
    
    info.setPColorBlendState(&colorBlend);

    //RenderPass
    info.setRenderPass(renderPass_);

    auto result = device_.createGraphicsPipeline(nullptr, info);
    if(result.result != vk::Result::eSuccess)
    {
        throw std::runtime_error("pipeline create failed");
    }

    pipeline_ = result.value; 

}

vk::ShaderModule Renderer::CreateShaderModule(const char* filename)
{
    std::ifstream file(filename, std::ios::binary|std::ios::in);
    std::vector<char> content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
    
    file.close();

    vk::ShaderModuleCreateInfo info;
    info.pCode = (uint32_t*)content.data();
    info.codeSize = content.size();

    shaderModules_.push_back(device_.createShaderModule(info));
    return shaderModules_.back();
}

vk::PipelineLayout Renderer::createLayout()
{
    vk::PipelineLayoutCreateInfo info;
    return device_.createPipelineLayout(info);
}

vk::RenderPass Renderer::createRenderPass()
{
    vk::RenderPassCreateInfo createInfo;
    vk::AttachmentDescription attachmentDesc;
    attachmentDesc.setSamples(vk::SampleCountFlagBits::e1)
                  .setLoadOp(vk::AttachmentLoadOp::eClear)
                  .setStoreOp(vk::AttachmentStoreOp::eStore)
                  .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                  .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                  .setFormat(requiredInfo_.format.format)
                  .setInitialLayout(vk::ImageLayout::eUndefined)
                  .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
    createInfo.setAttachments(attachmentDesc);

    vk::SubpassDescription subpassDesc;
    vk::AttachmentReference refer;
    refer.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
         .setAttachment(0);
    subpassDesc.setColorAttachments(refer);
    subpassDesc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

    createInfo.setSubpasses(subpassDesc);

    return device_.createRenderPass(createInfo);

}

std::vector<vk::Framebuffer> Renderer::createFramebuffers()
{
    std::vector<vk::Framebuffer> result;

    for(int i = 0; i < imageViews_.size(); i ++)
    {
        vk::FramebufferCreateInfo info;
        info.setRenderPass(renderPass_);
        info.setLayers(1);
        info.setWidth(requiredInfo_.extent.width);
        info.setHeight(requiredInfo_.extent.height);
        info.setAttachments(imageViews_[i]);
        result.push_back(device_.createFramebuffer(info));
    }
    return result;
}