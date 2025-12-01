#include "student/VKSetup.hpp"
#include <iostream>
#include <string> 
#include "student/VKCommand.hpp"
#include "student/VKImage.hpp"
#include "student/VKPipeline.hpp"
#include "student/VKMesh.hpp"

using namespace std;
using namespace student;

struct ForgeVertex {
        glm::vec3 pos;
        glm::vec4 color;
    };

struct UniformPush {
    alignas(16) glm::mat4 modelMat;
};

glm::mat4 modelMat(1.0);
string transformString = "v";

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

void printRM(string name, glm::mat3 &M) {
    cout << name << ":" << endl;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
                cout << M[j][i] << ", ";
        }
        cout << endl;
    }
}

void printRM(string name, glm::mat4 &M) {
    cout << name << ":" << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
                cout << M[j][i] << ", ";
        }
        cout << endl;
    }
}

static void key_callback(GLFWwindow* window,
                            int key, int scancode,
                            int action, int mods) {
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
        if(key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, 1);
        }
        else if(key == GLFW_KEY_Q) {
            modelMat = glm::rotate(glm::radians(5.0f), glm::vec3(0,0,1))*modelMat;
            transformString = "R(S)*" + transformString;
        }
        else if(key == GLFW_KEY_E) {
            modelMat = glm::rotate(glm::radians(-5.0f), glm::vec3(0,0,1))*modelMat;
            transformString = "R(-S)*" + transformString;
        }
        else if(key == GLFW_KEY_F) {
            modelMat = glm::scale(glm::vec3(0.8f, 1.0f, 1.0f))*modelMat;
            transformString = "S(0.8, 1.0)*" + transformString;
        }
        else if(key == GLFW_KEY_G) {
            modelMat = glm::scale(glm::vec3(1.25f, 1.0f, 1.0f))*modelMat;
            transformString = "S(1.25, 1.0)*" + transformString;
        }
        else if(key == GLFW_KEY_R) {
            modelMat = glm::scale(glm::vec3(1.0f, 0.8f, 1.0f))*modelMat;
            transformString = "S(1.0, 0.8)*" + transformString;
        }
        else if(key == GLFW_KEY_T) {
            modelMat = glm::scale(glm::vec3(1.0f, 1.25f, 1.0f))*modelMat;
            transformString = "S(1.0, 1.25)*" + transformString;
        }
        else if(key == GLFW_KEY_W) {
            modelMat = glm::translate(glm::vec3(0.0f, 0.1f, 0.0f))*modelMat;
            transformString = "T(0.0, 0.1)*" + transformString;
        }
        else if(key == GLFW_KEY_S) {
            modelMat = glm::translate(glm::vec3(0.0f, -0.1f, 0.0f))*modelMat;
            transformString = "T(0.0, -0.1)*" + transformString;
        }
        else if(key == GLFW_KEY_A) {
            modelMat = glm::translate(glm::vec3(-0.1f, 0.0f, 0.0f))*modelMat;
            transformString = "T(-0.1, 0.0)*" + transformString;
        }
        else if(key == GLFW_KEY_D) {
            modelMat = glm::translate(glm::vec3(0.1f, 0.0f, 0.0f))*modelMat;
            transformString = "T(0.1, 0.0)*" + transformString;
        }
        else if(key == GLFW_KEY_SPACE) {
            modelMat = glm::mat4(1.0);
            transformString = "v";
        }

        cout << transformString << endl;
    }
}


void recordCommands( VulkanInitData &vkInitData,
                        uint32_t indexFIF,
                        uint32_t indexSwap,
                        vk::CommandBuffer &commandBuffer,
                        vk::QueryPool &queryPool,
                        VulkanPipelineData &pipelineData,
                        VulkanMesh &mesh) {
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
                .setClearValue(vk::ClearColorValue {0.0f, 0.0f, 0.5f, 1.0f});
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

        UniformPush pc {};
        pc.modelMat = modelMat;

        commandBuffer.pushConstants(
            pipelineData.pipelineLayout,
            vk::ShaderStageFlagBits::eVertex,
            0, sizeof(UniformPush),
            &pc
        );

        recordDrawVulkanMesh(commandBuffer, mesh);

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

    glm::mat3 M = glm::mat3(1, 2, 3, 4, 5, 6, 7, 8, 9);
    printRM("M", M);

    //Changed structuring of appName and windowTitle name
    string appName = "Exercises10";
    string windowTitle = appName + ": turckc";
    int windowWidth = 640;
    int windowHeight = 480;

    GLFWwindow* window = createGLFWWindow(
        windowTitle, windowWidth, windowHeight);

    glfwSetKeyCallback(window, key_callback);

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

    modelMat[0][0] = 0.5f;

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

    pipelineCreateInfo.attribDesc.push_back(vk::VertexInputAttributeDescription(
        0, // location
        0, // binding
        vk::Format::eR32G32B32A32Sfloat, // format
        offsetof(ForgeVertex, color) // offset
    )); 

    vk::PipelineRenderingCreateInfo renderInfo{};
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachmentFormats = &(vkInitData.swapchain.format);
    renderInfo.depthAttachmentFormat = vk::Format::eD32Sfloat;
    pipelineCreateInfo.renderInfo = renderInfo;

    pipelineCreateInfo.pushConstantRanges.push_back(
        {vk::ShaderStageFlagBits::eVertex, 0, sizeof(UniformPush)});

    VulkanPipelineData pipelineData
        = createBasicVulkanPipeline(vkInitData, pipelineCreateInfo);

    HostMesh<ForgeVertex> hostMesh {};
    hostMesh.vertices = {
        {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}}
    };
    hostMesh.indices = { 0, 2, 1, 2, 0, 3 };

    bool useStaging = true;
    // Start staging copies
    VulkanStagingData stagingData {};
    if(useStaging) {
        stagingData = beginStagingVulkanBufferCopies(vkInitData,
                                                    commandData.commandPool);
    }
    // Create Vulkan Mesh
    VulkanMesh mesh = createVulkanMesh(vkInitData, hostMesh, useStaging);
    // Copy data
    copyToVulkanMesh(vkInitData, mesh, hostMesh, useStaging, &stagingData);
    // End staging
    if(useStaging) {
        endStagingVulkanBufferCopies( vkInitData, commandData.commandPool,
                                        stagingData);
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        uint32_t indexFIF = framesRendered % commandData.numberFramesInFlight;
        uint32_t indexSwap = prepareFrameInFlight(vkInitData, commandData,
                                                    indexFIF);

        recordCommands(vkInitData, indexFIF, indexSwap,
                        commandData.perFIF[indexFIF].commandBuffer,
                        queryPools.at(indexFIF),
                        pipelineData,
                        mesh);

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

    cleanupVulkanMesh(vkInitData, mesh);

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
