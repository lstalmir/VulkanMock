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

#include "vk_mock_device.h"
#include "vk_mock_physical_device.h"
#include "vk_mock_buffer.h"
#include "vk_mock_device_memory.h"
#include "vk_mock_query_pool.h"
#include "vk_mock_queue.h"
#include "vk_mock_command_buffer.h"
#include "vk_mock_swapchain.h"
#include "vk_mock_image.h"
#include "vk_mock_icd_helpers.h"

namespace vkmock
{
    Device::Device( PhysicalDevice& physicalDevice, const VkDeviceCreateInfo& createInfo )
        : m_PhysicalDevice( physicalDevice )
        , m_Queue( nullptr )
    {
        if( createInfo.queueCreateInfoCount > 0 )
        {
            vk_check( vk_new<Queue>(
                &m_Queue, *this, createInfo.pQueueCreateInfos[ 0 ] ) );
        }
    }

    Device::~Device()
    {
        delete m_Queue;
    }

    void Device::vkDestroyDevice( const VkAllocationCallbacks* pAllocator )
    {
        delete this;
    }

    void Device::vkGetDeviceQueue( uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue )
    {
        *pQueue = m_Queue;
    }

    void Device::vkGetDeviceQueue2( const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue )
    {
        *pQueue = m_Queue;
    }

    VkResult Device::vkCreateQueryPool( const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool )
    {
        return vk_new_nondispatchable<VkQueryPool_T>( pQueryPool, *pCreateInfo );
    }

    void Device::vkDestroyQueryPool( VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator )
    {
        delete queryPool;
    }

    VkResult Device::vkAllocateCommandBuffers( const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers )
    {
        VkResult result = VK_SUCCESS;

        for( uint32_t i = 0; i < pAllocateInfo->commandBufferCount; ++i )
        {
            result = vk_new<CommandBuffer>( &pCommandBuffers[ i ] );

            if( result != VK_SUCCESS )
            {
                vkFreeCommandBuffers( pAllocateInfo->commandPool, i, pCommandBuffers );
                break;
            }
        }

        return result;
    }

    void Device::vkFreeCommandBuffers( VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers )
    {
        for( uint32_t i = 0; i < commandBufferCount; ++i )
        {
            delete pCommandBuffers[ i ];
        }
    }

    VkResult Device::vkAllocateMemory( const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory )
    {
        return vk_new_nondispatchable<VkDeviceMemory_T>( pMemory, pAllocateInfo->allocationSize );
    }

    void Device::vkFreeMemory( VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator )
    {
        delete memory;
    }

    VkResult Device::vkMapMemory( VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData )
    {
        *ppData = memory->m_pAllocation + offset;
        return VK_SUCCESS;
    }

    VkResult Device::vkCreateBuffer( const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer )
    {
        return vk_new_nondispatchable<VkBuffer_T>( pBuffer, *pCreateInfo );
    }

    void Device::vkDestroyBuffer( VkBuffer buffer, const VkAllocationCallbacks* pAllocator )
    {
        delete buffer;
    }

    void Device::vkGetBufferMemoryRequirements( VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements )
    {
        pMemoryRequirements->size = buffer->m_Size;
        pMemoryRequirements->alignment = 1;
        pMemoryRequirements->memoryTypeBits = 1;
    }

    VkResult Device::vkBindBufferMemory( VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset )
    {
        buffer->m_pData = memory->m_pAllocation + memoryOffset;
        return VK_SUCCESS;
    }

    void Device::vkGetBufferMemoryRequirements2( const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements )
    {
        vkGetBufferMemoryRequirements( pInfo->buffer, &pMemoryRequirements->memoryRequirements );
    }

    VkResult Device::vkBindBufferMemory2( uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos )
    {
        for( uint32_t i = 0; i < bindInfoCount; ++i )
        {
            vkBindBufferMemory( pBindInfos[ i ].buffer, pBindInfos[ i ].memory, pBindInfos[ i ].memoryOffset );
        }

        return VK_SUCCESS;
    }

    VkResult Device::vkCreateImage( const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage )
    {
        return vk_new_nondispatchable<VkImage_T>( pImage, *pCreateInfo );
    }

    void Device::vkDestroyImage( VkImage image, const VkAllocationCallbacks* pAllocator )
    {
        delete image;
    }

    void Device::vkGetImageMemoryRequirements( VkImage image, VkMemoryRequirements* pMemoryRequirements )
    {
        pMemoryRequirements->size = image->m_Extent.width * image->m_Extent.height * image->m_Extent.depth * 4;
        pMemoryRequirements->alignment = 1;
        pMemoryRequirements->memoryTypeBits = 1;
    }

    VkResult Device::vkBindImageMemory( VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset )
    {
        image->m_pData = memory->m_pAllocation + memoryOffset;
        return VK_SUCCESS;
    }

    void Device::vkGetImageMemoryRequirements2( const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements )
    {
        vkGetImageMemoryRequirements( pInfo->image, &pMemoryRequirements->memoryRequirements );
    }

    VkResult Device::vkBindImageMemory2( uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos )
    {
        for( uint32_t i = 0; i < bindInfoCount; ++i )
        {
            vkBindImageMemory( pBindInfos[ i ].image, pBindInfos[ i ].memory, pBindInfos[ i ].memoryOffset );
        }

        return VK_SUCCESS;
    }

#ifdef VK_KHR_swapchain
    VkResult Device::vkCreateSwapchainKHR( const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain )
    {
        return vk_new_nondispatchable<VkSwapchainKHR_T>( pSwapchain, *pCreateInfo );
    }

    void Device::vkDestroySwapchainKHR( VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator )
    {
        delete swapchain;
    }

    VkResult Device::vkAcquireNextImageKHR( VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex )
    {
        *pImageIndex = 0;
        return VK_SUCCESS;
    }
#endif
}