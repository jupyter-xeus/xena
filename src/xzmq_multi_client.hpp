/***************************************************************************
* Copyright (c) 2020, QuantStack and xena contributors                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XZMQ_MULTI_CLIENT_HPP
#define XZMQ_MULTI_CLIENT_HPP

#include <vector>

#include "xzmq_client.hpp"

namespace xena
{
    class xzmq_multi_client
    {
    public:

        xzmq_multi_client(zmq::context_t& context,
                          const std::string& controller_end_point);

        void run();

    private:

        size_t next_client_id() const;

        size_t get_pollitem_index(size_t client_index) const;

        void add_client(const std::string& shell_end_point,
                        const std::string& control_end_point,
                        const std::string& stdin_end_point,
                        const std::string& iopub_end_point);

        void remove_client(size_t cliend_index);

        void handle_controller_message();
        void handle_kernel_message(size_t client_index);

        zmq::context_t* p_context;
        zmq::socket_t m_controller;
        std::vector<zmq::pollitem_t> m_pollitems;
        std::vector<xzmq_client> m_client_list;
    };
}

#endif
