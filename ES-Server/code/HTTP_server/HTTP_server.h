#ifndef __HTTP_SERVER_H
#define __HTTP_SERVER_H

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <map>
#include <regex>
#include "../cpp-httplib/httplib.h"
#include "../json/single_include/nlohmann/json.hpp"
#include "../OTA_device/OTA_device.h" // 包含设备管理类
#include "../Log/Log.h"

void start_HTTP_server();

#endif
