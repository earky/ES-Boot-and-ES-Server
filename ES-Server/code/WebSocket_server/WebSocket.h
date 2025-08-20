#ifndef __WEBSOCKET_SERVER_H
#define __WEBSOCKET_SERVER_H

#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include "../json/single_include/nlohmann/json.hpp"
#include <unistd.h>
#include <atomic>
#include <mutex>
#include <fcntl.h>
#include "../OTA_device/OTA_device.h"
#include "../RW_tools/reader.h"
#include "../RW_tools/writer.h"
#include "../define.h"
#include "../Log/Log.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
public:
    WebSocketSession(tcp::socket socket);

    void run();

private:
    // 启动心跳检测
    void start_heartbeat();

    void readFromWebSocket();

    void writeToWebSocket();

    void cleanup();

    void ping();
    
    websocket::stream<tcp::socket> ws_;
    beast::flat_buffer buffer_;
    int pipe_in_;
    int pipe_out_;
    std::mutex mutex_;
    bool closed_ = false;
    net::steady_timer heartbeat_timer_; // 心跳定时器[6]
    bool OTA_Device_ID_flag_;           // 用于存储第一次获取到信息，即OTA设备ID
    sem_t *sem;
    int shm_fd;
    void * ptr;

    Reader* reader;
    Writer* writer;
    net::steady_timer delay_timer_;
};




class WebSocketServer {
public:
    WebSocketServer(net::io_context& ioc, tcp::endpoint endpoint);

private:
    void accept();

    tcp::acceptor acceptor_;
};

int start_WebSocket_server(void);

#endif
