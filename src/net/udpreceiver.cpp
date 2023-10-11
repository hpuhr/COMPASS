#include "udpreceiver.h"
//#include "util/files.h"
#include "logger.h"
//#include "stringconv.h"

#include <boost/bind.hpp>

//using namespace Utils;
using namespace std;

UDPReceiver::UDPReceiver(boost::asio::io_context& io_context, //const std::string& sender_ip, unsigned int port,
                         std::shared_ptr<DataSourceLineInfo> line_info,
                         std::function<void(const char*, unsigned int)> data_callback, unsigned int max_read_size)
    : line_info_(line_info), socket_(io_context),
      data_callback_(data_callback), max_read_size_(max_read_size)
{
    // udp::endpoint(boost::asio::ip::address_v4::any(), port)

    //    std::string address_listen = "1.2.3.4";
    //    std::string address_mcast = "224.0.0.0";
    //    unsigned short address_port = 50000;
    //    boost::system::error_code ec;
    boost::system::error_code ec;

    //    boost::asio::ip::address mcast_addr = boost::asio::ip::address::from_string(address_mcast, ec);
    boost::asio::ip::address mcast_addr = boost::asio::ip::address::from_string(line_info_->mcastIP(), ec);

    if (ec)
    {
        logerr << "UDPReceiver: ctor: mcast address error " << ec.message();
        return;
    }

    assert (max_read_size_ > 1024);
    data_ = new char[max_read_size_];

    //    boost::asio::ip::address listen_addr = boost::asio::ip::address::from_string(address_listen, ec);
    bool has_listen_address = line_info_->hasListenIP();
    boost::asio::ip::address listen_addr;

    if (has_listen_address)
    {
        listen_addr = boost::asio::ip::address::from_string(line_info_->listenIP(), ec);

        if (ec)
        {
            logerr << "UDPReceiver: ctor: listen address error " << ec.message();
            return;
        }

        //socket_endpoint_ = boost::asio::ip::udp::endpoint (listen_addr, line_info_->mcastPort());
    }

    //    boost::asio::ip::udp::endpoint listen_endpoint(mcast_addr, address_port);
    socket_endpoint_ = boost::asio::ip::udp::endpoint (mcast_addr, line_info_->mcastPort()); // always join mcast

    //    socket.open(listen_endpoint.protocol(), ec);
    socket_.open(socket_endpoint_.protocol(), ec);
    if (ec)
    {
        logerr << "UDPReceiver: ctor: socket error " << ec.message();
        return;
    }

    //    socket.set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);
    socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);
    if (ec)
    {
        logerr << "UDPReceiver: ctor: socket reuse error " << ec.message();
        return;
    }

    //    socket.bind(listen_endpoint, ec);
    socket_.bind(socket_endpoint_, ec);
    if (ec)
    {
        logerr << "UDPReceiver: ctor: socket bind error " << ec.message();
        return;
    }

    //    socket.set_option(boost::asio::ip::multicast::join_group(mcast_addr.to_v4(), listen_addr.to_v4()), ec);

    if (mcast_addr.is_multicast())
    {
        if (has_listen_address)
            socket_.set_option(boost::asio::ip::multicast::join_group(
                                   mcast_addr.to_v4(), listen_addr.to_v4()), ec);
        else
            socket_.set_option(boost::asio::ip::multicast::join_group(
                                   mcast_addr.to_v4()), ec);
        if (ec)
        {
            logerr << "UDPReceiver: ctor: socket join group error " << ec.message();
            return;
        }
    }

    if (line_info_->hasSenderIP())
    {
        sender_addr_ = boost::asio::ip::address::from_string(line_info_->senderIP(), ec);

        if (ec)
        {
            logerr << "UDPReceiver: ctor: sender address error " << ec.message();
            return;
        }

        has_sender_address_ = true;
    }

    socket_.async_receive_from(
                boost::asio::buffer(data_, max_read_size_), sender_endpoint_,
                boost::bind(&UDPReceiver::handle_receive_from, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
}

void UDPReceiver::handle_receive_from(const boost::system::error_code& error,
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
        if (has_sender_address_) // check
        {
            //sender_endpoint_.address().to_string()+":"+to_string(sender_endpoint_.port()),
            if (sender_endpoint_.address() == sender_addr_)
                data_callback_(data_, bytes_recvd);
        }
        else // accept all
            data_callback_(data_, bytes_recvd);
    }

    //sender_endpoint_.address() should be set to sender ip

    socket_.async_receive_from(
                boost::asio::buffer(data_, max_read_size_), sender_endpoint_,
                boost::bind(&UDPReceiver::handle_receive_from, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
}
