#ifndef __OTA_DEVICE_H
#define __OTA_DEVICE_H

#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdint.h>
#include <regex>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include "../json/single_include/nlohmann/json.hpp"

#define OTA_DEVICE_ID_LEN 8

#define SEMAPHORE_PRE     "/semaphore"
#define SHARED_MEMORY_PRE "/shared_memory"
#define SHARED_MEMORY_SIZE 4096

#define JSON_NAME "info.json"

class OTA_Device {
public:
    OTA_Device();

    /* 不包含终止符的ID */
    char OTA_Device_ID[OTA_DEVICE_ID_LEN];
    /* 包含终止符的ID */
    char OTA_Device_ID_str[OTA_DEVICE_ID_LEN + 1];

    uint16_t version;   //版本号
    uint8_t Bversion;   //大版本号
    uint8_t Sversion;   //小版本号


    int OTA_Devices_fd; //最初版本的fd



    // 验证设备ID对应的目录是否存在
    bool OTA_device_verify(const char* str);
    // 更新版本号
    bool update_version(void);
    // 添加文件条目到info.json
    bool add_file(const std::string& filename, const std::string& description);
    // 从info.json删除文件条目
    bool delete_file(const std::string& filename);
    // 更改活动的文件
    bool change_active_file(const std::string& filename);

private:
    std::string base_directory;  // 设备目录路径（如"device_id/"）
    std::string json_file;       // JSON文件完整路径（如"device_id/info.json"）

    //bool update_version();
};

#endif
