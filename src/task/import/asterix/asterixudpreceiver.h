#ifndef ASTERIXUDPRECEIVER_H
#define ASTERIXUDPRECEIVER_H

#include <boost/asio.hpp>

class ASTERIXUDPReceiver
{
public:
    ASTERIXUDPReceiver(boost::asio::io_context& io_context, const std::string& sender_ip, unsigned int port,
                       std::function<void(const char*, unsigned int)> data_callback);


    void handle_receive_from(const boost::system::error_code& error,
                             size_t bytes_recvd);

private:
    boost::asio::ip::udp::endpoint socket_endpoint_;
    boost::asio::ip::udp::endpoint sender_endpoint_;
    boost::asio::ip::udp::socket socket_;

    std::function<void(const char*, unsigned int)> data_callback_; // const std::string&,
    char* data_ {nullptr};
};

#endif // ASTERIXUDPRECEIVER_H
