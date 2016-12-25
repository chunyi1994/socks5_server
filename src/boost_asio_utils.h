#ifndef BOOST_ASIO_UTILS_H
#define BOOST_ASIO_UTILS_H
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <cstddef>
#include <vector>
#include <iostream>
namespace utils {
//功能：发送消息到socket
//参数
//1、socket,
//2、buf, 发送的信息
//3、n, 发送信息的大小
//4、error, 错误码
//5、yield, 协程上下文
static void send(boost::asio::ip::tcp::socket& socket,
                 std::vector<char> &buf,
                 std::size_t n,
                 boost::system::error_code& error,
                 boost::asio::yield_context &yield) {
    boost::asio::async_write(socket, boost::asio::buffer(buf, n), yield[error]);
}

//功能：带有超时检查的recv
//参数
//1、socket,
//2、buf, 发送的信息
//3、timeout, 超时时间
//4、error, 错误码
//5、yield, 协程上下文
//返回：接收的信息大小
static std::size_t recv_with_timeout(boost::asio::ip::tcp::socket& socket,
                                     std::vector<char> &buf,
                                     std::size_t timeout,
                                     boost::system::error_code& error,
                                     boost::asio::yield_context &yield) {
    boost::asio::deadline_timer timer(socket.get_io_service());
    if (timeout > 0) {
        timer.expires_from_now(boost::posix_time::seconds(timeout));
        timer.async_wait([&socket](const boost::system::error_code& err) {
            if (err) return; //定时器取消
            socket.shutdown(boost::asio::ip::tcp::socket::shutdown_receive);
            utils::log("有一个从浏览器到代理的接收链接超时");
        });
    }
    std::size_t n =  socket.async_read_some(boost::asio::buffer(buf), yield[error]);
    return n;
}

//功能：带有超时检查的connect连接
//参数
//1、socket,
//2、addr, 传入的ip地址或者网址
//3、port, 端口号
//4、timeout, 超时事件
//5、error，错误码
//6、yield, 协程上下文
static void connect_with_timeout(boost::asio::ip::tcp::socket& socket,
                                 const std::string &addr,
                                 const std::string &port,
                                 std::size_t timeout,
                                 boost::system::error_code& error,
                                 boost::asio::yield_context &yield) {
    boost::asio::deadline_timer timer(socket.get_io_service());
    if (timeout > 0) {
        timer.expires_from_now(boost::posix_time::seconds(timeout));
        timer.async_wait([&socket](const boost::system::error_code& err) {
            if (err)  return; //说明定时取消
            socket.cancel();
            utils::log("有一个链接超时");
        });
    }
    boost::asio::ip::tcp::resolver resolver(socket.get_io_service());
    std::cout<<"正在尝试连接"<<addr<<" : "<<port<<std::endl;
    boost::asio::ip::tcp::resolver::query query(addr, port);
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    boost::asio::async_connect(socket, endpoint_iterator, yield[error]);
}

//功能：用于将源socket接收的消息转发到目标socket
//参数
//1、src_socket, 源socket
//2、dest_socket, 目标socket
//3、yield, 协程上下文
static void iocopy(boost::asio::ip::tcp::socket &src_socket,
                   boost::asio::ip::tcp::socket &dest_socket,
                   boost::asio::yield_context yield) {
    std::size_t n = 0;
    boost::system::error_code error;
    std::vector<char> buf(1024);
    for(;;) {
        n = recv_with_timeout(src_socket, buf, 0, error, yield);//0表示无timeout
        if (error) {
            utils::log("error:" + error.message());
            try {
                dest_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_receive);
            } catch (std::exception& e) {
                //std::cerr << "Exception: " << e.what() << "\n";
            }
            break;
        }
        send(dest_socket, buf, n, error, yield);
        if (error) {
            break;
        }
    }
}
} //namespace


//1、socket,
//2、buf, 发送的信息
//3、timeout, 超时时间
//4、error, 错误码
//5、yield, 协程上下文
//返回：接收的信息大小
#endif // BOOST_ASIO_UTILS_H

