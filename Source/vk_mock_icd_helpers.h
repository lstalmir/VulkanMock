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
#include <vulkan/vulkan.h>
#include <vulkan/vk_icd.h>
#include <memory>

namespace vkmock
{
    extern const VkAllocationCallbacks g_DefaultAllocator;
    extern thread_local VkAllocationCallbacks g_CurrentAllocator;

    inline const VkAllocationCallbacks& vk_allocator(
        const VkAllocationCallbacks* pAllocator,
        const VkAllocationCallbacks& fallbackAllocator )
    {
        return pAllocator ? *pAllocator : fallbackAllocator;
    }

    template<typename T, typename... Args>
    inline VkResult vk_new( T** ptr, const VkAllocationCallbacks& allocator, VkSystemAllocationScope scope, Args&&... args ) noexcept
    {
        VkResult result = VK_SUCCESS;
        VkAllocationCallbacks previousAllocator =
            std::exchange( g_CurrentAllocator, allocator );

        ( *ptr ) = nullptr;
        try
        {
            // Allocate memory for the new object.
            ( *ptr ) = static_cast<T*>( allocator.pfnAllocation(
                allocator.pUserData,
                sizeof( T ),
                alignof( T ),
                scope ) );

            if( !( *ptr ) )
            {
                result = VK_ERROR_OUT_OF_HOST_MEMORY;
            }
            else
            {
                // Invoke constructor.
                new( *ptr ) T( std::forward<Args>( args )... );
            }
        }
        catch( const std::bad_alloc& )
        {
            result = VK_ERROR_OUT_OF_HOST_MEMORY;
        }
        catch( VkResult result_ )
        {
            result = result_;
        }
        catch( ... )
        {
            result = VK_ERROR_UNKNOWN;
        }

        if( result != VK_SUCCESS && ( *ptr ) )
        {
            // Free memory.
            allocator.pfnFree( allocator.pUserData, *ptr );
            ( *ptr ) = nullptr;
        }

        g_CurrentAllocator = previousAllocator;

        return result;
    }

    template<typename T>
    inline void vk_delete( T* ptr, const VkAllocationCallbacks& allocator ) noexcept
    {
        if( ptr )
        {
            VkAllocationCallbacks previousAllocator =
                std::exchange( g_CurrentAllocator, allocator );

            try
            {
                // Invoke destructor.
                ptr->~T();

                // Free memory.
                allocator.pfnFree( allocator.pUserData, ptr );
            }
            catch( ... )
            {
            }

            g_CurrentAllocator = previousAllocator;
        }
    }

    inline void vk_check( VkResult result )
    {
        if( result != VK_SUCCESS )
        {
            throw result;
        }
    }

    template<typename T>
    struct vk_stl_allocator
    {
        using value_type = T;

        VkAllocationCallbacks m_Allocator = g_DefaultAllocator;

        vk_stl_allocator() = default;
        vk_stl_allocator( const VkAllocationCallbacks& allocator )
            : m_Allocator( allocator )
        {}

        template<typename U>
        vk_stl_allocator( const vk_stl_allocator<U>& other )
            : m_Allocator( other.m_Allocator )
        {}

        T* allocate( size_t n )
        {
            return static_cast<T*>( m_Allocator.pfnAllocation(
                m_Allocator.pUserData,
                n * sizeof( T ),
                alignof( T ),
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT ) );
        }

        void deallocate( T* p, size_t )
        {
            m_Allocator.pfnFree( m_Allocator.pUserData, p );
        }
    };
}
