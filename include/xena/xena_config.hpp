/***************************************************************************
* Copyright (c) 2020, Martin Renou, Johan Mabille, Sylvain Corlay, and     *
* Wolf Vollprecht                                                          *
* Copyright (c) 2020, QuantStack
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XENA_CONFIG_HPP
#define XENA_CONFIG_HPP

// Project version
#define XENA_VERSION_MAJOR 0
#define XENA_VERSION_MINOR 0
#define XENA_VERSION_PATCH 1

#ifdef _WIN32
    #ifdef XENA_EXPORTS
        #define XENA_API __declspec(dllexport)
    #else
        #define XENA_API __declspec(dllimport)
    #endif
#else
    #define XENA_API
#endif

#endif
