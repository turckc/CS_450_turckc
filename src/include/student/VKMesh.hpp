#pragma once
#include <vector>
#include <cstddef>
#include "student/VKBuffer.hpp"

namespace student {
    template<typename T>
    struct HostMesh {
        vector<T> vertices {};
        vector<unsigned int> indices {};
    };

    struct VulkanMesh {
        VulkanBuffer vertices;
        VulkanBuffer indices;
        unsigned int indexCnt = 0;
    };

    template<typename T>
    VulkanMesh createVulkanMesh(VulkanInitData &vkInitData, HostMesh<T> &hostMesh,
                                bool useStaging) {
        // Set up Vulkan mesh
        VulkanMesh mesh;
        // Create appropriate VMA info and usage flags
        VmaAllocationCreateInfo vmaInfo {};
        vk::BufferUsageFlags vertUsageFlags = vk::BufferUsageFlagBits::eVertexBuffer;
        vk::BufferUsageFlags indexUsageFlags = vk::BufferUsageFlagBits::eIndexBuffer;
        
        if(useStaging) {
            vmaInfo = createVMADeviceLocalInfo();
            vertUsageFlags |= vk::BufferUsageFlagBits::eTransferDst;
            indexUsageFlags |= vk::BufferUsageFlagBits::eTransferDst;
        }
        else {
            vmaInfo = createVMAHostVisibleInfo();
        }
    // Create vertex buffer and index buffer
        vk::DeviceSize vertBufferSize =
            sizeof(hostMesh.vertices[0]) * hostMesh.vertices.size();
        mesh.vertices = createVulkanBuffer(vkInitData, vertBufferSize,
                            vertUsageFlags, vmaInfo);
        
        vk::DeviceSize indexBufferSize =
            sizeof(hostMesh.indices[0]) * hostMesh.indices.size();
        mesh.indices = createVulkanBuffer(vkInitData, indexBufferSize,
                            indexUsageFlags, vmaInfo);
        // Return mesh
        return mesh;
    }

    template<typename T>
    void copyToVulkanMesh( VulkanInitData &vkInitData, VulkanMesh &mesh,
                            HostMesh<T> &hostMesh, bool useStaging,
                            VulkanStagingData &stagingData = nullptr)
        if(useStaging) {
            copyToDeviceLocalVulkanBuffer(vkInitData, stagingData, mesh.vertices,
                                            hostMesh.vertices.data());
            copyToDeviceLocalVulkanBuffer(vkInitData, stagingData, mesh.indices,
                                            hostMesh.indices.data());
        }
        else {
            copyToHostVisibleVulkanBuffer(vkInitData, mesh.vertices,
                                            hostMesh.vertices.data());
            copyToHostVisibleVulkanBuffer(vkInitData, mesh.indices,
                                            hostMesh.indices.data());
        }
        mesh.indexCnt = hostMesh.indices.size();
    }

    void cleanupVulkanMesh(VulkanInitData &vkInitData,
                        VulkanMesh &mesh);

    void recordDrawVulkanMesh(vk::CommandBuffer &commandBuffer,
                        VulkanMesh &mesh);

}