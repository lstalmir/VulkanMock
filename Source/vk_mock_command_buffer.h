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
#include "vk_mock.h"
#include "vk_mock_icd_base.h"
#include "vk_mock_icd_helpers.h"
#include <vector>

namespace vkmock
{
    struct CommandBuffer : CommandBufferBase
    {
        VkCommandPool m_CommandPool;
        std::vector<VkMockCommandEXT, vk_stl_allocator<VkMockCommandEXT>> m_Commands;

        CommandBuffer( VkDevice device, VkCommandPool commandPool );
        ~CommandBuffer();

        void Reset();

        VkResult vkBeginCommandBuffer( const VkCommandBufferBeginInfo* pBeginInfo );
        VkResult vkResetCommandBuffer( VkCommandBufferResetFlags flags );

        void vkCmdDraw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance );
        void vkCmdDispatch( uint32_t x, uint32_t y, uint32_t z );
        void vkCmdExecuteCommands( uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers );
        void vkCmdWriteTimestamp( VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query );
        void vkCmdCopyBuffer( VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions );
        void vkCmdCopyQueryPoolResults( VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags );
    };
}

struct VkCommandBuffer_T : vkmock::CommandBuffer
{
    using CommandBuffer::CommandBuffer;
};
