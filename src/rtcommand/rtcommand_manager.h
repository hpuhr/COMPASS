#ifndef RTCOMMANDRECEIVER_H
#define RTCOMMANDRECEIVER_H

#include "configurable.h"
#include "singleton.h"
#include "logger.h"

#include <QThread>


#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// netcat call for console
// netcat 127.0.0.1 27960

class session
        : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket)
        : socket_(std::move(socket))
    {
    }

    void start()
    {
        do_read();
    }

private:
    void do_read()
    {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                [this, self](boost::system::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                std::string tmp;
                tmp.assign(data_, length);

                loginf << "SOCKET GOT MSG '" << tmp << "' LEN " << length;

                tmp = "are you talking to me:'"+tmp+"'";

                assert (tmp.size() <= max_length);
                tmp.copy(data_, tmp.size());
                length = tmp.size();

                do_write(length);


            }
        });
    }

    void do_write(std::size_t length)
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
            {
                do_read();
            }
        });
    }

    tcp::socket socket_;
    enum { max_length = 10*1024 };
    char data_[max_length];
};

class server
{
public:
    server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
                    [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                std::make_shared<session>(std::move(socket))->start();
            }

            do_accept();
        });
    }

    tcp::acceptor acceptor_;
};


class RTCommandManager : public QThread, public Singleton, public Configurable
{
public:
    virtual ~RTCommandManager();

    void shutdown();

    static RTCommandManager& instance()
    {
        static RTCommandManager instance;
        return instance;
    }

protected:
    volatile bool stop_requested_;
    volatile bool stopped_;

    unsigned int port_num_ {27960};

    RTCommandManager();

private:
    void run();
};

#endif // RTCOMMANDRECEIVER_H
