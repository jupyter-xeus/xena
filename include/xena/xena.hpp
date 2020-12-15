/***************************************************************************
* Copyright (c) 2020, Martin Renou, Johan Mabille, Sylvain Corlay, and     *
* Wolf Vollprecht                                                          *
* Copyright (c) 2020, QuantStack
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XENA_HPP
#define XENA_HPP

#ifdef _WIN32
    #ifdef XENA_EXPORTS
        #define XENA_API __declspec(dllexport)
    #else
        #define XENA_API __declspec(dllimport)
    #endif
#else
    #define XENA_API
#endif


// Project version
#define XENA_VERSION_MAJOR 0
#define XENA_VERSION_MINOR 0
#define XENA_VERSION_PATCH 1

// Binary version
#define XENA_BINARY_CURRENT 1
#define XENA_BINARY_REVISION 0
#define XENA_BINARY_AGE 0

// Kernel protocol version
#define XENA_KERNEL_PROTOCOL_VERSION_MAJOR 5
#define XENA_KERNEL_PROTOCOL_VERSION_MINOR 3

// Composing the protocol version string from major, and minor
#define XENA_CONCATENATE(A, B) XENA_CONCATENATE_IMPL(A, B)
#define XENA_CONCATENATE_IMPL(A, B) A##B
#define XENA_STRINGIFY(a) XENA_STRINGIFY_IMPL(a)
#define XENA_STRINGIFY_IMPL(a) #a

#define XENA_KERNEL_PROTOCOL_VERSION XENA_STRINGIFY(XENA_CONCATENATE(    XENA_KERNEL_PROTOCOL_VERSION_MAJOR,\
                                                    XENA_CONCATENATE(.,  XENA_KERNEL_PROTOCOL_VERSION_MINOR)))

#endif

