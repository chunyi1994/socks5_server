#include "session.h"
#include "utils.h"
#include "boost_asio_utils.h"
namespace socks {
Session::Session(boost::asio::io_service &io_service)
    : ioservice_(io_service), socket_(io_service)
{
}

Session::~Session() {
    utils::log("~session");
}

Session::Pointer Session::create(boost::asio::io_service &io_service) {
    return std::make_shared<Session>(io_service);
}

boost::asio::ip::tcp::socket &Session::socket() {
    return socket_;
}

void Session::go(){
    boost::asio::spawn(ioservice_,  //协程会调用handleConnection函数，并且持有引用计数，直到handleConnection函数结束退出
                       std::bind(&Session::handleConnection,
                                 shared_from_this(), std::placeholders::_1));
}

int handshake(boost::asio::ip::tcp::socket &socket,
              boost::asio::ip::tcp::socket & remote_conn,
              boost::asio::yield_context& yield) {
    boost::system::error_code error;
    std::vector<char> data(200, 0x00);
    //第一次握手
    std::size_t n = utils::recv_with_timeout(socket, data, ASYNC_TIME_OUT_SEC, error, yield);
    if (error || data[0] != 0x05 || data[1] != 0x01 || data[2] != 0x00) {
        return -1; //todo 应该回一个错误报文
    }
    std::vector<char> vec_ok1 = {0x05, 0x00};
    utils::send(socket, vec_ok1, vec_ok1.size(), error, yield);
    if (error) {
        utils::log(error.message());
        return -1;
    }
    //第二次握手
    n =  utils::recv_with_timeout(socket,data, ASYNC_TIME_OUT_SEC, error, yield);
    if(error || data[0] != 0x05 || data[1] != 0x01 || data[2] != 0x00) {
        utils::log(error.message());
        return -1; //todo 应该回一个错误报文
    }

    std::size_t url_size = data[4];
    //解析请求报文   //char addr_type = recv[3];
    std::string url(data.data() + 5, url_size);
    unsigned short port_short = *((unsigned short*) &(data[5 + url_size]));
    port_short = boost::asio::detail::socket_ops::network_to_host_short(port_short);
    //尝试连接远程主机
    utils::connect_with_timeout(remote_conn, url,
                                utils::int2string((int)port_short),
                                ASYNC_TIME_OUT_SEC, error, yield);
    //构造返回报文
    if (error) {
       // data[1] = 0x01; //o X'01' general SOCKS server failure
        utils::log(error.message());
        return -1;
    } else {
        data[1] = 0x00;
    }
    utils::send(socket, data, n, error, yield);
    if (error) {
        utils::log(error.message());
        return -1;
    }
    return port_short;
}

const int MAX_BUFF_SIZE = 4000;
void Session::handleConnection(boost::asio::yield_context yield) {
    try {
        std::vector<char> data(MAX_BUFF_SIZE);
        boost::asio::ip::tcp::socket remote_conn(ioservice_);
        int port = handshake(socket_, remote_conn, yield);
        if (port < 0) return;

        //相当于开了一个线程 utils::iocopy(socket_, remote_conn, yield);
        boost::asio::spawn(ioservice_,
                           std::bind(utils::iocopy, std::ref(socket_),
                                     std::ref(remote_conn), std::placeholders::_1));

        utils::iocopy(remote_conn, socket_, yield);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
}//namespace
