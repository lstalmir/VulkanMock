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

#include "vk_mock_instance.h"
#include "vk_mock_physical_device.h"
#include "vk_mock_surface.h"
#include "vk_mock_icd_helpers.h"

namespace vkmock
{
    Instance::Instance( const VkInstanceCreateInfo& createInfo )
        : m_Allocator( g_CurrentAllocator )
        , m_PhysicalDevice( nullptr )
    {
        try
        {
            vk_check( vk_new(
                &m_PhysicalDevice,
                m_Allocator,
                VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE,
                GetApiHandle() ) );
        }
        catch( ... )
        {
            Instance::~Instance();
            throw;
        }
    }

    Instance::~Instance()
    {
        vk_delete( m_PhysicalDevice, g_CurrentAllocator );
    }

    void Instance::vkDestroyInstance( const VkAllocationCallbacks* pAllocator )
    {
        vk_delete( this,
            vk_allocator( pAllocator, m_Allocator ) );
    }

    VkResult Instance::vkEnumeratePhysicalDevices( uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices )
    {
        if( pPhysicalDevices == nullptr )
        {
            *pPhysicalDeviceCount = 1;
            return VK_SUCCESS;
        }

        if( *pPhysicalDeviceCount < 1 )
        {
            return VK_INCOMPLETE;
        }

        *pPhysicalDevices = m_PhysicalDevice;

        return VK_SUCCESS;
    }

#ifdef VK_KHR_win32_surface
    VkResult Instance::vkCreateWin32SurfaceKHR( const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface )
    {
        return vk_new(
            pSurface,
            vk_allocator( pAllocator, m_Allocator ),
            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT );
    }
#endif

#ifdef VK_KHR_xlib_surface
    VkResult Instance::vkCreateXlibSurfaceKHR( const VkXlibSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface )
    {
        return vk_new(
            pSurface,
            vk_allocator( pAllocator, m_Allocator ),
            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT );
    }
#endif

#ifdef VK_KHR_xcb_surface
    VkResult Instance::vkCreateXcbSurfaceKHR( const VkXcbSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface )
    {
        return vk_new(
            pSurface,
            vk_allocator( pAllocator, m_Allocator ),
            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT );
    }
#endif

#ifdef VK_KHR_wayland_surface
    VkResult Instance::vkCreateWaylandSurfaceKHR( const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface )
    {
        return vk_new(
            pSurface,
            vk_allocator( pAllocator, m_Allocator ),
            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT );
    }
#endif

#ifdef VK_KHR_android_surface
    VkResult Instance::vkCreateAndroidSurfaceKHR( const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface )
    {
        return vk_new(
            pSurface,
            vk_allocator( pAllocator, m_Allocator ),
            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT );
    }
#endif

#ifdef VK_KHR_surface
    void Instance::vkDestroySurfaceKHR( VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator )
    {
        vk_delete( surface,
            vk_allocator( pAllocator, m_Allocator ) );
    }
#endif
}
