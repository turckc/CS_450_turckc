#pragma once
#include <iostream>
#include <string>
#include "VKFile.hpp"
#include "student/VKSetup.hpp"

using namespace std;

namespace student {
    vk::ShaderModule createVulkanShaderModule(
                        VulkanInitData &vkInitData,
                        const vector<char>& code);
    void cleanupVulkanShaderModule(
                        VulkanInitData &vkInitData,
                        vk::ShaderModule &shaderModule);

    struct VulkanPipelineCreationInfo {
        string vertSPVFilename {};
        string fragSPVFilename {};

        vk::VertexInputBindingDescription bindDesc{};
        vector<vk::VertexInputAttributeDescription> attribDesc{};

        vk::PipelineRenderingCreateInfo renderInfo {};
    };

    struct VulkanPipelineData {
        vk::PipelineCache cache;
        vk::PipelineLayout pipelineLayout;
        vk::Pipeline graphicsPipeline;
        // TODO
    };

    VulkanPipelineData createBasicVulkanPipeline(
                        VulkanInitData &vkInitData,
                        VulkanPipelineCreationInfo &creationInfo);

    void cleanupVulkanPipeline( VulkanInitData &vkInitData,
                                VulkanPipelineData &pipelineData);


}
