/***************************************************************************
* Copyright (c) 2020, Martin Renou, Johan Mabille, Sylvain Corlay, and     *
* Wolf Vollprecht                                                          *
* Copyright (c) 2020, QuantStack
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <thread>
#include <chrono>

#include "xheartbeat.hpp"
#include "xutils.hpp"

namespace xena
{
    xheartbeat::xheartbeat(zmq::context_t& context,
                           const std::string& heartbeat_end_point,
                           const std::string& controller_end_point,
                           const to_notifier_t& notifier,
                           int timeout,
                           int max_retry)
        : m_heartbeat(context, zmq::socket_type::dealer)
        , m_controller(context, zmq::socket_type::rep)
        , m_notifier(notifier)
        , m_timeout(timeout)
        , m_max_retry(max_retry)
    {
        m_heartbeat.set(zmq::sockopt::linger, get_socket_linger());
        m_controller.set(zmq::sockopt::linger, get_socket_linger());

        m_heartbeat.connect(heartbeat_end_point);
        m_controller.connect(controller_end_point);
    }

    void xheartbeat::run()
    {
        zmq::pollitem_t items[] = {
            { m_heartbeat, 0, ZMQ_POLLIN, 0 },
            { m_controller, 0, ZMQ_POLLIN, 0 }
        };

        int nb_retry = 0;
        while(true)
        {
            m_heartbeat.send(zmq::message_t("ping", 4), zmq::send_flags::none);
            zmq::poll(&items[0], 2, long(m_timeout));

            if (items[0].revents & ZMQ_POLLIN)
            {
                zmq::multipart_t wire_msg;
                wire_msg.recv(m_heartbeat);
                nb_retry = 0;

                std::chrono::milliseconds sf(m_timeout / 2);
                std::this_thread::sleep_for(sf);
            }
            else if (items[1].revents & ZMQ_POLLIN)
            {
                // stop message
                zmq::multipart_t wire_msg;
                wire_msg.recv(m_controller);
                wire_msg.send(m_controller);
                break;
            }
            else
            {
                // Timeout
                ++nb_retry;
                if (nb_retry == m_max_retry)
                {
                    m_notifier();
                }
            }
        }
    }
}

