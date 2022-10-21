#ifndef ASTERIXUDPRECEIVER_H
#define ASTERIXUDPRECEIVER_H

#include "datasourcelineinfo.h"

#include <boost/asio.hpp>

class ASTERIXUDPReceiver
{
public:
    ASTERIXUDPReceiver(boost::asio::io_context& io_context, std::shared_ptr<DataSourceLineInfo> line_info,
                       std::function<void(const char*, unsigned int)> data_callback);


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
    char* data_ {nullptr};
};

#endif // ASTERIXUDPRECEIVER_H
