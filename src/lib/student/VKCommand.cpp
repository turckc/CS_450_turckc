#include "student/VKCommand.hpp"

namespace student {
    vk::CommandPool createVulkanCommandPool(VulkanInitData &vkInitData,
                                                unsigned int queueIndex) {
        return vkInitData.device.createCommandPool(
            vk::CommandPoolCreateInfo(
                vk::CommandPoolCreateFlags(
                        vk::CommandPoolCreateFlagBits::eResetCommandBuffer),
                        // Reset pool every frame
                queueIndex));
    }

    void cleanupVulkanCommandPool(VulkanInitData &vkInitData,
                                    vk::CommandPool &commandPool) {
        vkInitData.device.destroyCommandPool(commandPool);
    }

    vk::CommandBuffer createVulkanCommandBuffer(VulkanInitData &vkInitData,
                                            vk::CommandPool &commandPool)
    {
        return vkInitData.device.allocateCommandBuffers(
            vk::CommandBufferAllocateInfo(commandPool,
                    vk::CommandBufferLevel::ePrimary, 1)).front();
    }

    vk::Fence createVulkanFence(VulkanInitData &vkInitData) {
        return vkInitData.device.createFence(
            vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
    }
    
    void cleanupVulkanFence(VulkanInitData &vkInitData, vk::Fence &f) {
        vkInitData.device.destroyFence(f);
    }

    vk::Semaphore createVulkanSemaphore(VulkanInitData &vkInitData) {
        return vkInitData.device.createSemaphore(
                                        vk::SemaphoreCreateInfo());
    }

    void cleanupVulkanSemaphore(VulkanInitData &vkInitData,
                                vk::Semaphore &s) {
        vkInitData.device.destroySemaphore(s);
    }

    VulkanCommandData createVulkanCommandData(VulkanInitData &vkInitData,
                                                int numberFramesInFlight) {
        VulkanCommandData commandData {};
        commandData.numberFramesInFlight = numberFramesInFlight;
        commandData.commandPool = createVulkanCommandPool(vkInitData,
                                                vkInitData.graphicsQueue.index);
        
        for(int i = 0; i < vkInitData.swapchain.images.size(); i++) {
            commandData.perSwapRenderDone.push_back(
                    createVulkanSemaphore(vkInitData));
        }

        for(unsigned int i = 0; i < commandData.numberFramesInFlight; i++) {
            VulkanFIFCommandData frameData;
            frameData.commandBuffer = createVulkanCommandBuffer(vkInitData,
                                                        commandData.commandPool);
            frameData.imageAvailableSemaphore = createVulkanSemaphore(vkInitData);
            frameData.inFlightFence = createVulkanFence(vkInitData);
            commandData.perFIF.push_back(frameData);
        }
        return commandData;
    }

    void cleanupVulkanCommandData(VulkanInitData &vkInitData,
                                    VulkanCommandData &commandData) {

        for(int i = 0; i < commandData.perSwapRenderDone.size(); i++) {
            cleanupVulkanSemaphore(vkInitData, commandData.perSwapRenderDone.at(i));
        }
        commandData.perSwapRenderDone.clear();

        for(unsigned int i = 0; i < commandData.numberFramesInFlight; i++) {
            cleanupVulkanSemaphore(vkInitData,
                            commandData.perFIF.at(i).imageAvailableSemaphore);
            cleanupVulkanFence(vkInitData, commandData.perFIF.at(i).inFlightFence);
        }
        commandData.perFIF.clear();

        cleanupVulkanCommandPool(vkInitData, commandData.commandPool);
    }

    uint32_t prepareFrameInFlight( VulkanInitData &vkInitData,
                                    VulkanCommandData &commandData,
                                    uint32_t indexFIF) {
        // CPU waiting until FIF has completed any rendering commands.
        vkInitData.device.waitForFences(
                commandData.perFIF[indexFIF].inFlightFence, true, UINT64_MAX);
        // Get next swap image
        auto frameResult = vkInitData.device.acquireNextImageKHR(
                vkInitData.swapchain.chain, UINT64_MAX,
                commandData.perFIF[indexFIF].imageAvailableSemaphore,
                nullptr);
        // Get swap image index
        int indexSwap = frameResult.value;
        // Reset the fence since we're about to submit work
        vkInitData.device.resetFences(
                commandData.perFIF[indexFIF].inFlightFence);
        // Reset our command buffer so it's cleared and ready to go
        commandData.perFIF[indexFIF].commandBuffer.reset();
        // Return SWAP image index
        return indexSwap;
    }

    void submitToGraphicsQueue( VulkanInitData &vkInitData,
                                VulkanCommandData &commandData,
                                uint32_t indexFIF,
                                uint32_t indexSwap) {
        // Set up our semaphores and stages to wait on
        vk::Semaphore waitSemaphores[] =
                {commandData.perFIF[indexFIF].imageAvailableSemaphore};
        vk::Semaphore signalSemaphores[] =
                {commandData.perSwapRenderDone[indexSwap]};
        vk::PipelineStageFlags waitStages[] =
                {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        
        // Prepare submission and submit
        // (Noting that we will also signal the fence as well)
        vk::SubmitInfo submitInfo(waitSemaphores, waitStages,
                        commandData.perFIF[indexFIF].commandBuffer,
                        signalSemaphores);
        vkInitData.graphicsQueue.queue.submit(submitInfo,
                commandData.perFIF[indexFIF].inFlightFence);
    }

    bool presentSwapImage( VulkanInitData &vkInitData,
                            VulkanCommandData &commandData,
                            uint32_t indexFIF,
                            uint32_t indexSwap) {
        vk::PresentInfoKHR presentInfo {};
        presentInfo.setWaitSemaphores(commandData.perSwapRenderDone[indexSwap]);
        presentInfo.setSwapchains(vkInitData.swapchain.chain);
        presentInfo.setImageIndices(indexSwap);
        
        bool successPresent = true;
        try {
            auto presentRes = vkInitData.presentQueue.queue.presentKHR(presentInfo);
        }
        catch(const vk::OutOfDateKHRError& e) {
            // Will probably have to recreate the swap chain
            successPresent = false;
        }
        return successPresent;
    }

}
