/***************************************************************************
* Copyright (c) 2020, QuantStack and xena contributors                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "xzmq_client.hpp"

namespace xena
{
    xzmq_client::xzmq_client(size_t id,
                             zmq::context_t& context,
                             const std::string& control_end_point,
                             const std::string& shell_end_point,
                             const std::string& stdin_end_point,
                             const std::string& iopub_end_point)
        : m_id(id)
        , m_socket()
    {
        size_t control_index = channel_index(channel::control);
        m_socket[control_index] = zmq::socket_t(context, zmq::socket_type::dealer);
        m_socket[control_index].connect(control_end_point);

        size_t shell_index = channel_index(channel::shell);
        m_socket[shell_index] = zmq::socket_t(context, zmq::socket_type::dealer);
        m_socket[shell_index].connect(shell_end_point);

        size_t stdin_index = channel_index(channel::stdin_);
        m_socket[stdin_index] = zmq::socket_t(context, zmq::socket_type::router);
        m_socket[stdin_index].connect(stdin_end_point);

        size_t iopub_index = channel_index(channel::iopub);
        m_socket[iopub_index] = zmq::socket_t(context, zmq::socket_type::sub);
        m_socket[iopub_index].connect(iopub_end_point);
    }

    void xzmq_client::send(zmq::multipart_t& message, channel c)
    {
        message.send(m_socket[channel_index(c)]);
    }

    zmq::multipart_t xzmq_client::receive(channel c)
    {
        zmq::multipart_t message;
        message.recv(m_socket[channel_index(c)]);
        return message;
    }

    size_t xzmq_client::get_id() const
    {
        return m_id;
    }

    xzmq_client::socket_list& xzmq_client::get_sockets()
    {
        return m_socket;
    }
}

