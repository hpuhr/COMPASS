#include "asterixudpreceiver.h"
#include "asterixdecodejob.h"
#include "util/files.h"
#include "logger.h"
#include "stringconv.h"

#include <boost/bind.hpp>

using namespace Utils;
using namespace std;

ASTERIXUDPReceiver::ASTERIXUDPReceiver(boost::asio::io_context& io_context, const std::string& sender_ip, unsigned int port,
                                       std::function<void(const char*, unsigned int)> data_callback)
    : socket_endpoint_(boost::asio::ip::address::from_string(sender_ip), port),
      socket_(io_context),
      data_callback_(data_callback)
{
    // udp::endpoint(boost::asio::ip::address_v4::any(), port)

    data_ = new char[MAX_UDP_READ_SIZE];

    //loginf << "ctor: " << sender_ip_ << ":" << port_;
    socket_.open(socket_endpoint_.protocol());

    socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    socket_.bind(socket_endpoint_);

    socket_.set_option(boost::asio::ip::multicast::join_group(
                           boost::asio::ip::address::from_string(sender_ip)));


    //        std::string address_listen = "1.2.3.4";
    //        std::string address_mcast = "224.0.0.0";
    //        boost::system::error_code ec;
    //        boost::asio::ip::address listen_addr = boost::asio::ip::address::from_string(address_listen, ec);
    //        boost::asio::ip::address mcast_addr = boost::asio::ip::address::from_string(address_mcast, ec);
    //        socket_.set_option(boost::asio::ip::multicast::join_group(mcast_addr.to_v4(), listen_addr.to_v4()), ec);

    socket_.async_receive_from(
                boost::asio::buffer(data_, MAX_UDP_READ_SIZE), sender_endpoint_,
                boost::bind(&ASTERIXUDPReceiver::handle_receive_from, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
}

void ASTERIXUDPReceiver::handle_receive_from(const boost::system::error_code& error,
                                             size_t bytes_recvd)
{
    //loginf << "handle_receive_from: from " << sender_ip_ << ":" << port_ << " bytes " << bytes_recvd;

    if (error && error != boost::asio::error::message_size)
    {
        logerr << "UDPReceiver: handle_receive_from: from "
               << sender_endpoint_.address().to_string()+":"+to_string(sender_endpoint_.port())
               << " error " << error;
        return;
    }
    else
    {
        //sender_endpoint_.address().to_string()+":"+to_string(sender_endpoint_.port()),
        data_callback_(data_, bytes_recvd);
    }

    //sender_endpoint_.address() should be set to sender ip

    socket_.async_receive_from(
                boost::asio::buffer(data_, MAX_UDP_READ_SIZE), sender_endpoint_,
                boost::bind(&ASTERIXUDPReceiver::handle_receive_from, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
}
