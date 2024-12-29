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

namespace vkmock
{
    struct Instance : InstanceBase
    {
        VkPhysicalDevice m_PhysicalDevice;

        Instance( const VkInstanceCreateInfo& createInfo );
        ~Instance();

        void vkDestroyInstance( const VkAllocationCallbacks* pAllocator );

        VkResult vkEnumeratePhysicalDevices( uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices );

#ifdef VK_KHR_win32_surface
        VkResult vkCreateWin32SurfaceKHR( const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface );
#endif

#ifdef VK_KHR_surface
        void vkDestroySurfaceKHR( VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator );
#endif
    };
}

struct VkInstance_T : vkmock::Instance
{
    using Instance::Instance;
};
