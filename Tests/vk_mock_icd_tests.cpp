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

#include <gtest/gtest.h>
#include <vulkan/vulkan.h>
#include <vk_mock.h>

struct vk_mock_icd_tests : testing::Test
{
    VkAllocationCallbacks* allocator = nullptr;

    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;

    PFN_vkSetDeviceMockProcAddrEXT vkSetDeviceMockProcAddrEXT = nullptr;
    PFN_vkAppendMockCommandEXT vkAppendMockCommandEXT = nullptr;
    PFN_vkExecuteMockCommandBufferEXT vkExecuteMockCommandBufferEXT = nullptr;

    void TearDown() override
    {
        if( device ) vkDestroyDevice( device, allocator );
        if( instance ) vkDestroyInstance( instance, allocator );
        if( allocator ) free( allocator );
    }

    void CreateAllocator()
    {
        allocator = (VkAllocationCallbacks*)malloc( sizeof( VkAllocationCallbacks ) );
        ASSERT_NE( nullptr, allocator );
        allocator->pfnAllocation = []( void* userData, size_t size, size_t alignment, VkSystemAllocationScope scope ) { return malloc( size ); };
        allocator->pfnFree = []( void* userData, void* memory ) { free( memory ); };
        allocator->pfnReallocation = []( void* userData, void* original, size_t size, size_t alignment, VkSystemAllocationScope scope ) { return realloc( original, size ); };
        allocator->pfnInternalAllocation = nullptr;
        allocator->pfnInternalFree = nullptr;
        allocator->pUserData = nullptr;
    }

    void CreateInstance()
    {
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        VkResult result = vkCreateInstance( &createInfo, allocator, &instance );
        ASSERT_EQ( VK_SUCCESS, result );
    }

    void CreateDevice()
    {
        uint32_t physicalDeviceCount = 1;
        vkEnumeratePhysicalDevices( instance, &physicalDeviceCount, &physicalDevice );
        ASSERT_NE( VK_NULL_HANDLE, physicalDevice );

        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueCount = 1;
        const float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

        VkResult result = vkCreateDevice( physicalDevice, &deviceCreateInfo, allocator, &device );
        ASSERT_EQ( VK_SUCCESS, result );

        vkGetDeviceQueue( device, 0, 0, &queue );
    }

    void LoadMockExtension()
    {
        vkSetDeviceMockProcAddrEXT = (PFN_vkSetDeviceMockProcAddrEXT)vkGetDeviceProcAddr( device, "vkSetDeviceMockProcAddrEXT" );
        ASSERT_NE( nullptr, vkSetDeviceMockProcAddrEXT );

        vkAppendMockCommandEXT = (PFN_vkAppendMockCommandEXT)vkGetDeviceProcAddr( device, "vkAppendMockCommandEXT" );
        ASSERT_NE( nullptr, vkAppendMockCommandEXT );

        vkExecuteMockCommandBufferEXT = (PFN_vkExecuteMockCommandBufferEXT)vkGetDeviceProcAddr( device, "vkExecuteMockCommandBufferEXT" );
        ASSERT_NE( nullptr, vkExecuteMockCommandBufferEXT );
    }
};

static bool mockDestroyDeviceCalled = false;
static void mockDestroyDevice( VkDevice device, const VkAllocationCallbacks* pAllocator )
{
    mockDestroyDeviceCalled = true;
}

static void mockCommand( VkQueue, VkMockCommandEXT* pCommand )
{
    bool* pMockCommandCalled = reinterpret_cast<bool*>( pCommand->data.u64[ 0 ] );
    ( *pMockCommandCalled ) = true;
}

static void mockCommandFree( VkMockCommandEXT* pCommand )
{
    bool* pMockCommandCalled = reinterpret_cast<bool*>( pCommand->data.u64[ 1 ] );
    ( *pMockCommandCalled ) = true;
}

TEST_F( vk_mock_icd_tests, vkCreateInstance )
{
    CreateInstance();
}

TEST_F( vk_mock_icd_tests, vkCreateDevice )
{
    CreateInstance();
    CreateDevice();
}

TEST_F( vk_mock_icd_tests, vkCreateBufferWithAllocator )
{
    CreateAllocator();
    CreateInstance();
    CreateDevice();

    size_t allocatedSize = 0;
    allocator->pUserData = &allocatedSize;
    allocator->pfnAllocation = []( void* userData, size_t size, size_t alignment, VkSystemAllocationScope scope ) {
        size_t* pAllocatedSize = reinterpret_cast<size_t*>( userData );
        ( *pAllocatedSize ) += size;
        return malloc( size );
    };

    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.size = 1024;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkBuffer buffer = VK_NULL_HANDLE;
    VkResult result = vkCreateBuffer( device, &bufferCreateInfo, allocator, &buffer );
    ASSERT_EQ( VK_SUCCESS, result );
    ASSERT_NE( VK_NULL_HANDLE, buffer );
    EXPECT_LT( 0, allocatedSize );

    vkDestroyBuffer( device, buffer, allocator );
}

TEST_F( vk_mock_icd_tests, vkSetDeviceMockProcAddrEXT )
{
    CreateInstance();
    CreateDevice();
    LoadMockExtension();

    mockDestroyDeviceCalled = false;
    vkSetDeviceMockProcAddrEXT( device, "vkDestroyDevice", (PFN_vkVoidFunction)mockDestroyDevice );

    vkDestroyDevice( device, nullptr );
    EXPECT_TRUE( mockDestroyDeviceCalled );

    device = VK_NULL_HANDLE;
}

TEST_F( vk_mock_icd_tests, vkAppendMockCommandEXT )
{
    CreateInstance();
    CreateDevice();
    LoadMockExtension();

    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = 0;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkResult result = vkCreateCommandPool( device, &commandPoolCreateInfo, nullptr, &commandPool );
    ASSERT_EQ( VK_SUCCESS, result );

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    result = vkAllocateCommandBuffers( device, &commandBufferAllocateInfo, &commandBuffer );
    ASSERT_EQ( VK_SUCCESS, result );

    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    result = vkBeginCommandBuffer( commandBuffer, &commandBufferBeginInfo );
    ASSERT_EQ( VK_SUCCESS, result );

    bool mockCommandCalled = false;
    bool mockFreeCalled = false;

    VkMockCommandEXT command = {};
    command.data.u64[ 0 ] = reinterpret_cast<uintptr_t>( &mockCommandCalled );
    command.data.u64[ 1 ] = reinterpret_cast<uintptr_t>( &mockFreeCalled );
    command.pfnExecute = &mockCommand;
    command.pfnFree = &mockCommandFree;
    vkAppendMockCommandEXT( commandBuffer, &command );

    result = vkEndCommandBuffer( commandBuffer );
    ASSERT_EQ( VK_SUCCESS, result );

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    result = vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE );
    ASSERT_EQ( VK_SUCCESS, result );

    vkQueueWaitIdle( queue );

    EXPECT_TRUE( mockCommandCalled );
    EXPECT_FALSE( mockFreeCalled );

    vkResetCommandBuffer( commandBuffer, 0 );

    EXPECT_TRUE( mockFreeCalled );
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
