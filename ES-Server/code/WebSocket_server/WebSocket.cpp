#include "WebSocket.h"
using namespace nlohmann;

WebSocketSession::WebSocketSession(tcp::socket socket)
        : ws_(std::move(socket)),
          heartbeat_timer_(ws_.get_executor()),delay_timer_(ws_.get_executor()), OTA_Device_ID_flag_(true){}

void WebSocketSession::run() {
    ws_.async_accept(
        [self = shared_from_this()](beast::error_code ec) {
            if (ec) {
                std::cerr << "Accept error: " << ec.message() << std::endl;
                return;
            }
            Log(WEBSOCKET_TYPE ,"WebSocket handshake completed");
            self->writer = NULL;
            self->reader = NULL;
            self->readFromWebSocket();
        });
}

void WebSocketSession::readFromWebSocket() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (closed_) return;
    
    ws_.async_read(
        buffer_,
        [self = shared_from_this()](beast::error_code ec, size_t) {
            if (ec == websocket::error::closed) {
                self->cleanup();
                return;
            }
            if (ec == net::error::operation_aborted) {
                return;
            }
            if (ec) {
                std::cerr << "Read error: " << ec.message() << std::endl;
                self->cleanup();
                return;
            }
            
            /* 第一次需要先验证该设备ID是否开启 */
            if(self->OTA_Device_ID_flag_){
                /* 第一次需要先验证该设备ID是否开启 */

                auto data = beast::buffers_to_string(self->buffer_.data());
                self->buffer_.clear();
                
                json json_data = json::parse(data);
                std::string OTA_ID = json_data["OTA_Device_ID"];
                
                // 创建与TCP Server通信所需要的内容
                char sem_addr[50];
                char shared_memory_addr[50];
                sprintf(sem_addr, "%s%s", SEMAPHORE_PRE, OTA_ID.c_str());
                sprintf(shared_memory_addr, "%s%s", SHARED_MEMORY_PRE, OTA_ID.c_str());

                json send_data = {
                    {"information", "connection success"},
                    {"connected","on"}
                };

                if(!self->reader){
                    self->reader = new Reader(sem_addr, shared_memory_addr, NORMAL_MODE);
                }

    
                if(self->reader->is_connected_ == false){
                    Log(WEBSOCKET_TYPE ,"this device is not opened or exists");
                    send_data["information"] = "this device is not opened or exists";
                    send_data["connected"]   = "off";

                    self->ws_.async_write(
                    net::buffer(send_data.dump()),
                    [self](beast::error_code ec, size_t) {
                        if (ec && ec != net::error::operation_aborted) {
                            std::cerr << "Write error: " << ec.message() << std::endl;
                            self->cleanup();
                            return;
                        }
                    });
                    
                    self->cleanup();
                    return;
                }else{
                    sprintf(sem_addr, "%s%s2", SEMAPHORE_PRE, OTA_ID.c_str());
                    sprintf(shared_memory_addr, "%s%s2", SHARED_MEMORY_PRE, OTA_ID.c_str());
                    if(!self->writer){
                        self->writer = new Writer(sem_addr, shared_memory_addr, NORMAL_MODE);
                    }
                    self->ws_.async_write(
                        net::buffer(send_data.dump()),
                        [self](beast::error_code ec, size_t) {
                            if (ec && ec != net::error::operation_aborted) {
                                std::cerr << "Write error: " << ec.message() << std::endl;
                                self->cleanup();
                                return;
                            }
                    });
                    self->writeToWebSocket();
                }
                self->OTA_Device_ID_flag_ = false;
            }else{
                // 获得json数据并且清空
                auto data = beast::buffers_to_string(self->buffer_.data());
                self->buffer_.clear();

                std::string data_str = data.c_str();
                json data_json;
                try {
                    data_json = json::parse(data_str);
                } catch (json::parse_error& e) {
                    std::cerr << "JSON解析失败: " << e.what() << "\n";
                    // 记录原始数据日志，便于调试
                    Log(WEBSOCKET_TYPE ,"原始数据: %s", data.c_str());
                }

                if(data_json.contains("method")){
                    std::string method = data_json["method"];
                    
                    char msg[256];
    
                    if(strcmp(method.c_str(), "RESET") == 0){
                        Log(WEBSOCKET_TYPE ,"RESET command recive");
                        msg[0] = '\0';
                        self->writer->write(msg, RESET);
                    }else if(strcmp(method.c_str(), "update PID") == 0){
                        Log(WEBSOCKET_TYPE ,"update PID command recive");
                        float f_arr[9] = {(float)data_json["Pitch_P"], (float)data_json["Pitch_I"], (float)data_json["Pitch_D"],
                                          (float)data_json["Yaw_P"],   (float)data_json["Yaw_I"],   (float)data_json["Yaw_D"],
                                          (float)data_json["Roll_P"],  (float)data_json["Roll_I"],  (float)data_json["Roll_D"]};
                        memcpy(msg, f_arr, sizeof(float) * 9);
                        self->writer->write_len(msg, UPDATE_PID, sizeof(float)*9);
                    }else if(strcmp(method.c_str(), "save PID") == 0){
                        Log(WEBSOCKET_TYPE ,"save PID command recive");
                        float f_arr[9] = {(float)data_json["Pitch_P"], (float)data_json["Pitch_I"], (float)data_json["Pitch_D"],
                                          (float)data_json["Yaw_P"],   (float)data_json["Yaw_I"],   (float)data_json["Yaw_D"],
                                          (float)data_json["Roll_P"],  (float)data_json["Roll_I"],  (float)data_json["Roll_D"]};
                        memcpy(msg, f_arr, sizeof(float) * 9);
                        self->writer->write_len(msg, SAVE_PID, sizeof(float)*9);
                    }else if(strcmp(method.c_str(), "init PID") == 0){
                        Log(WEBSOCKET_TYPE ,"init PID command recive");
                        msg[0] = '\0';
                        self->writer->write(msg, INIT_PID);
                    }
                }
                write(STDOUT_FILENO, data.c_str(), data.size());
                self->buffer_.consume(self->buffer_.size());
            }
            self->readFromWebSocket();
        });
}

void WebSocketSession::writeToWebSocket() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (closed_) return;
    
    char msg[1024];
    uint8_t type;
    uint32_t len;
    bool flag = false;

    reader->read(msg, &len, &type);
    json send_data = {
        {"connected","on"}
    };
    switch(type){
        case PRINT:
            // 将数据异步写入WebSocket
            send_data["log"] = msg;
            break;
        case EXIT:
            Log(WEBSOCKET_TYPE ,"recv EXIT sign");
            send_data["log"] = "The device is disconnected from the server";
            send_data["information"] = "device exits";
            send_data["connected"] = "off";
            writer->write("\0", EXIT);
            break;
        case INIT_PID:
            float arr[9]; 
            sscanf(msg, "%f %f %f %f %f %f %f %f %f",
                            &arr[0], &arr[1], &arr[2],
                            &arr[3], &arr[4], &arr[5],
                            &arr[6], &arr[7], &arr[8]);
            send_data["PID_set"] = "SET";
            send_data["Pitch_P"] = arr[0];
            send_data["Pitch_I"] = arr[1];
            send_data["Pitch_D"] = arr[2];
            send_data["Yaw_P"]   = arr[3];
            send_data["Yaw_I"]   = arr[4];
            send_data["Yaw_D"]   = arr[5];
            send_data["Roll_P"]  = arr[6];
            send_data["Roll_I"]  = arr[7];
            send_data["Roll_D"]  = arr[8];
            break;
        case GY86:
            int gy86_arr[9];
            float gy86_brr[5];
            
            sscanf(msg, "%d %d %d %d %d %d %d %d %d %f %f %f %f %f",
                            &gy86_arr[0], &gy86_arr[1], &gy86_arr[2],
                            &gy86_arr[3], &gy86_arr[4], &gy86_arr[5],
                            &gy86_arr[6], &gy86_arr[7], &gy86_arr[8],
                            &gy86_brr[0], &gy86_brr[1], &gy86_brr[2],
                            &gy86_brr[3], &gy86_brr[4]);
            Log(WEBSOCKET_TYPE ,"gy86 SET");
            send_data["gy86"]  = msg;
            send_data["AccX"]  = gy86_arr[0];
            send_data["AccY"]  = gy86_arr[1];
            send_data["AccZ"]  = gy86_arr[2];
            send_data["GyroX"] = gy86_arr[3];
            send_data["GyroY"] = gy86_arr[4];
            send_data["GyroZ"] = gy86_arr[5];
            send_data["GaX"]   = gy86_arr[6];
            send_data["GaY"]   = gy86_arr[7];
            send_data["GaZ"]   = gy86_arr[8];
            send_data["Pressure"]    = gy86_brr[0];
            send_data["Temperature"] = gy86_brr[1];
            send_data["pitch"] = gy86_brr[2];
            send_data["yaw"]   = gy86_brr[3];
            send_data["roll"]  = gy86_brr[4];
            break;
        case SERVO:
            int servo_arr[8];
            sscanf(msg, "%d %d %d %d %d %d %d %d",
                        &servo_arr[0], &servo_arr[1], &servo_arr[2], &servo_arr[3],
                        &servo_arr[4], &servo_arr[5], &servo_arr[6], &servo_arr[7]);
            Log(WEBSOCKET_TYPE ,"servo SET");
            send_data["servo"] = msg;
            
            break;
        default:
            flag = true;
            break;
    }



    if(type){
        net::post(ws_.get_executor(), 
            [self = shared_from_this(), data = send_data.dump(), type = type]() {
                std::lock_guard<std::mutex> lock(self->mutex_);
                if (self->closed_) return;
                self->ws_.async_write(
                    net::buffer(data),
                    [self, type](beast::error_code ec, size_t) {
                        if (ec && ec != net::error::operation_aborted) {
                            std::cerr << "Write error: " << ec.message() << std::endl;
                            self->cleanup();
                            return;
                        }
                        // 继续读取管道
                        if(type == EXIT){
                            self->cleanup();
                        }else{
                            self->writeToWebSocket();
                        }
                    });
        });
    }else{
        writeToWebSocket();
    }

    // if(flag){
    //     // 创建1ms延时定时器
    //     delay_timer_.expires_after(std::chrono::milliseconds(1));
    //     delay_timer_.async_wait(
    //         [self = shared_from_this()](beast::error_code ec) {
    //             if (ec) {
    //                 // 定时器被取消或出错
    //                 if (ec != net::error::operation_aborted) {
    //                     std::cerr << "Delay timer error: " << ec.message() << std::endl;
    //                 }
    //                 return;
    //             }
    //             // 1ms后重新调用writeToWebSocket
    //             self->writeToWebSocket();
    //         });
    // }
}

void WebSocketSession::cleanup() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (closed_) return;
    closed_ = true;
    
    beast::error_code ec;
    // 取消心跳定时器
    heartbeat_timer_.cancel();
    // 关闭WebSocket
    ws_.close(websocket::close_code::normal, ec);
    if (ec) {
        std::cerr << "Close error: " << ec.message() << std::endl;
    }

    if(writer){
        writer->destory();
        delete writer;
    }
    if(reader){
        reader->destory();
        delete reader;
    }
    Log(WEBSOCKET_TYPE ,"Connection closed");
}

WebSocketServer::WebSocketServer(net::io_context& ioc, tcp::endpoint endpoint)
        : acceptor_(ioc, endpoint) {
        accept();
}

void WebSocketServer::accept() {
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            if (ec) {
                std::cerr << "Accept failed: " << ec.message() << std::endl;
                accept();
                return;
            }
            
            Log(WEBSOCKET_TYPE ,"New connection accepted");
            

            // 创建处理线程
            std::thread([sock = std::move(socket)]() mutable {
                pthread_setname_np(pthread_self(), "ws_thread");
                
                try {
                    // 创建会话
                    auto session = std::make_shared<WebSocketSession>(
                        std::move(sock));
                    Log(WEBSOCKET_TYPE ,"Starting WebSocket session");
                    
                    session->run();
                    
                } catch (const std::exception& e) {
                    std::cerr << "Thread error: " << e.what() << std::endl;
                }
            }).detach();
            
            
            // 继续接受新连接
            accept();
        });
}

int start_WebSocket_server() {
    try {
        net::io_context ioc;
        auto const address = net::ip::make_address("0.0.0.0");
        auto const port = static_cast<unsigned short>(WEBSOCKET_PORT);
        
        WebSocketServer server(ioc, tcp::endpoint{address, port});
        
        Log(WEBSOCKET_TYPE, "WebSocket server listening on port %d...", WEBSOCKET_PORT);
        ioc.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    Log(WEBSOCKET_TYPE ,"exit");
    return EXIT_SUCCESS;
}
