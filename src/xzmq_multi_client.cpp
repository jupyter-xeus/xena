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

    auto xzmq_multi_client::next_client_id() const -> client_id_t
    {
        static client_id_t client_id = 0;
        return client_id++;
    }

    size_t xzmq_multi_client::get_pollitem_index(size_t client_index) const
    {
        return ITEM_OFFSET + client_index * size_t(channel::last);
    }

    auto xzmq_multi_client::add_client(const std::string& control_end_point,
                                       const std::string& shell_end_point,
                                       const std::string& stdin_end_point,
                                       const std::string& iopub_end_point) -> client_id_t
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
        cient_id_t id = m_client_list.back().get_id();
        m_client_index_map.insert(id, m_client_list.size() - 1);
        return id;
    }

    bool xzmq_multi_client::remove_client(client_id_t id)
    {
        bool res = false;
        auto iter = m_client_index_map.find(id);
        if (iter != m_client_index_map.end())
        {
            size_t index = it->second;
            m_client_index_map.erase(it);
            m_client_list.erase(m_client_list.begin() + index);
            auto poll_it = m_pollitems.begin() + static_cast<ptrdiff_t>(get_pollitem_index(index));
            auto poll_it_end = it + static_cast<ptrdiff_t>(channel::last);
            m_pollitems.erase(poll_it,  poll_it_end);
            for (size_t i = index; i < m_client_list.size(); ++i)
            {
                m_client_index_map[m_client_list[i].get_id()] = i;
            }
            res = true;
        }
        return res;
    }

    void xzmq_multi_client::handle_controller_message()
    {
        zmq::multipart_t wire_msg;
        wire_msg.recv(m_controller);

        zmq::message_t command = wire_msg.pop();
        const char* buffer = command.data<const char>();
        nl::json j = nl::json::parse(buffer, buffer + command.size());

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
            nl::json jrep = 
            {
                {"type", "response"},
                {"command", "remove"},
                {"client_id", id}
            };
            if (remove_client(id))
            {
                jrep["success"] = true;
            }
            else
            {
                jrep["success"] = false;
            }
            std::string buffer_rep = jrep.dump(-1, ' ', false);
            zmq::message_t rep(buffer_rep.c_str(), buffer_rep.size());
            m_controller.send(rep, zmq::send_flags::none);
        }
        else if (j["command"] = "forward"])
        {
            size_t id = j["client_id"].get<size_t>();
            channel c = from_channel_name(j["channel_name"]);
            auto iter = m_client_index_map.find(id);
            nl::json jrep =
            {
                {"type", "response"},
                {"command", "forward"},
                {"success", false}
            };
            if (iter != m_client_index_map.end())
            {
                m_client_list[iter->second].send(wire_msg, c);
                jrep["success"] = true;
            }
            std::string buffer_rep = jrep.dump(-1, ' ', false);
            zmq::message_t rep(buffer_rep.c_str(), buffer_rep.size());
            m_controller.send(rep, zmq::send_flags::none);
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
                zmq::multipart_t to_send;
                to_send.push_back(zmq::message_t(std::string("{channel: ") + channel_name(c) + "}"));
                to_send.push_back(zmq::message_t(std::to_string(m_client_list[i].get_id())));
                to_send.push_back(msg.pop()); // Signature
                to_send.push_back(msg.pop()); // Header
                to_send.push_back(msg.pop()); // Parent Header
                to_send.push_back(msg.pop()); // Metadata
                to_send.push_back(msg.pop()); // Content
                to_send.send(m_controller);
            }
        }
    }
}

