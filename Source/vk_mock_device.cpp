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
#include "vk_mock_command_pool.h"
#include "vk_mock_swapchain.h"
#include "vk_mock_image.h"
#include "vk_mock_icd_helpers.h"

namespace vkmock
{
    Device::Device( VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo& createInfo )
        : m_PhysicalDevice( physicalDevice )
        , m_Queue( nullptr )
    {
        try
        {
            m_pMockFunctions = new Functions();

            if( createInfo.queueCreateInfoCount > 0 )
            {
                vk_check( vk_new(
                    &m_Queue, GetApiHandle(), createInfo.pQueueCreateInfos[ 0 ] ) );
            }
        }
        catch( ... )
        {
            Device::~Device();
            throw;
        }
    }

    Device::~Device()
    {
        delete m_Queue;
        delete m_pMockFunctions;
    }

    void Device::vkDestroyDevice( const VkAllocationCallbacks* pAllocator )
    {
        if( m_pMockFunctions->vkDestroyDevice )
        {
            m_pMockFunctions->vkDestroyDevice(
                GetApiHandle(),
                pAllocator );
        }

        delete this;
    }

    void Device::vkGetDeviceQueue( uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue )
    {
        if( m_pMockFunctions->vkGetDeviceQueue )
        {
            return m_pMockFunctions->vkGetDeviceQueue(
                GetApiHandle(),
                queueFamilyIndex,
                queueIndex,
                pQueue );
        }

        *pQueue = m_Queue;
    }

    void Device::vkGetDeviceQueue2( const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue )
    {
        if( m_pMockFunctions->vkGetDeviceQueue2 )
        {
            return m_pMockFunctions->vkGetDeviceQueue2(
                GetApiHandle(),
                pQueueInfo,
                pQueue );
        }

        *pQueue = m_Queue;
    }

    VkResult Device::vkCreateQueryPool( const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool )
    {
        if( m_pMockFunctions->vkCreateQueryPool )
        {
            return m_pMockFunctions->vkCreateQueryPool(
                GetApiHandle(),
                pCreateInfo,
                pAllocator,
                pQueryPool );
        }

        return vk_new( pQueryPool, *pCreateInfo );
    }

    void Device::vkDestroyQueryPool( VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator )
    {
        if( m_pMockFunctions->vkDestroyQueryPool )
        {
            return m_pMockFunctions->vkDestroyQueryPool(
                GetApiHandle(),
                queryPool,
                pAllocator );
        }

        delete queryPool;
    }

    VkResult Device::vkCreateCommandPool( const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool )
    {
        if( m_pMockFunctions->vkCreateCommandPool )
        {
            return m_pMockFunctions->vkCreateCommandPool(
                GetApiHandle(),
                pCreateInfo,
                pAllocator,
                pCommandPool );
        }

        return vk_new( pCommandPool );
    }

    void Device::vkDestroyCommandPool( VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator )
    {
        if( m_pMockFunctions->vkDestroyCommandPool )
        {
            return m_pMockFunctions->vkDestroyCommandPool(
                GetApiHandle(),
                commandPool,
                pAllocator );
        }

        delete commandPool;
    }

    VkResult Device::vkResetCommandPool( VkCommandPool commandPool, VkCommandPoolResetFlags flags )
    {
        if( m_pMockFunctions->vkResetCommandPool )
        {
            return m_pMockFunctions->vkResetCommandPool(
                GetApiHandle(),
                commandPool,
                flags );
        }

        for( VkCommandBuffer commandBuffer : commandPool->m_CommandBuffers )
        {
            commandBuffer->Reset();
        }

        return VK_SUCCESS;
    }

    VkResult Device::vkAllocateCommandBuffers( const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers )
    {
        if( m_pMockFunctions->vkAllocateCommandBuffers )
        {
            return m_pMockFunctions->vkAllocateCommandBuffers(
                GetApiHandle(),
                pAllocateInfo,
                pCommandBuffers );
        }

        for( uint32_t i = 0; i < pAllocateInfo->commandBufferCount; ++i )
        {
            VkResult result = vk_new(
                &pCommandBuffers[ i ], GetApiHandle(), pAllocateInfo->commandPool );

            if( result != VK_SUCCESS )
            {
                vkFreeCommandBuffers( pAllocateInfo->commandPool, i, pCommandBuffers );
                return result;
            }
        }

        return VK_SUCCESS;
    }

    void Device::vkFreeCommandBuffers( VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers )
    {
        if( m_pMockFunctions->vkFreeCommandBuffers )
        {
            return m_pMockFunctions->vkFreeCommandBuffers(
                GetApiHandle(),
                commandPool,
                commandBufferCount,
                pCommandBuffers );
        }

        for( uint32_t i = 0; i < commandBufferCount; ++i )
        {
            delete pCommandBuffers[ i ];
        }
    }

    VkResult Device::vkAllocateMemory( const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory )
    {
        if( m_pMockFunctions->vkAllocateMemory )
        {
            return m_pMockFunctions->vkAllocateMemory(
                GetApiHandle(),
                pAllocateInfo,
                pAllocator,
                pMemory );
        }

        return vk_new( pMemory, pAllocateInfo->allocationSize );
    }

    void Device::vkFreeMemory( VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator )
    {
        if( m_pMockFunctions->vkFreeMemory )
        {
            m_pMockFunctions->vkFreeMemory(
                GetApiHandle(),
                memory,
                pAllocator );
        }

        delete memory;
    }

    VkResult Device::vkMapMemory( VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData )
    {
        if( m_pMockFunctions->vkMapMemory )
        {
            return m_pMockFunctions->vkMapMemory(
                GetApiHandle(),
                memory,
                offset,
                size,
                flags,
                ppData );
        }

        *ppData = memory->m_pAllocation + offset;

        return VK_SUCCESS;
    }

    VkResult Device::vkCreateBuffer( const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer )
    {
        if( m_pMockFunctions->vkCreateBuffer )
        {
            return m_pMockFunctions->vkCreateBuffer(
                GetApiHandle(),
                pCreateInfo,
                pAllocator,
                pBuffer );
        }

        return vk_new( pBuffer, *pCreateInfo );
    }

    void Device::vkDestroyBuffer( VkBuffer buffer, const VkAllocationCallbacks* pAllocator )
    {
        if( m_pMockFunctions->vkDestroyBuffer )
        {
            m_pMockFunctions->vkDestroyBuffer(
                GetApiHandle(),
                buffer,
                pAllocator );
        }

        delete buffer;
    }

    void Device::vkGetBufferMemoryRequirements( VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements )
    {
        if( m_pMockFunctions->vkGetBufferMemoryRequirements )
        {
            return m_pMockFunctions->vkGetBufferMemoryRequirements(
                GetApiHandle(),
                buffer,
                pMemoryRequirements );
        }

        pMemoryRequirements->size = buffer->m_Size;
        pMemoryRequirements->alignment = 1;
        pMemoryRequirements->memoryTypeBits = 1;
    }

    VkResult Device::vkBindBufferMemory( VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset )
    {
        if( m_pMockFunctions->vkBindBufferMemory )
        {
            return m_pMockFunctions->vkBindBufferMemory(
                GetApiHandle(),
                buffer,
                memory,
                memoryOffset );
        }

        buffer->m_pData = memory->m_pAllocation + memoryOffset;

        return VK_SUCCESS;
    }

    void Device::vkGetBufferMemoryRequirements2( const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements )
    {
        if( m_pMockFunctions->vkGetBufferMemoryRequirements2 )
        {
            m_pMockFunctions->vkGetBufferMemoryRequirements2(
                GetApiHandle(),
                pInfo,
                pMemoryRequirements );
        }

        pMemoryRequirements->memoryRequirements.size = pInfo->buffer->m_Size;
        pMemoryRequirements->memoryRequirements.alignment = 1;
        pMemoryRequirements->memoryRequirements.memoryTypeBits = 1;
    }

    VkResult Device::vkBindBufferMemory2( uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos )
    {
        if( m_pMockFunctions->vkBindBufferMemory2 )
        {
            return m_pMockFunctions->vkBindBufferMemory2(
                GetApiHandle(),
                bindInfoCount,
                pBindInfos );
        }

        for( uint32_t i = 0; i < bindInfoCount; ++i )
        {
            pBindInfos[ i ].buffer->m_pData = pBindInfos[ i ].memory->m_pAllocation + pBindInfos[ i ].memoryOffset;
        }

        return VK_SUCCESS;
    }

    VkResult Device::vkCreateImage( const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage )
    {
        if( m_pMockFunctions->vkCreateImage )
        {
            return m_pMockFunctions->vkCreateImage(
                GetApiHandle(),
                pCreateInfo,
                pAllocator,
                pImage );
        }

        return vk_new( pImage, *pCreateInfo );
    }

    void Device::vkDestroyImage( VkImage image, const VkAllocationCallbacks* pAllocator )
    {
        if( m_pMockFunctions->vkDestroyImage )
        {
            m_pMockFunctions->vkDestroyImage(
                GetApiHandle(),
                image,
                pAllocator );
        }

        delete image;
    }

    void Device::vkGetImageMemoryRequirements( VkImage image, VkMemoryRequirements* pMemoryRequirements )
    {
        if( m_pMockFunctions->vkGetImageMemoryRequirements )
        {
            return m_pMockFunctions->vkGetImageMemoryRequirements(
                GetApiHandle(),
                image,
                pMemoryRequirements );
        }

        VkExtent3D extent = image->m_Extent;
        pMemoryRequirements->size = extent.width * extent.height * extent.depth * 4;
        pMemoryRequirements->alignment = 1;
        pMemoryRequirements->memoryTypeBits = 1;
    }

    VkResult Device::vkBindImageMemory( VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset )
    {
        if( m_pMockFunctions->vkBindImageMemory )
        {
            return m_pMockFunctions->vkBindImageMemory(
                GetApiHandle(),
                image,
                memory,
                memoryOffset );
        }

        image->m_pData = memory->m_pAllocation + memoryOffset;
        return VK_SUCCESS;
    }

    void Device::vkGetImageMemoryRequirements2( const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements )
    {
        if( m_pMockFunctions->vkGetImageMemoryRequirements2 )
        {
            m_pMockFunctions->vkGetImageMemoryRequirements2(
                GetApiHandle(),
                pInfo,
                pMemoryRequirements );
        }

        VkExtent3D extent = pInfo->image->m_Extent;
        pMemoryRequirements->memoryRequirements.size = extent.width * extent.height * extent.depth * 4;
        pMemoryRequirements->memoryRequirements.alignment = 1;
        pMemoryRequirements->memoryRequirements.memoryTypeBits = 1;
    }

    VkResult Device::vkBindImageMemory2( uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos )
    {
        if( m_pMockFunctions->vkBindImageMemory2 )
        {
            return m_pMockFunctions->vkBindImageMemory2(
                GetApiHandle(),
                bindInfoCount,
                pBindInfos );
        }

        for( uint32_t i = 0; i < bindInfoCount; ++i )
        {
            vkBindImageMemory( pBindInfos[ i ].image, pBindInfos[ i ].memory, pBindInfos[ i ].memoryOffset );
        }

        return VK_SUCCESS;
    }

#ifdef VK_KHR_swapchain
    VkResult Device::vkCreateSwapchainKHR( const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain )
    {
        if( m_pMockFunctions->vkCreateSwapchainKHR )
        {
            return m_pMockFunctions->vkCreateSwapchainKHR(
                GetApiHandle(),
                pCreateInfo,
                pAllocator,
                pSwapchain );
        }

        return vk_new( pSwapchain, *pCreateInfo );
    }

    void Device::vkDestroySwapchainKHR( VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator )
    {
        if( m_pMockFunctions->vkDestroySwapchainKHR )
        {
            m_pMockFunctions->vkDestroySwapchainKHR(
                GetApiHandle(),
                swapchain,
                pAllocator );
        }

        delete swapchain;
    }

    VkResult Device::vkAcquireNextImageKHR( VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex )
    {
        if( m_pMockFunctions->vkAcquireNextImageKHR )
        {
            return m_pMockFunctions->vkAcquireNextImageKHR(
                GetApiHandle(),
                swapchain,
                timeout,
                semaphore,
                fence,
                pImageIndex );
        }

        *pImageIndex = 0;
        return VK_SUCCESS;
    }
#endif
}
