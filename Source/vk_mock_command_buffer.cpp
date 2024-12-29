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

#include "vk_mock_command_buffer.h"
#include "vk_mock_queue.h"
#include "vk_mock_query_pool.h"
#include "vk_mock_buffer.h"

#include <chrono>
#include <thread>
#include <string.h>

namespace vkmock
{
    CommandBuffer::CommandBuffer()
        : m_Commands( 0 )
    {
    }

    CommandBuffer::~CommandBuffer()
    {
        Reset();
    }

    void CommandBuffer::Reset()
    {
        for( VkMockCommandEXT& cmd : m_Commands )
        {
            if( cmd.pfnFree )
            {
                cmd.pfnFree( &cmd );
            }
        }

        m_Commands.clear();
    }

    VkResult CommandBuffer::vkBeginCommandBuffer( const VkCommandBufferBeginInfo* pBeginInfo )
    {
        return vkResetCommandBuffer( 0 );
    }

    VkResult CommandBuffer::vkResetCommandBuffer( VkCommandBufferResetFlags flags )
    {
        Reset();

        if( m_pMockFunctions && m_pMockFunctions->vkResetCommandBuffer )
        {
            return m_pMockFunctions->vkResetCommandBuffer( GetApiHandle(), flags );
        }

        return VK_SUCCESS;
    }

    void CommandBuffer::vkCmdDraw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance )
    {
        VkMockCommandEXT command = {};
        command.data.u32[ 0 ] = vertexCount * instanceCount;
        command.pfnExecute = []( VkQueue, VkMockCommandEXT* cmd ) {
            std::this_thread::sleep_for(
                std::chrono::nanoseconds( cmd->data.u32[ 0 ] ) );
        };

        m_Commands.push_back( command );
    }

    void CommandBuffer::vkCmdDispatch( uint32_t x, uint32_t y, uint32_t z )
    {
        VkMockCommandEXT command = {};
        command.data.u32[ 0 ] = x * y * z;
        command.pfnExecute = []( VkQueue, VkMockCommandEXT* cmd ) {
            std::this_thread::sleep_for(
                std::chrono::nanoseconds( cmd->data.u32[ 0 ] ) );
        };

        m_Commands.push_back( command );
    }

    void CommandBuffer::vkCmdExecuteCommands( uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers )
    {
        for( uint32_t i = 0; i < commandBufferCount; ++i )
        {
            VkMockCommandEXT command = {};
            command.data.ptr = pCommandBuffers[ i ];
            command.pfnExecute = []( VkQueue queue, VkMockCommandEXT* pCommand ) {
                queue->ExecuteCommandBuffer( static_cast<VkCommandBuffer>( pCommand->data.ptr ) );
            };

            m_Commands.push_back( command );
        }
    }

    void CommandBuffer::vkCmdWriteTimestamp( VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query )
    {
        struct CommandData
        {
            VkQueryPool queryPool;
            uint32_t query;
        };

        static_assert( sizeof( CommandData ) <= sizeof( VkMockCommandEXT::data ),
            "Command data size exceeds VkMockCommandEXT::data size" );

        VkMockCommandEXT command = {};
        CommandData& cmdData = *reinterpret_cast<CommandData*>( command.data.u64 );
        cmdData.queryPool = queryPool;
        cmdData.query = query;

        command.pfnExecute = []( VkQueue queue, VkMockCommandEXT* pCommand ) {
            CommandData& cmdData = *reinterpret_cast<CommandData*>( pCommand->data.u64 );

            auto nanosecondsSinceEpoch = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch() );

            cmdData.queryPool->m_Timestamps.at( cmdData.query ) = nanosecondsSinceEpoch.count();
        };

        m_Commands.push_back( command );
    }

    void CommandBuffer::vkCmdCopyBuffer( VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions )
    {
        struct CommandData
        {
            VkBuffer srcBuffer;
            VkBuffer dstBuffer;
            VkBufferCopy region;
        };

        static_assert( sizeof( CommandData ) <= sizeof( VkMockCommandEXT::data ),
            "Command data size exceeds VkMockCommandEXT::data size" );

        for( uint32_t i = 0; i < regionCount; ++i )
        {
            VkMockCommandEXT command = {};
            CommandData& cmdData = *reinterpret_cast<CommandData*>( command.data.u64 );
            cmdData.srcBuffer = srcBuffer;
            cmdData.dstBuffer = dstBuffer;
            cmdData.region = pRegions[ i ];

            command.pfnExecute = []( VkQueue queue, VkMockCommandEXT* pCommand ) {
                CommandData& cmdData = *reinterpret_cast<CommandData*>( pCommand->data.u64 );
                memcpy( cmdData.dstBuffer->m_pData + cmdData.region.dstOffset,
                    cmdData.srcBuffer->m_pData + cmdData.region.srcOffset,
                    cmdData.region.size );
            };

            m_Commands.push_back( command );
        }
    }

    void CommandBuffer::vkCmdCopyQueryPoolResults( VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags )
    {
        struct CommandData
        {
            VkQueryPool queryPool;
            uint32_t firstQuery;
            uint32_t queryCount;
            VkBuffer dstBuffer;
            VkDeviceSize dstOffset;
            VkDeviceSize stride;
            VkQueryResultFlags flags;
        };

        static_assert( sizeof( CommandData ) <= sizeof( VkMockCommandEXT::data ),
            "Command data size exceeds VkMockCommandEXT::data size" );

        VkMockCommandEXT command = {};
        CommandData& cmdData = *reinterpret_cast<CommandData*>( command.data.u64 );
        cmdData.queryPool = queryPool;
        cmdData.firstQuery = firstQuery;
        cmdData.queryCount = queryCount;
        cmdData.dstBuffer = dstBuffer;
        cmdData.dstOffset = dstOffset;
        cmdData.stride = stride;
        cmdData.flags = flags;

        command.pfnExecute = []( VkQueue, VkMockCommandEXT* pCommand ) {
            CommandData& cmdData = *reinterpret_cast<CommandData*>( pCommand->data.u64 );
            for( uint32_t i = 0; i < cmdData.queryCount; ++i )
            {
                if( cmdData.flags & VK_QUERY_RESULT_64_BIT )
                {
                    *reinterpret_cast<uint64_t*>( cmdData.dstBuffer->m_pData + cmdData.dstOffset + i * cmdData.stride ) =
                        cmdData.queryPool->m_Timestamps.at( cmdData.firstQuery + i );
                }
                else
                {
                    *reinterpret_cast<uint32_t*>( cmdData.dstBuffer->m_pData + cmdData.dstOffset + i * cmdData.stride ) =
                        static_cast<uint32_t>( cmdData.queryPool->m_Timestamps.at( cmdData.firstQuery + i ) & 0xFFFFFFFF );
                }
            }
        };

        m_Commands.push_back( command );
    }
}
