/***************************************************************************
* Copyright (c) 2020, QuantStack and xena contributors                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XENA_CHANNEL_HPP
#define XENA_CHANNEL_HPP

#include <array>
#include <cstddef>
#include <string>

#include "xenum_iterator.hpp"

namespace xena
{
    enum class channel
    {
        control = 0,
        shell,
        stdin_,
        iopub,
        last,
        first = control
    };

    std::size_t channel_index(channel c);
    const std::string& channel_name(channel c);

    /******************
     * Implementation *
     ******************/

    inline std::size_t channel_index(channel c)
    {
        return static_cast<std::size_t>(c);
    }

    inline const std::string& channel_name(channel c)
    {
        static std::array<std::string, 4> names = {{ "control", "shell", "stdin_", "iopub" }};
        return names[channel_index(c)];
    }
}

#endif

