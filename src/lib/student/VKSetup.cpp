#include "student/VKSetup.hpp"

namespace student {

    bool createVulkanSwapchain(VulkanInitData &vkInitData) {
        vkb::SwapchainBuilder swapchainBuilder { vkInitData.bootDevice };
        VkSurfaceFormatKHR format;
        format.format = VK_FORMAT_B8G8R8A8_UNORM;
        format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        auto swapRet = swapchainBuilder.set_desired_format(format).build();
        if(!swapRet) {
            cerr << "ERROR: " << swapRet.error().message() << endl;
            return false;
        }
    vkb ::Swapchain vkbSwapchain = swapRet. value();
    vkInitData.swapchain.chain = vk::SwapchainKHR { vkbSwapchain.swapchain };
    vkInitData.swapchain.extent = vk::Extent2D { vkbSwapchain.extent };
    vkInitData.swapchain.format = vk::Format(vkbSwapchain.image_format);
    
    vector<VkImage> vkImages = vkbSwapchain.get_images().value();
    vector<VkImageView> vkView = vkbSwapchain.get_image_views().value();
    for (int i = 0; i < vkImages.size(); i++) {
        vkInitData.swapchain.images.push_back(vk::Image { vkImages.at(i) });
        vkInitData.swapchain.views.push_back(vk::ImageView { vkView.at(i) });
    }
    return true;

    }

    void recreateVulkanSwapchain(VulkanInitData &vkInitData) {
        vkInitData.device.waitIdle();
        cleanupVulkanSwapchain(vkInitData);
        createVulkanSwapchain(vkInitData);
    }

    void cleanupVulkanSwapchain(VulkanInitData &vkInitData) {
        for (int i = 0; i < vkInitData.swapchain.views.size(); i++) {
            vkInitData.device.destroyImageView(vkInitData.swapchain.views.at(i));
        }
        vkInitData.swapchain.views.clear();
        vkInitData.swapchain.images.clear();
        vkInitData.swapchain.extent = vk::Extent2D ();
        vkInitData.swapchain.format = vk::Format {};
        vkInitData.device.destroySwapchainKHR(vkInitData.swapchain.chain);
        }

    bool createVulkanSetup(VulkanInitData &vkInitData) {
        vkb::InstanceBuilder builder;
        auto instRet = builder.set_app_name(vkInitData.appName.c_str())
                                .set_engine_name("Forge Eninge")
                                .request_validation_layers()
                                .use_default_debug_messenger()
                                .require_api_version(
                                    vkInitData.minVersionMajor,
                                    vkInitData.minVersionMinor,
                                    0
                                ).build();
    if (!instRet) {
        cerr << "ERROR: " << instRet.error().message() << endl;
        return false;
    }

    vkb::Instance vkbInstance = instRet.value();
    vkInitData.bootInstance = instRet.value();
    vkInitData.instance = vk::Instance { vkInitData.bootInstance.instance };

    VkSurfaceKHR surface = nullptr;
    VkResult surfErr = glfwCreateWindowSurface(
        vkbInstance.instance, vkInitData.window, NULL, &surface);
    if(surfErr != VK_SUCCESS) {
        cerr << "Error: " << surfErr << endl;
        return false;
    }
    vkInitData.surface = vk::SurfaceKHR { surface };

    vk::PhysicalDeviceFeatures reqFeatures {};
    reqFeatures.samplerAnisotropy = true;
    vk::PhysicalDeviceVulkan13Features reqFeatures13;
    reqFeatures13.dynamicRendering = true;
    reqFeatures13.synchronization2 = true;
    vkb::PhysicalDeviceSelector selector { vkbInstance };
    selector.set_surface(surface);
    selector.set_minimum_version(vkInitData.minVersionMajor,
        vkInitData.minVersionMinor);
    selector.set_required_features(reqFeatures);
    selector.set_required_features_13(reqFeatures13);
    auto physRet = selector.select();
    if(!physRet) {
        cerr << "Error: " << physRet.error().message() << endl;
        return false;
    }
    vkb::PhysicalDevice vkbPhysicalDevice = physRet.value();
    vkInitData.physicalDevice;
    vkb::DeviceBuilder deviceBuilder { vkbPhysicalDevice };
    auto devRet = deviceBuilder.build();
    if(!devRet) {
        cerr << "Error: " << devRet.error().message() << endl;
        return false;
    }
    vkb::Device vkbDevice = devRet.value();
    vkInitData.device = vk::Device { vkbDevice.device };
    vkInitData.bootDevice = vkbDevice;
    return true;

    }

    void cleanupVulkanSetup(VulkanInitData &vkInitData) {
        vkInitData.device.destroy();
        vkInitData.instance.destroySurfaceKHR(vkInitData.surface);
        vkb::destroy_instance(vkInitData.bootInstance);
    }


    GLFWwindow* createGLFWWindow(   string windowName,
                                    int windowWidth, int windowHeight,
                                    bool isWindowResizable) {
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, isWindowResizable);
            GLFWwindow *window = glfwCreateWindow(
                windowWidth, windowHeight,
                windowName.c_str(), nullptr, nullptr);
            return window;
}

void cleanupGLFWWindow(GLFWwindow *window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}
}

