#include "student/VKBuffer.hpp"

namespace student {
    VmaAllocationCreateInfo createVMAHostVisibleInfo() {
    VmaAllocationCreateInfo vci{};
    vci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                    VMA_ALLOCATION_CREATE_MAPPED_BIT;
    vci.usage = VMA_MEMORY_USAGE_AUTO;
    return vci;
    }

    VmaAllocationCreateInfo createVMADeviceLocalInfo() {
        VmaAllocationCreateInfo vci{};
        vci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        return vci;
    }

    VulkanBuffer createVulkanBuffer(VulkanInitData &vkInitData,
                                    vk::DeviceSize size,
                                    vk::BufferUsageFlags usage,
                                    VmaAllocationCreateInfo vmaInfo) {

        VkBufferCreateInfo bci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bci.size = size;
        bci.usage = static_cast<VkBufferUsageFlags>(usage);
        bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkBuffer rawBuf{};
        VmaAllocation alloc{};
        VmaAllocationInfo ainfo{};
        vmaCreateBuffer(vkInitData.allocator, &bci, &vmaInfo,
                        &rawBuf, &alloc, &ainfo);
        VulkanBuffer out;
        out.size = size;
        out.usage = usage;
        out.buffer = vk::Buffer(rawBuf);
        out.allocation = alloc;
        out.mapped = ainfo.pMappedData;

        return out;
    }

    void cleanupVulkanBuffer(VulkanInitData &vkInitData,
                            VulkanBuffer &bufferData) {
        if(bufferData.buffer) {
            vmaDestroyBuffer(
                vkInitData.allocator,
                static_cast<VkBuffer>(bufferData.buffer),
                bufferData.allocation);
            bufferData = {};
        }
    }

    void copyToHostVisibleVulkanBuffer(VulkanInitData &vkInitData,
                                        VulkanBuffer &bufferData,
                                        void *hostData) {
        memcpy(bufferData.mapped, hostData, bufferData.size);
        vmaFlushAllocation(vkInitData.allocator, bufferData.allocation,
                            0, VK_WHOLE_SIZE);
    }

    VulkanStagingData beginStagingVulkanBufferCopies(
            VulkanInitData &vkInitData,
            vk::CommandPool &commandPool) {

        VulkanStagingData stagingData {};
        stagingData.commandBuffer = createVulkanCommandBuffer(
                                            vkInitData, commandPool);
        
        stagingData.commandBuffer.begin(
            vk::CommandBufferBeginInfo(
                    vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
        
        return stagingData;
    }

    void copyToDeviceLocalVulkanBuffer( VulkanInitData &vkInitData,
                                        VulkanStagingData &stagingData,
                                        VulkanBuffer &bufferData,
                                        void *hostData) {
    // Create host-visible staging buffer (TRANSFER_SRC)
    VulkanBuffer stageData = createVulkanBuffer(vkInitData, bufferData.size,
                                        vk::BufferUsageFlagBits::eTransferSrc,
                                        createVMAHostVisibleInfo());
    // Copy host data into staging buffer
    copyToHostVisibleVulkanBuffer(vkInitData, stageData, hostData);
    // Record copy from staging buffer to device-local buffer
    vk::BufferCopy copyRegion{};
    copyRegion.size = bufferData.size;
    stagingData.commandBuffer.copyBuffer(stageData.buffer, bufferData.buffer,
                                        1, &copyRegion);
    // Add temporary buffer to list (for cleanup later)
    stagingData.allTempBuffers.push_back(stageData);
    }

    void endStagingVulkanBufferCopies( VulkanInitData &vkInitData,
                                        vk::CommandPool &commandPool,
                                        VulkanStagingData &stagingData) {
        // End recording
        stagingData.commandBuffer.end();
        // Submit to queue
        vk::SubmitInfo submitInfo = vk::SubmitInfo().setCommandBuffers(
                                                    stagingData.commandBuffer);
        vkInitData.graphicsQueue.queue.submit(submitInfo);
        vkInitData.graphicsQueue.queue.waitIdle();
        // Clean up command buffer
        vkInitData.device.freeCommandBuffers(commandPool,stagingData.commandBuffer);
        // Destroy temporary buffers
        for(int i = 0; i < stagingData.allTempBuffers.size(); i++) {
            cleanupVulkanBuffer(vkInitData, stagingData.allTempBuffers.at(i));
        }
        stagingData.allTempBuffers.clear();
    }


}