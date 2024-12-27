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
#include <vector>

namespace vkmock
{
    struct Command
    {
        enum Type
        {
            eUnknown,
            eDraw,
            eDispatch,
            eExecuteCommands,
            eWriteTimestamp,
            eCopyBuffer,
            eCopyQueryPoolResults
        };

        struct Draw
        {
            uint32_t m_VertexCount;
            uint32_t m_InstanceCount;
            uint32_t m_FirstVertex;
            uint32_t m_FirstInstance;
        };

        struct Dispatch
        {
            uint32_t m_GroupCountX;
            uint32_t m_GroupCountY;
            uint32_t m_GroupCountZ;
        };

        struct ExecuteCommands
        {
            VkCommandBuffer m_CommandBuffer;
        };

        struct WriteTimestamp
        {
            VkQueryPool m_QueryPool;
            uint32_t m_Index;
        };

        struct CopyBuffer
        {
            VkBuffer m_SrcBuffer;
            VkBuffer m_DstBuffer;
            VkBufferCopy m_Copy;
        };

        struct CopyQueryPoolResults
        {
            VkQueryPool m_QueryPool;
            uint32_t m_FirstQuery;
            uint32_t m_QueryCount;
            VkBuffer m_DstBuffer;
            VkDeviceSize m_DstOffset;
            VkDeviceSize m_Stride;
            VkQueryResultFlags m_Flags;
        };

        Type m_Type = eUnknown;
        union
        {
            Draw m_Draw;
            Dispatch m_Dispatch;
            ExecuteCommands m_ExecuteCommands;
            WriteTimestamp m_WriteTimestamp;
            CopyBuffer m_CopyBuffer;
            CopyQueryPoolResults m_CopyQueryPoolResults;
        };
    };

    struct CommandBuffer : CommandBufferBase
    {
        std::vector<Command> m_Commands;

        CommandBuffer();
        ~CommandBuffer();

        VkResult vkBeginCommandBuffer( const VkCommandBufferBeginInfo* pBeginInfo ) override;
        VkResult vkResetCommandBuffer( VkCommandBufferResetFlags flags ) override;

        void vkCmdDraw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance ) override;
        void vkCmdDispatch( uint32_t x, uint32_t y, uint32_t z ) override;
        void vkCmdExecuteCommands( uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers ) override;
        void vkCmdWriteTimestamp( VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query ) override;
        void vkCmdCopyBuffer( VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions ) override;
        void vkCmdCopyQueryPoolResults( VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags ) override;
    };
}