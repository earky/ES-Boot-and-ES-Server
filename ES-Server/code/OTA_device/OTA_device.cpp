#include "OTA_device.h"

using namespace nlohmann;

OTA_Device::OTA_Device(){
    memset(OTA_Device_ID, 0, OTA_DEVICE_ID_LEN);
    version = 0;
}

bool OTA_Device::update_version(){
    try {
        // 1. 读取JSON文件
        std::ifstream in_file(json_file);
        if (!in_file.good()) {
            std::cerr << "JSON文件不存在: " << json_file << std::endl;
            return false;
        }

        nlohmann::json data = nlohmann::json::parse(in_file);

        // 2. 获取active字段对应的文件名
        if (!data.contains("active") || !data["active"].is_string()) {
            std::cerr << "active字段不存在或类型错误" << std::endl;
            return false;
        }
        std::string active_file = data["active"].get<std::string>();

        // 3. 正则解析版本号（如：v1.135.bin）
        std::regex pattern(R"(v(\d+)\.(\d+)\.bin)");
        std::smatch matches;
        if (!std::regex_match(active_file, matches, pattern) || matches.size() != 3) {
            std::cerr << "文件名格式错误: " << active_file << std::endl;
            return false;
        }

        // 4. 提取主版本号（Bversion）和次版本号（Sversion）
        uint16_t major = static_cast<uint16_t>(std::stoi(matches[1].str()));
        uint16_t minor = static_cast<uint16_t>(std::stoi(matches[2].str()));

        // 5. 赋值成员变量
        Bversion = major;
        Sversion = minor;
        version = (Bversion << 8) | Sversion;  // 合并为16位版本号

        return true;
    } catch (const std::exception& e) {
        std::cerr << "版本更新异常: " << e.what() << std::endl;
        return false;
    }
}


bool OTA_Device::OTA_device_verify(const char* str) {
    struct stat info;
    std::string dir_path = std::string(str);
    if(stat((const char*)dir_path.c_str(), &info) == 0 && S_ISDIR(info.st_mode)){
        // 验证通过
        base_directory = dir_path + "/";
        json_file = base_directory + JSON_NAME;
        memcpy(OTA_Device_ID, dir_path.c_str(), OTA_DEVICE_ID_LEN);
        memcpy(OTA_Device_ID_str, dir_path.c_str(), OTA_DEVICE_ID_LEN + 1);
        printf("base_directory is %s, json_file is %s\n",base_directory.c_str(), json_file.c_str());

        update_version();
        return true;
    }else{
        // 验证失败
        return false;
    }
}


bool OTA_Device::add_file(const std::string& filename, const std::string& description) {
    try {
        // 读取现有JSON文件
        json data;
        std::ifstream in_file(json_file);
        if (in_file.good()) {
            in_file >> data;
            in_file.close();
        }
        
        // 添加或更新条目
        data[filename] = description;
        
        // 写回文件
        std::ofstream out_file(json_file);
        out_file << data.dump(4);  // 带缩进格式化
        return out_file.good();
    }
    catch (...) {
        return false;
    }
}

bool OTA_Device::delete_file(const std::string& filename) {
    try {
        // 读取现有JSON文件
        json data;
        std::ifstream in_file(json_file);
        if (!in_file.good()) return false;
        
        in_file >> data;
        in_file.close();
        
        // 删除指定键
        if (data.contains(filename)) {
            data.erase(filename);
            
            // 如果删除的是活动文件，清除活动文件设置
            if (data["active"] == filename) {
                data["active"] = "";
            }
            
            // 写回文件
            std::ofstream out_file(json_file);
            out_file << data.dump(4);
            return out_file.good();
        }
        return false;
    }
    catch (...) {
        return false;
    }
}

bool OTA_Device::change_active_file(const std::string& filename) {
    try {
        // 读取现有JSON文件
        json data;
        std::ifstream in_file(json_file);
        if (!in_file.good()) return false;
        
        in_file >> data;
        in_file.close();
        
        // 确保文件存在
        if (data.contains(filename)) {
            data["active"] = filename;
            
            // 写回文件
            std::ofstream out_file(json_file);
            out_file << data.dump(4);
            return out_file.good();
        }
        return false;
    }
    catch (...) {
        return false;
    }
}