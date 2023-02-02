#include "tcpserver.h"
#include "logger.h"
#include "stringconv.h"

using namespace std;
using namespace Utils;

TCPSession::TCPSession(boost::asio::ip::tcp::socket socket)
    : socket_(std::move(socket))
{
    loginf << "TCPSession: ctor";
}

void TCPSession::start()
{
    do_read();
}

bool TCPSession::hasStrData()
{
    boost::mutex::scoped_lock lock(str_data_mutex_);
    return str_data_.size();
}

std::vector<std::string> TCPSession::getStrData()
{
    boost::mutex::scoped_lock lock(str_data_mutex_);

    assert (str_data_.size());
    return move(str_data_);
}

void TCPSession::sendStrData(const std::string& str)
{
    str.copy(data_, str.size());
    do_write(str.size());
}

void TCPSession::do_read()
{
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            [this, self](boost::system::error_code ec, std::size_t length)
    {
        if (!ec)
        {
            std::string tmp;
            tmp.assign(data_, length);

            vector<string> string_vec = String::split(tmp, '\n');

            //loginf << "SOCKET GOT MSG '" << tmp << "' LEN " << length << " num sub strings " << string_vec.size();

            boost::mutex::scoped_lock lock(str_data_mutex_);
            str_data_.insert(str_data_.end(), string_vec.begin(), string_vec.end());

            do_read();
        }
    });
}

void TCPSession::do_write(std::size_t length)
{
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
                             [this, self](boost::system::error_code ec, std::size_t /*length*/)
    {
        assert (!ec);
    });
}

TCPServer::TCPServer(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
    do_accept();
}

TCPServer::~TCPServer()
{

}

void TCPServer::do_accept()
{
    acceptor_.async_accept(
                [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
    {
        if (!ec)
        {
            session_ = std::make_shared<TCPSession>(std::move(socket));
            session_->start();
        }

        do_accept();
    });
}

bool TCPServer::hasSession()
{
    return session_ != nullptr;
}

bool TCPServer::hasStrData()
{
    assert (session_);
    return session_->hasStrData();
}

std::vector<std::string> TCPServer::getStrData()
{
    assert (session_);
    return session_->getStrData();
}

void TCPServer::sendStrData(const std::string& str)
{
    assert (session_);
    session_->sendStrData(str);
}
