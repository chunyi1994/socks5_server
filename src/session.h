#ifndef SESSION_H
#define SESSION_H
#include <boost/asio/spawn.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <functional>
#include <iostream>
#include <vector>
namespace socks {

class Session : public std::enable_shared_from_this < Session > {
public:
    typedef std::shared_ptr<Session> Pointer;

public:
    explicit Session(boost::asio::io_service& io_service);
    ~Session();

    static Pointer create(boost::asio::io_service& io_service);
    boost::asio::ip::tcp::socket& socket();
    void go();

private:
    void handleConnection(boost::asio::yield_context yield);

private:
    boost::asio::io_service& ioservice_;
    boost::asio::ip::tcp::socket socket_;
};

const int ASYNC_TIME_OUT_SEC = 10;

}//namespace
#endif // SESSION_H
