#pragma once

#include "vulkanSetup.h"

class IGpuImageObject;

namespace vulkan {
void submitDisplayTransfer(VkCommandBuffer cmdBuffer, VulkanObj* obj, VkSemaphore semPresentReady, VkSemaphore displayReady, VkSemaphore waitSema,
                           size_t waitValue);

void transfer2Display(VkCommandBuffer cmdBuffer, VulkanObj* obj, vulkan::SwapchainData& swapchain, VkImage displayImage, IGpuImageObject* image,
                      uint32_t index);

void presentImage(VulkanObj* obj, SwapchainData& swapchain, uint32_t& index);

void waitFlipped(VulkanObj* obj); /// Call before submit
} // namespace vulkan
