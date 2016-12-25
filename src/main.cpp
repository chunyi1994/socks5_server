#include <boost/asio/spawn.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <functional>
#include <iostream>
#include <thread>
#include "session.h"
using namespace socks;
using namespace std;

void do_accept(boost::asio::io_service& io_service,
               boost::asio::ip::tcp::acceptor& acceptor,
               boost::asio::yield_context yield) {

    for (;;) {
        boost::system::error_code ec;
        Session::Pointer new_session = Session::create(io_service);
        acceptor.async_accept(new_session->socket(), yield[ec]);
        if (!ec) {
            new_session->go();
        }
    }
}

int main(int argc, char* argv[]) {
    try {
        int port = 1081;
        boost::asio::io_service io_service;
        boost::asio::ip::tcp::acceptor acceptor(io_service,
                                                boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

        boost::asio::spawn(io_service,     //协程会执行do_accept这个函数
                           std::bind(do_accept, std::ref(io_service), std::ref(acceptor), std::placeholders::_1));
        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}

//void main_func(boost::asio::ip::tcp::acceptor& acceptor) {
//    try {
//        boost::asio::io_service io_service;
//        boost::asio::spawn(io_service,     //协程会执行do_accept这个函数
//                           std::bind(do_accept, std::ref(io_service), std::ref(acceptor), std::placeholders::_1));
//        boost::asio::io_service::work work(io_service);
//        io_service.run();
//        cout<<"no!"<<endl;
//    } catch (std::exception& e) {
//        std::cerr << "Exception: " << e.what() << "\n";
//    }
//}

//int main(int argc, char* argv[]) {
//    int port = 1081;
//    boost::asio::io_service io_service;
//    boost::asio::ip::tcp::acceptor acceptor(io_service,
//        boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

//    try {
//        boost::asio::spawn(io_service,     //协程会执行do_accept这个函数
//                           std::bind(do_accept, std::ref(io_service), std::ref(acceptor), std::placeholders::_1));
//        io_service.run();
//    } catch (std::exception& e) {
//        std::cerr << "Exception: " << e.what() << "\n";
//    }
//    return 0;
//}
