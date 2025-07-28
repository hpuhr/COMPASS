#pragma once

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

#include <memory>

class TCPSession : public std::enable_shared_from_this<TCPSession>
{

public:
    TCPSession(boost::asio::ip::tcp::socket socket);

    void start();

    bool hasStrData();
    std::vector<std::string> getStrData();

    void sendStrData(const std::string& str);

private:
    boost::asio::ip::tcp::socket socket_;

    std::unique_ptr<char> data_;
    unsigned int data_size_{10*1024*1024};

    boost::mutex str_data_mutex_;
    std::vector<std::string> str_data_;

    void do_read();
    void do_write(std::size_t length);
};

class TCPServer
{
public:
    TCPServer(boost::asio::io_context& io_context, short port);
    virtual ~TCPServer();

    void start();

    bool hasSession();

    bool hasStrData();
    std::vector<std::string> getStrData();

    void sendStrData(const std::string& str);

private:
    boost::asio::ip::tcp::acceptor acceptor_;
    std::shared_ptr<TCPSession> session_;

    bool started_ {false};

    void do_accept();
};
