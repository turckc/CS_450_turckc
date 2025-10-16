#include "student/VKSetup.hpp"
#include <iostream>
#include <string> 
#include "student/VKCommand.hpp"
#include "student/VKImage.hpp"
#include "student/VKPipeline.hpp"

using namespace std;
using namespace student;

float red = 0.0f;
float redInc = 0.0001f;

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

void recordCommands( VulkanInitData &vkInitData,
                        uint32_t indexFIF,
                        uint32_t indexSwap,
                        vk::CommandBuffer &commandBuffer,
                        vk::QueryPool &queryPool,
                        VulkanPipelineData &pipelineData) {
        commandBuffer.begin(vk::CommandBufferBeginInfo());
        commandBuffer.resetQueryPool(queryPool, 0, 2);
        commandBuffer.writeTimestamp2(
            vk::PipelineStageFlagBits2::eTopOfPipe, queryPool, 0);

        VulkanImageTransition colorBarrier = createVulkanImageTransition(
                vkInitData.swapchain.images[indexSwap],
                VK_IMAGE_TRANSITION_TYPE::UNDEF_TO_COLOR);
        performVulkanImageTransition(commandBuffer, colorBarrier);
        
        red += redInc;
        if(red > 1.0f) {
            redInc = -redInc;
        }
        if(red < 0.0f) {
            redInc = -redInc;
        }

        vk::RenderingAttachmentInfoKHR colorAtt{};
        colorAtt.setImageView(vkInitData.swapchain.views[indexSwap])
                .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setClearValue(vk::ClearColorValue {red, 0.0f, 0.0f, 1.0f});
        vk::RenderingInfoKHR ri{};
        ri.setRenderArea(vk::Rect2D{ {0,0}, vkInitData.swapchain.extent })
            .setLayerCount(1)
            .setColorAttachments(colorAtt);

        commandBuffer.beginRendering(ri);

        commandBuffer.bindPipeline(
            vk::PipelineBindPoint::eGraphics,
            pipelineData.graphicsPipeline);

        vk::Viewport viewports[] = {
            {0, 0,
            (float)vkInitData.swapchain.extent.width,
            (float)vkInitData.swapchain.extent.height,
            0.0f, 1.0f}};
        commandBuffer.setViewport(0, viewports);
        
        vk::Rect2D scissors[]
        = {{{0,0}, vkInitData.swapchain.extent}};
        commandBuffer.setScissor(0, scissors);

        commandBuffer.endRendering();

        // Render commands here
        VulkanImageTransition presentBarrier = createVulkanImageTransition(
            vkInitData.swapchain.images[indexSwap],
            VK_IMAGE_TRANSITION_TYPE::COLOR_TO_PRESENT);
        performVulkanImageTransition(commandBuffer, presentBarrier);

        commandBuffer.writeTimestamp2(
        vk::PipelineStageFlagBits2::eBottomOfPipe, queryPool, 1);
        commandBuffer.end();
    }

int main(int argc, char **argv) {
    cout << "BEGIN VULKAN EXERCISE" << endl;

    //Changed structuring of appName and windowTitle name
    string appName = "Exercises06";
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

    struct ForgeVertex {
        glm::vec3 pos;
    };


    VulkanPipelineCreationInfo pipelineCreateInfo{};
    pipelineCreateInfo.vertSPVFilename
        = "build/compiledshaders/" + appName + "/shader.vert.spv";
    pipelineCreateInfo.fragSPVFilename
        = "build/compiledshaders/" + appName + "/shader.frag.spv";

    pipelineCreateInfo.bindDesc = vk::VertexInputBindingDescription(
        0, sizeof(ForgeVertex), vk::VertexInputRate::eVertex);
    
    // POSITION
    pipelineCreateInfo.attribDesc.push_back(vk::VertexInputAttributeDescription(
        0, // location
        0, // binding
        vk::Format::eR32G32B32Sfloat, // format
        offsetof(ForgeVertex, pos) // offset
    ));

    vk::PipelineRenderingCreateInfo renderInfo{};
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachmentFormats = &(vkInitData.swapchain.format);
    renderInfo.depthAttachmentFormat = vk::Format::eD32Sfloat;
    pipelineCreateInfo.renderInfo = renderInfo;


    VulkanPipelineData pipelineData
        = createBasicVulkanPipeline(vkInitData, pipelineCreateInfo);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        uint32_t indexFIF = framesRendered % commandData.numberFramesInFlight;
        uint32_t indexSwap = prepareFrameInFlight(vkInitData, commandData,
                                                    indexFIF);

        recordCommands(vkInitData, indexFIF, indexSwap,
                        commandData.perFIF[indexFIF].commandBuffer,
                        queryPools.at(indexFIF),
                        pipelineData);

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

    cleanupVulkanPipeline(vkInitData, pipelineData);

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
