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

#include "vk_mock_icd.h"
#include "vk_mock_icd_helpers.h"
#include "vk_mock_instance.h"
#include "vk_mock_device.h"
#include "vk_mock_physical_device.h"
#include "vk_mock_command_buffer.h"
#include "vk_mock_queue.h"
#undef VK_NO_PROTOTYPES
#include "vk_mock.h"

#include "vk_mock_icd_dispatch.h"

PFN_vkVoidFunction vk_icdGetInstanceProcAddr(
    VkInstance,
    const char* pName )
{
    if( !strcmp( "vkGetInstanceProcAddr", pName ) ) return reinterpret_cast<PFN_vkVoidFunction>( vk_icdGetInstanceProcAddr );
    if( !strcmp( "vkGetDeviceProcAddr", pName ) ) return reinterpret_cast<PFN_vkVoidFunction>( vk_icdGetInstanceProcAddr );

#ifdef VK_EXT_mock
    if( !strcmp( "vkSetDeviceMockProcAddrEXT", pName ) ) return reinterpret_cast<PFN_vkVoidFunction>( vkSetDeviceMockProcAddrEXT );
    if( !strcmp( "vkAppendMockCommandEXT", pName ) ) return reinterpret_cast<PFN_vkVoidFunction>( vkAppendMockCommandEXT );
    if( !strcmp( "vkExecuteMockCommandBufferEXT", pName ) ) return reinterpret_cast<PFN_vkVoidFunction>( vkExecuteMockCommandBufferEXT );
#endif // VK_EXT_mock

    return vkGetInstanceProcAddr( nullptr, pName );
}

VkResult vk_icdNegotiateLoaderICDInterfaceVersion(
    uint32_t* pSupportedVersion )
{
    if( *pSupportedVersion >= 5 )
    {
        *pSupportedVersion = 5;
        return VK_SUCCESS;
    }

    if( *pSupportedVersion >= 2 )
    {
        *pSupportedVersion = 2;
        return VK_SUCCESS;
    }

    return VK_ERROR_INCOMPATIBLE_DRIVER;
}

VkResult vkCreateInstance(
    const VkInstanceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkInstance* pInstance )
{
    return vkmock::vk_new(
        pInstance,
        vkmock::vk_allocator( pAllocator, vkmock::g_DefaultAllocator ),
        VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE,
        *pCreateInfo );
}

VkResult vkEnumerateInstanceVersion(
    uint32_t* pApiVersion )
{
    *pApiVersion = VK_API_VERSION_1_3;
    return VK_SUCCESS;
}

VkResult vkEnumerateInstanceLayerProperties(
    uint32_t* pPropertyCount,
    VkLayerProperties* pProperties )
{
    *pPropertyCount = 0;
    return VK_SUCCESS;
}

VkResult vkEnumerateInstanceExtensionProperties(
    const char* pLayerName,
    uint32_t* pPropertyCount,
    VkExtensionProperties* pProperties )
{
    const VkExtensionProperties properties[] = {
#ifdef VK_KHR_surface
        { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_SURFACE_SPEC_VERSION },
#endif
#ifdef VK_KHR_win32_surface
        { VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_SPEC_VERSION },
#endif
#ifdef VK_KHR_get_physical_device_properties2
        { VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_SPEC_VERSION },
#endif
    };

    const uint32_t propertyCount = std::size( properties );

    if( !pProperties )
    {
        *pPropertyCount = propertyCount;
        return VK_SUCCESS;
    }

    const uint32_t count = std::min( *pPropertyCount, propertyCount );
    for( uint32_t i = 0; i < count; ++i )
    {
        pProperties[ i ] = properties[ i ];
    }

    *pPropertyCount = count;

    if( count < propertyCount )
    {
        return VK_INCOMPLETE;
    }

    return VK_SUCCESS;
}

void vkSetInstanceMockProcAddrEXT(
    VkInstance instance,
    const char* pName,
    PFN_vkVoidFunction pFunction )
{
    if( instance->m_pMockFunctions )
    {
        instance->m_pMockFunctions->SetProcAddr( pName, pFunction );
    }
}

void vkSetDeviceMockProcAddrEXT(
    VkDevice device,
    const char* pName,
    PFN_vkVoidFunction pFunction )
{
    if( device->m_pMockFunctions )
    {
        device->m_pMockFunctions->SetProcAddr( pName, pFunction );
    }
}

void vkAppendMockCommandEXT(
    VkCommandBuffer commandBuffer,
    const VkMockCommandEXT* pCommand )
{
    commandBuffer->m_Commands.push_back( *pCommand );
}

void vkExecuteMockCommandBufferEXT(
    VkQueue queue,
    VkCommandBuffer commandBuffer )
{
    queue->ExecuteCommandBuffer( commandBuffer );
}
