#include "student/VKPipeline.hpp"

namespace student {
    vk::ShaderModule createVulkanShaderModule(VulkanInitData &vkInitData,
                                                const vector<char>& code) {
        return vkInitData.device.createShaderModule(vk::ShaderModuleCreateInfo(
            vk::ShaderModuleCreateFlags(), code.size(),
            reinterpret_cast<const uint32_t*>(code.data())
            // Cast that pretends as if it were a uint32_t pointer
        ));
    }

    void cleanupVulkanShaderModule(VulkanInitData &vkInitData,
                                        vk::ShaderModule &shaderModule) {
        vkInitData.device.destroyShaderModule(shaderModule);
    }

    VulkanPipelineData createBasicVulkanPipeline(VulkanInitData &vkInitData,
                                            VulkanPipelineCreationInfo &creationInfo) {
        VulkanPipelineData data {};
        
        auto vertShaderCode = readBinaryFile(creationInfo.vertSPVFilename);
        auto fragShaderCode = readBinaryFile(creationInfo.fragSPVFilename);
        vk::ShaderModule vertShaderModule = createVulkanShaderModule(vkInitData,
                                                                            vertShaderCode);
        vk::ShaderModule fragShaderModule = createVulkanShaderModule(vkInitData,
                                                                            fragShaderCode);
        
        vk::PipelineShaderStageCreateInfo vertShaderStageInfo(
            {}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main");
        vk::PipelineShaderStageCreateInfo fragShaderStageInfo(
            {}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main");
        
        vk::PipelineShaderStageCreateInfo shaderStages[] = {
            vertShaderStageInfo,
            fragShaderStageInfo};

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
            {}, creationInfo.bindDesc, creationInfo.attribDesc);

        cleanupVulkanShaderModule(vkInitData, fragShaderModule);
        cleanupVulkanShaderModule(vkInitData, vertShaderModule);
        
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
            {}, vk::PrimitiveTopology::eTriangleList, false);

        vk::Viewport viewport(0, 0,
                                (float)vkInitData.swapchain.extent.width,
                                (float)vkInitData.swapchain.extent.height,
                                0.0f, 1.0f);

        vk::Rect2D scissor({0,0}, vkInitData.swapchain.extent);

        vk::PipelineViewportStateCreateInfo viewportState(
                                                {}, viewport, scissor);

        vector<vk::DynamicState> dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };
        
        vk::PipelineDynamicStateCreateInfo dynamicState(
            {}, dynamicStates);

        vk::PipelineRasterizationStateCreateInfo rasterizer {};
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = vk::CullModeFlagBits::eNone;
        // eBack for back face culling
        rasterizer.frontFace = vk::FrontFace::eCounterClockwise;

        vk::PipelineColorBlendAttachmentState colorBlendAttachment {};
        colorBlendAttachment.colorWriteMask =
            vk::ColorComponentFlagBits::eR
            | vk::ColorComponentFlagBits::eG
            | vk::ColorComponentFlagBits::eB
            | vk::ColorComponentFlagBits::eA;

        vk::PipelineColorBlendStateCreateInfo colorBlending(
            {}, false, vk::LogicOp::eCopy, colorBlendAttachment);

        vk::PipelineDepthStencilStateCreateInfo depthStencil(
            {},
            true, // Enable depth testing
            true, // Enable depth writing
            vk::CompareOp::eLess, // Lower depth = closer = keep
            false, // Not putting bounds on depth test
            false, {}, {} // Not using stencil test
        );

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, {}, {});
        data.pipelineLayout
            = vkInitData.device.createPipelineLayout(pipelineLayoutInfo);

        vk::PipelineMultisampleStateCreateInfo multisample(
            {}, vk::SampleCountFlagBits::e1);

        data.cache =
        vkInitData.device.createPipelineCache(vk::PipelineCacheCreateInfo());

        vk::GraphicsPipelineCreateInfo pinfo {};
        pinfo.setFlags(vk::PipelineCreateFlags());
        pinfo.setStages(shaderStages);
        pinfo.setPVertexInputState(&vertexInputInfo);
        pinfo.setPInputAssemblyState(&inputAssembly);
        pinfo.setPViewportState(&viewportState);
        pinfo.setPRasterizationState(&rasterizer);
        pinfo.setPMultisampleState(&multisample);
        pinfo.setPDepthStencilState(&depthStencil);
        pinfo.setPColorBlendState(&colorBlending);
        pinfo.setPDynamicState(&dynamicState);
        pinfo.setLayout(data.pipelineLayout);
        pinfo.setPNext(&(creationInfo.renderInfo));
        pinfo.setRenderPass(nullptr);
        auto ret
        = vkInitData.device.createGraphicsPipeline(data.cache, pinfo);

        if (ret.result != vk::Result::eSuccess) {
        throw runtime_error("Failed to create graphics pipeline!");
        }
        // Set pipeline
        data.graphicsPipeline = ret.value;

        // Return data
        return data;
    }

    void cleanupVulkanPipeline(VulkanInitData &vkInitData,
                                VulkanPipelineData &pipelineData) {
        // TODO
        vkInitData.device.destroyPipelineCache(pipelineData.cache);
        vkInitData.device.destroyPipelineLayout(pipelineData.pipelineLayout);
        vkInitData.device.destroyPipeline(pipelineData.graphicsPipeline);
}

}
