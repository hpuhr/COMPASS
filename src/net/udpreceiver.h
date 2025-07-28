#pragma once

#include "datasourcelineinfo.h"

#include <boost/asio.hpp>

class UDPReceiver
{
public:
    UDPReceiver(boost::asio::io_context& io_context, std::shared_ptr<DataSourceLineInfo> line_info,
                std::function<void(const char*, unsigned int)> data_callback, unsigned int max_read_size);


    void handle_receive_from(const boost::system::error_code& error,
                             size_t bytes_recvd);

private:
    std::shared_ptr<DataSourceLineInfo> line_info_;

    boost::asio::ip::udp::endpoint socket_endpoint_;
    boost::asio::ip::udp::endpoint sender_endpoint_;
    boost::asio::ip::udp::socket socket_;

    bool has_sender_address_ {false};
    boost::asio::ip::address sender_addr_;

    std::function<void(const char*, unsigned int)> data_callback_; // const std::string&,

    unsigned int max_read_size_ {0};
    char* data_ {nullptr};
};
