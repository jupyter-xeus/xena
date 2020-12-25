/***************************************************************************
* Copyright (c) 2020, QuantStack and xena contributors                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XZMQ_CLIENT_HPP
#define XZMQ_CLIENT_HPP

#include <array>
#include <functional>
#include <string>

#include "zmq_addon.hpp"

#include "xena/xchannel.hpp"

namespace xena
{
    class xzmq_client
    {
    public:

        using socket_list = std::array<zmq::socket_t, static_cast<size_t>(channel::last)>;

        xzmq_client(size_t id,
                    zmq::context_t& context,
                    const std::string& control_end_point,
                    const std::string& shell_end_point,
                    const std::string& stdin_end_point,
                    const std::string& iopub_end_point);

        void send(zmq::multipart_t& message, channel c);
        zmq::multipart_t receive(channel c);

        size_t get_id() const;
        socket_list& get_sockets();

    private:

        size_t m_id;
        socket_list m_socket;
    };
}

#endif

