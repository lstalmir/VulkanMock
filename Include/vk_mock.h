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

#ifndef VK_EXT_mock
#define VK_EXT_mock 1
#define VK_EXT_MOCK_EXTENSION_NAME "VK_EXT_mock"
#define VK_EXT_MOCK_SPEC_VERSION 1

#include <vulkan/vulkan.h>

union VkMockCommandDataEXT
{
    uint64_t u64[ 8 ];
    uint32_t u32[ 16 ];
    uint8_t u8[ 64 ];
    void* ptr;
};

typedef void( VKAPI_PTR* PFN_vkExecuteMockCommandCallbackEXT )( VkQueue queue, struct VkMockCommandEXT* pCommand );
typedef void( VKAPI_PTR* PFN_vkFreeMockCommandCallbackEXT )( struct VkMockCommandEXT* pCommand );

struct VkMockCommandEXT
{
    PFN_vkExecuteMockCommandCallbackEXT pfnExecute;
    PFN_vkFreeMockCommandCallbackEXT pfnFree;
    VkMockCommandDataEXT data;
};

typedef void( VKAPI_PTR* PFN_vkSetDeviceMockProcAddrEXT )( VkDevice device, const char* pName, PFN_vkVoidFunction pFunction );
typedef void( VKAPI_PTR* PFN_vkAppendMockCommandEXT )( VkCommandBuffer commandBuffer, const VkMockCommandEXT* pCommand );
typedef void( VKAPI_PTR* PFN_vkExecuteMockCommandBufferEXT )( VkQueue queue, VkCommandBuffer commandBuffer );

#ifndef VK_NO_PROTOTYPES
/**
 * @brief
 *   Set function to be called when a mock function is called.
 * @param device
 *   The device to set the mock function for.
 * @param pName
 *   The name of the function to set the mock function for.
 * @param pFunction
 *   The mock function to call instead of the real function.
 */
VKAPI_ATTR void VKAPI_CALL vkSetDeviceMockProcAddrEXT(
    VkDevice device,
    const char* pName,
    PFN_vkVoidFunction pFunction );

/**
 * @brief
 *   Append a mock command to the command buffer.
 * @param commandBuffer
 *   The command buffer to append the command to.
 * @param pCommand
 *   The command to append to the command buffer.
 */
VKAPI_ATTR void VKAPI_CALL vkAppendMockCommandEXT(
    VkCommandBuffer commandBuffer,
    const VkMockCommandEXT* pCommand );

/**
 * @brief
 *   Execute a command buffer with mock commands.
 * @param
 *   The queue to execute the command buffer on.
 * @param commandBuffer
 *   The command buffer to execute.
 */
VKAPI_ATTR void VKAPI_CALL vkExecuteMockCommandBufferEXT(
    VkQueue queue,
    VkCommandBuffer commandBuffer );

#endif // VK_NO_PROTOTYPES

#endif // VK_EXT_mock
