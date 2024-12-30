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

#include "vk_mock_queue.h"
#include "vk_mock_device.h"
#include "vk_mock_command_buffer.h"
#include "vk_mock_query_pool.h"
#include "vk_mock_buffer.h"
#include <chrono>
#include <thread>

namespace vkmock
{
    Queue::Queue( VkDevice device, const VkDeviceQueueCreateInfo& createInfo )
    {
        m_pMockFunctions = device->m_pMockFunctions;
    }

    Queue::~Queue()
    {
    }

    VkResult Queue::vkQueueSubmit( uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence )
    {
        if( m_pMockFunctions->vkQueueSubmit )
        {
            return m_pMockFunctions->vkQueueSubmit( GetApiHandle(), submitCount, pSubmits, fence );
        }

        for( uint32_t i = 0; i < submitCount; ++i )
        {
            for( uint32_t j = 0; j < pSubmits[ i ].commandBufferCount; ++j )
            {
                ExecuteCommandBuffer( pSubmits[ i ].pCommandBuffers[ j ] );
            }
        }

        return VK_SUCCESS;
    }

    VkResult Queue::vkQueueSubmit2( uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence )
    {
        if( m_pMockFunctions->vkQueueSubmit2 )
        {
            return m_pMockFunctions->vkQueueSubmit2( GetApiHandle(), submitCount, pSubmits, fence );
        }

        for( uint32_t i = 0; i < submitCount; ++i )
        {
            for( uint32_t j = 0; j < pSubmits[ i ].commandBufferInfoCount; ++j )
            {
                ExecuteCommandBuffer( pSubmits[ i ].pCommandBufferInfos[ j ].commandBuffer );
            }
        }

        return VK_SUCCESS;
    }

    void Queue::ExecuteCommandBuffer( VkCommandBuffer commandBuffer )
    {
        for( VkMockCommandEXT& cmd : commandBuffer->m_Commands )
        {
            if( cmd.pfnExecute )
            {
                cmd.pfnExecute( GetApiHandle(), &cmd );
            }
        }
    }
}
