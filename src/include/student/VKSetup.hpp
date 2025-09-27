#pragma once
#include <iostream>
#include <string>
#include "VKCore.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "vk_mem_alloc.h"

using namespace std;

namespace student {
    struct VulkanSwapChain {
        vk::SwapchainKHR chain {};
        vector<vk::Image> images {};
        vector<vk::ImageView> views {};
        vk::Extent2D extent {};
        vk::Format format {};
    };


    struct VulkanInitData {
        string appName = "";
        int minVersionMajor = 1;
        int minVersionMinor = 3;

        vkb::Instance bootInstance {};
        vk::Instance instance {};
        GLFWwindow *window = nullptr;

        vk::SurfaceKHR surface {};

        vk::PhysicalDevice physicalDevice {};

        vkb::Device bootDevice {};
        vk::Device device {};

        VulkanSwapChain swapchain {};
    };

    bool createVulkanSwapchain(VulkanInitData &vkInitData);
    void recreateVulkanSwapchain(VulkanInitData &vkInitData);
    void cleanupVulkanSwapchain(VulkanInitData &vkInitData);

    bool createVulkanSetup(VulkanInitData &vkInitData);
    void cleanupVulkanSetup(VulkanInitData &vkInitData);

GLFWwindow* createGLFWWindow(string windowName, int windowWidth,
        int windowHeight, bool isWindowResizable = true);
void cleanupGLFWWindow(GLFWwindow *window);
}
