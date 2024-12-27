// Copyright (c) 2024 Lukasz Stalmirski
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#include "vk_mock_icd_base.h"

namespace vkmock
{
    struct PhysicalDevice;
    struct Queue;

    struct Device : DeviceBase
    {
        PhysicalDevice& m_PhysicalDevice;
        VkQueue m_Queue;

        Device( PhysicalDevice& physicalDevice, const VkDeviceCreateInfo& createInfo );
        ~Device();

        void vkDestroyDevice( const VkAllocationCallbacks* pAllocator ) override;

        void vkGetDeviceQueue( uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue ) override;
        void vkGetDeviceQueue2( const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue ) override;

        VkResult vkCreateQueryPool( const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool ) override;
        void vkDestroyQueryPool( VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator ) override;

        VkResult vkAllocateCommandBuffers( const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers ) override;
        void vkFreeCommandBuffers( VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers ) override;

        VkResult vkAllocateMemory( const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory ) override;
        void vkFreeMemory( VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator ) override;
        VkResult vkMapMemory( VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData ) override;

        VkResult vkCreateBuffer( const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer ) override;
        void vkDestroyBuffer( VkBuffer buffer, const VkAllocationCallbacks* pAllocator ) override;
        void vkGetBufferMemoryRequirements( VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements ) override;
        VkResult vkBindBufferMemory( VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset ) override;
        void vkGetBufferMemoryRequirements2( const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements ) override;
        VkResult vkBindBufferMemory2( uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos ) override;

        VkResult vkCreateImage( const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage ) override;
        void vkDestroyImage( VkImage image, const VkAllocationCallbacks* pAllocator ) override;
        void vkGetImageMemoryRequirements( VkImage image, VkMemoryRequirements* pMemoryRequirements ) override;
        VkResult vkBindImageMemory( VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset ) override;
        void vkGetImageMemoryRequirements2( const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements ) override;
        VkResult vkBindImageMemory2( uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos ) override;

#ifdef VK_KHR_swapchain
        VkResult vkCreateSwapchainKHR( const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain ) override;
        void vkDestroySwapchainKHR( VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator ) override;
        VkResult vkAcquireNextImageKHR( VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex ) override;
#endif
    };
}