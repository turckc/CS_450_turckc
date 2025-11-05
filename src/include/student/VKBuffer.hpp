#pragma once
#include <iostream>
#include <string>
#include "student/VKSetup.hpp"
#include "student/VKCommand.hpp"

using namespace std;

namespace student {
    VmaAllocationCreateInfo createVMAHostVisibleInfo();
    VmaAllocationCreateInfo createVMADeviceLocalInfo();

    struct VulkanBuffer {
        vk::Buffer buffer = nullptr;
        VmaAllocation allocation{};
        vk::DeviceSize size{};
        vk::BufferUsageFlags usage{};
        void* mapped = nullptr;
    };
    
    VulkanBuffer createVulkanBuffer(VulkanInitData &vkInitData,
                                    vk::DeviceSize size,
                                    vk::BufferUsageFlags usage,
                                    VmaAllocationCreateInfo vmaInfo);
    void cleanupVulkanBuffer(VulkanInitData &vkInitData,
                            VulkanBuffer &bufferData);
    void copyToHostVisibleVulkanBuffer(VulkanInitData &vkInitData,
                                        VulkanBuffer &bufferData,
                                        void *hostData);

    struct VulkanStagingData {
        vk::CommandBuffer commandBuffer {};
        vector<VulkanBuffer> allTempBuffers {};
    };

    VulkanStagingData beginStagingVulkanBufferCopies(VulkanInitData &vkInitData,
                                                    vk::CommandPool &commandPool);
    
    void copyToDeviceLocalVulkanBuffer( VulkanInitData &vkInitData,
                                        VulkanStagingData &stagingData,
                                        VulkanBuffer &bufferData,
                                        void *hostData);
    
    void endStagingVulkanBufferCopies( VulkanInitData &vkInitData,
                                        vk::CommandPool &commandPool,
                                        VulkanStagingData &stagingData);
}