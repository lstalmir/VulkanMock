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
    struct Device : DeviceBase
    {
        VkPhysicalDevice m_PhysicalDevice;
        VkQueue m_Queue;

        Device( VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo& createInfo );
        ~Device();

        void vkDestroyDevice( const VkAllocationCallbacks* pAllocator );

        void vkGetDeviceQueue( uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue );
        void vkGetDeviceQueue2( const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue );

        VkResult vkCreateQueryPool( const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool );
        void vkDestroyQueryPool( VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator );

        VkResult vkCreateCommandPool( const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool );
        void vkDestroyCommandPool( VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator );
        VkResult vkResetCommandPool( VkCommandPool commandPool, VkCommandPoolResetFlags flags );

        VkResult vkAllocateCommandBuffers( const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers );
        void vkFreeCommandBuffers( VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers );

        VkResult vkAllocateMemory( const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory );
        void vkFreeMemory( VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator );
        VkResult vkMapMemory( VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData );

        VkResult vkCreateBuffer( const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer );
        void vkDestroyBuffer( VkBuffer buffer, const VkAllocationCallbacks* pAllocator );
        void vkGetBufferMemoryRequirements( VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements );
        VkResult vkBindBufferMemory( VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset );

        VkResult vkCreateImage( const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage );
        void vkDestroyImage( VkImage image, const VkAllocationCallbacks* pAllocator );
        void vkGetImageMemoryRequirements( VkImage image, VkMemoryRequirements* pMemoryRequirements );
        VkResult vkBindImageMemory( VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset );

        void vkGetBufferMemoryRequirements2( const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements );
        void vkGetImageMemoryRequirements2( const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements );

        VkResult vkBindBufferMemory2( uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos );
        VkResult vkBindImageMemory2( uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos );

#ifdef VK_KHR_swapchain
        VkResult vkCreateSwapchainKHR( const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain );
        void vkDestroySwapchainKHR( VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator );
        VkResult vkGetSwapchainImagesKHR( VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages );
        VkResult vkAcquireNextImageKHR( VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex );
        VkResult vkAcquireNextImage2KHR( const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex );
#endif
    };
}

struct VkDevice_T : vkmock::Device
{
    using Device::Device;
};
