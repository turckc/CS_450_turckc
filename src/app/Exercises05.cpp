#include "student/VKSetup.hpp"
#include <iostream>
#include <string> 
#include "student/VKCommand.hpp"
#include "student/VKImage.hpp"

using namespace std;
using namespace student;

const char* getDeviceTypeString(vk::PhysicalDeviceType t) {
    switch (t) {
        case vk::PhysicalDeviceType::eIntegratedGpu: return "Integrated GPU";
        case vk::PhysicalDeviceType::eDiscreteGpu: return "Discrete GPU";
        case vk::PhysicalDeviceType::eVirtualGpu: return "Virtual GPU";
        case vk::PhysicalDeviceType::eCpu: return "CPU";
        default: return "Other";
    }
}

void printPhysicalDeviceProperties(vk::PhysicalDevice &pd) {
    vk::PhysicalDeviceProperties props = pd.getProperties();
    uint32_t ver = props.apiVersion;

    cout << "Name: " << props.deviceName.data() << endl;
    cout << "Type: " << getDeviceTypeString(props.deviceType) << endl;
    cout << "API Version: "
                    << VK_VERSION_MAJOR(ver) << "."
                    << VK_VERSION_MINOR(ver) << "."
                    << VK_VERSION_PATCH(ver) << endl;
}

void listAvailablePhysicalDevices(VulkanInitData &vkInitData) {
    vector<vk::PhysicalDevice> phys
        = vkInitData.instance.enumeratePhysicalDevices();
    cout << "Found " << phys.size() << " physical device(s):" << endl;
    for (int i = 0; i < phys.size(); i++) {
        cout << "** Device " << i << " ***********" << endl;
        printPhysicalDeviceProperties(phys[i]);
    }
}

int main(int argc, char **argv) {
    cout << "BEGIN VULKAN EXERCISE" << endl;

    //Changed structuring of appName and windowTitle name
    string appName = "Assign01";
    string windowTitle = appName + ": turckc";
    int windowWidth = 640;
    int windowHeight = 480;

    GLFWwindow* window = createGLFWWindow(
        windowTitle, windowWidth, windowHeight);

    if (!window) {
        glfwTerminate();
        exit(1);
    }

    VulkanInitData vkInitData {};
    vkInitData.appName = appName;
    vkInitData.window = window;
    if(!createVulkanSetup(vkInitData)) {
        exit(1);
    }

    listAvailablePhysicalDevices(vkInitData);

    void recordCommands( VulkanInitData &vkInitData,
                        uint32_t indexFIF,
                        uint32_t indexSwap,
                        vk::CommandBuffer &commandBuffer,
                        vk::QueryPool &queryPool) {
        commandBuffer.begin(vk::CommandBufferBeginInfo());
        commandBuffer.resetQueryPool(queryPool, 0, 2);
        commandBuffer.writeTimestamp2(
            vk::PipelineStageFlagBits2::eTopOfPipe, queryPool, 0);

        VulkanImageTransition colorBarrier = createVulkanImageTransition(
                vkInitData.swapchain.images[indexSwap],
                VK_IMAGE_TRANSITION_TYPE::UNDEF_TO_COLOR);
        performVulkanImageTransition(commandBuffer, colorBarrier);
        
        // Render commands here
        VulkanImageTransition presentBarrier = createVulkanImageTransition(
            vkInitData.swapchain.images[indexSwap],
            VK_IMAGE_TRANSITION_TYPE::COLOR_TO_PRESENT);
        performVulkanImageTransition(commandBuffer, presentBarrier);

        commandBuffer.writeTimestamp2(
        vk::PipelineStageFlagBits2::eBottomOfPipe, queryPool, 1);
        commandBuffer.end();
    }

    uint32_t apiVersion = vk::enumerateInstanceVersion();
    cout << "My loader supports Vulkan "
            << VK_VERSION_MAJOR(apiVersion) << "."
            << VK_VERSION_MINOR(apiVersion) << "."
            << VK_VERSION_PATCH(apiVersion) << endl;

    cout << "** Chosen Physical Device: *********" << endl;
    printPhysicalDeviceProperties(vkInitData.physicalDevice);

    // Not sure if this is in the proper place
    int numberFramesInFlight = 2;
    VulkanCommandData commandData = createVulkanCommandData(vkInitData,
                                            numberFramesInFlight);
    uint64_t framesRendered = 0;

    vk::QueryPoolCreateInfo qpCI{};
    qpCI.queryType = vk::QueryType::eTimestamp;
    qpCI.queryCount = 2;
    vector<vk::QueryPool> queryPools = {};
    for(int i = 0; i < commandData.numberFramesInFlight; i++) {
    queryPools.push_back(vkInitData.device.createQueryPool(qpCI));
    }


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        uint32_t indexFIF = framesRendered % commandData.numberFramesInFlight;
        uint32_t indexSwap = prepareFrameInFlight(vkInitData, commandData,
                                                    indexFIF);

        recordCommands(vkInitData, indexFIF, indexSwap,
                        commandData.perFIF[indexFIF].commandBuffer,
                        queryPools.at(indexFIF));

        // TODO SUBMIT ACTUAL COMMANDS
        submitToGraphicsQueue(vkInitData, commandData, indexFIF, indexSwap);
        
        bool successPresent = presentSwapImage( vkInitData, commandData,
                                                    indexFIF, indexSwap);
        if(!successPresent) {
            recreateVulkanSwapchain(vkInitData);
        }

        uint64_t timestamps[2] = {};
        vkInitData.device.getQueryPoolResults(
            queryPools.at(indexFIF), 0, 2,
            sizeof(timestamps), timestamps,
            sizeof(uint64_t),
            vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait
        );

        // Convert ticks to nanoseconds
        auto props = vkInitData.physicalDevice.getProperties();
        double nsPerTick = props.limits.timestampPeriod;
        double deltaNs = (timestamps[1] - timestamps[0]) * nsPerTick;
        
        cout << "TIME for FIF " << indexFIF << ": " << deltaNs << endl;

        framesRendered++;
    }

    vkInitData.device.waitIdle();
    for(int i = 0; i < queryPools.size(); i++) {
        vkInitData.device.destroyQueryPool(queryPools.at(i));
    }
    queryPools.clear();
    
    cleanupVulkanCommandData(vkInitData, commandData);
    //}        
    // TODO
    // Cleanup
    cleanupVulkanSetup(vkInitData);
    cleanupGLFWWindow(window);
    return 0;
}
