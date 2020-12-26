/***************************************************************************
* Copyright (c) 2020, QuantStack and xena contributors                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "nlohmann/json.hpp"

#include "xeus/xmiddleware.hpp"

#include "xutils.hpp"
#include "xzmq_multi_client.hpp"

namespace nl = nlohmann;

namespace xena
{
    namespace
    {
        constexpr size_t ITEM_OFFSET = 1;
    }

    xzmq_multi_client::xzmq_multi_client(zmq::context_t& context,
                                         const std::string& controller_end_point)
        : p_context(&context)
        , m_controller(context, zmq::socket_type::router)
    {
        m_controller.set(zmq::sockopt::linger, get_socket_linger());
        m_controller.bind(controller_end_point);
        zmq::pollitem_t item = { m_controller, 0, ZMQ_POLLIN, 0 };
        m_pollitems.push_back(std::move(item));
    }

    void xzmq_multi_client::run()
    {
        while (true)
        {
            zmq::poll(m_pollitems.data(), m_pollitems.size(), -1);
            
            if (m_pollitems[0].revents & ZMQ_POLLIN)
            {
                handle_controller_message();
            }
            for (size_t i = 0; i < m_client_list.size(); ++i)
            {
                handle_kernel_message(i);
            }
        }
    }

    size_t xzmq_multi_client::next_client_id() const
    {
        static size_t client_id = 0;
        return client_id++;
    }

    size_t xzmq_multi_client::get_pollitem_index(size_t client_index) const
    {
        return ITEM_OFFSET + client_index * size_t(channel::last);
    }

    size_t xzmq_multi_client::add_client(const std::string& control_end_point,
                                         const std::string& shell_end_point,
                                         const std::string& stdin_end_point,
                                         const std::string& iopub_end_point)
    {
        m_client_list.push_back(xzmq_client(next_client_id(),
                                            *p_context,
                                            control_end_point,
                                            shell_end_point,
                                            stdin_end_point,
                                            iopub_end_point));

        xzmq_client::socket_list& sockets = m_client_list.back().get_sockets();
        std::transform(sockets.begin(), sockets.end(), std::back_inserter(m_pollitems),
                       [](auto& s) -> zmq::pollitem_t
        {
            return { s, 0, ZMQ_POLLIN, 0 };
        });
        return m_client_list.back().get_id();
    }

    void xzmq_multi_client::remove_client(ptrdiff_t i)
    {
        m_client_list.erase(m_client_list.begin() + i);
        auto it = m_pollitems.begin() + static_cast<ptrdiff_t>(get_pollitem_index(i));
        auto it_end = it + static_cast<ptrdiff_t>(channel::last);
        m_pollitems.erase(it, it_end);
    }

    void xzmq_multi_client::handle_controller_message()
    {
        zmq::message_t message;
        m_controller.recv(message, zmq::recv_flags::none);
        const char* buffer = message.data<const char>();
        nl::json j = nl::json::parse(buffer, buffer + message.size());

        if (j["command"] == "add")
        {
            std::string transport = j["transport"].get<std::string>();
            std::string ip = j["ip"].get<std::string>();
            std::string control_port = j["control_port"].get<std::string>();
            std::string shell_port = j["shell_port"].get<std::string>();
            std::string stdin_port = j["stdin_port"].get<std::string>();
            std::string iopub_port = j["ioput_port"].get<std::string>();
            size_t id = add_client(xeus::get_end_point(transport, ip, control_port),
                                   xeus::get_end_point(transport, ip, shell_port),
                                   xeus::get_end_point(transport, ip, stdin_port),
                                   xeus::get_end_point(transport, ip, iopub_port));
            nl::json jrep =
            {
                {"type", "response"},
                {"command", "add"},
                {"client_id", id}
            };
            std::string buffer_rep = jrep.dump(-1, ' ', false);
            zmq::message_t rep(buffer_rep.c_str(), buffer_rep.size());
            m_controller.send(rep, zmq::send_flags::none);
        }
        else if (j["command"] == "remove")
        {
            size_t id = j["client_id"].get<size_t>();
            auto it = std::find_if(m_client_list.begin(), m_client_list.end(),
                                   [id](const auto& c) { return c.get_id() == id; });
            if (it != m_client_list.end())
            {
                remove_client(it - m_client_list.begin());
                nl::json jrep = 
                {
                    {"type", "response"},
                    {"command", "remove"},
                    {"client_id", id}
                };
                std::string buffer_rep = jrep.dump(-1, ' ', false);
                zmq::message_t rep(buffer_rep.c_str(), buffer_rep.size());
                m_controller.send(rep, zmq::send_flags::none);
            }
        }
    }

    void xzmq_multi_client::handle_kernel_message(size_t i)
    {
        zmq::pollitem_t* pollitem = &m_pollitems[get_pollitem_index(i)];
        for (auto c: make_enum_range<channel>())
        {
            if (pollitem[channel_index(c)].revents & ZMQ_POLLIN)
            {
                zmq::multipart_t msg = m_client_list[i].receive(c);
                msg.push_back(zmq::message_t(std::string("{channel: ") + channel_name(c) + "}"));
                msg.push_back(zmq::message_t(std::to_string(m_client_list[i].get_id())));
                msg.send(m_controller);
            }
        }
    }
}

