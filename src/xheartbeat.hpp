/***************************************************************************
* Copyright (c) 2020, QuantStack and xena contributors                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XENA_HEARTBEAT_HPP
#define XENA_HEARTBEAT_HPP

#include "zmq_addon.hpp"

namespace xena
{
    class xheartbeat
    {
    public:

        xheartbeat(zmq::context_t& context,
                   const std::string& heartbeat_end_point,
                   const std::string& controller_end_point,
                   int timeout,
                   int max_retry);

        void run();

    private:

        zmq::socket_t m_heartbeat;
        zmq::socket_t m_controller;
        int m_timeout;
        int m_max_retry;
    };
}

#endif

