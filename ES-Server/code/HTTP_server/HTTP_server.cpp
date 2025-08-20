#include "HTTP_server.h"
#include <filesystem> 

namespace fs = std::filesystem;
using json = nlohmann::json;

// 处理文件上传
void handle_upload(const httplib::Request& req, httplib::Response& res) {
    std::string device_id = req.path_params.at("device_id");
    OTA_Device device;
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "POST");
    res.set_header("Access-Control-Allow-Headers", "*");
    // 验证设备目录
    if (!device.OTA_device_verify(device_id.c_str())) {
        res.set_content(json{{"success", false}, {"message", "Device directory not found"}}.dump(), "application/json");
        res.status = 404;
        return;
    }

    if (!req.form.has_file("firmware")) {
        res.set_content(json{{"success", false}, {"message", "No firmware file provided"}}.dump(), "application/json");
        res.status = 400;
        return;
    }

    const auto& file = req.form.get_file("firmware");
    std::string filename = file.filename;

    // 验证文件名格式
    if (!std::regex_match(filename, std::regex("^v(\\d{1,3})\\.(\\d{1,3})\\.bin$"))) {
        res.set_content(json{{"success", false}, {"message", "Invalid filename format"}}.dump(), "application/json");
        res.status = 400;
        return;
    }

    std::string description;
    if(req.form.has_field("description")) {
        description = req.form.get_field("description");  // 获取文件对象
    }

    // 保存文件（自动覆盖）
    std::string filepath = std::string(device.OTA_Device_ID_str) + "/" + filename;
    std::ofstream out(filepath, std::ios::binary | std::ios::trunc);
    out << file.content;

    std::cout<< "upload file size is " << file.content.size() << std::endl;
    out.close();

    // 更新设备信息
    if (device.add_file(filename, description)) {
        Log(HTTP_SERVER_TYPE ,"device : %d, File uploaded/updated: %s", device_id.c_str() ,filename.c_str());
        res.set_content(json{{"success", true}, {"message", "File uploaded successfully"}}.dump(), "application/json");
    } else {
        res.set_content(json{{"success", false}, {"message", "Failed to update device info"}}.dump(), "application/json");
        res.status = 500;
    }
}


// 获取文件列表
void handle_get_files(const httplib::Request& req, httplib::Response& res) {
    std::string device_id = req.path_params.at("device_id");
    OTA_Device device;
    
    if (!device.OTA_device_verify(device_id.c_str())) {
        res.set_content(json{{"success", false}, {"message", "Device directory not found"}}.dump(), "application/json");
        res.status = 404;
        return;
    }

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET");
    res.set_header("Access-Control-Allow-Headers", "*");
    // 直接读取JSON文件
    std::ifstream f(device.OTA_Device_ID_str + std::string("/info.json"));
    if (!f.is_open()) {
        res.set_content(json{{"success", false}, {"message", "Info file not found"}}.dump(), "application/json");
        res.status = 404;
        return;
    }

    json device_info = json::parse(f);
    json response = {
        {"success", true},
        {"files", json::array()}
    };

    for (auto it = device_info.begin(); it != device_info.end(); ++it) {
        if (it.key() == "active") continue;

        response["files"].push_back({
            {"name", it.key()},
            {"description", it.value().get<std::string>()},
            {"active", (device_info["active"] == it.key())},
        });
    }

    res.set_content(response.dump(), "application/json");
}

// 处理 OPTIONS 请求（预检请求）
void handle_active_options(const httplib::Request& req, httplib::Response& res) {
    // 设置跨域相关头部
    const std::string origin = req.has_header("Origin") ? 
                              req.get_header_value("Origin") : "";
    if (!origin.empty()) {
        res.set_header("Access-Control-Allow-Origin", origin);
    } else {
        res.set_header("Access-Control-Allow-Origin", "*");
    }
    // 允许的请求方法
    res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    // 允许的请求头
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    // 预检请求的有效期（秒）
    res.set_header("Access-Control-Max-Age", "86400"); // 24小时
    
    res.status = 204; // No Content
}

// 设置活动文件
void handle_set_active(const httplib::Request& req, httplib::Response& res) {
    // 动态匹配请求源（关键！）
    const std::string origin = req.has_header("Origin") ? 
                              req.get_header_value("Origin") : "";
    if (!origin.empty()) {
        res.set_header("Access-Control-Allow-Origin", origin);
    } else {
        res.set_header("Access-Control-Allow-Origin", "*"); // 非浏览器请求回退
    }
    
    // 明确指定允许的Header而非通配符[6](@ref)
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization"); 

    std::string device_id = req.path_params.at("device_id");
    OTA_Device device;

    if (!device.OTA_device_verify(device_id.c_str())) {
        res.set_content(json{{"success", false}, {"message", "Device directory not found"}}.dump(), "application/json");
        res.status = 404;
        return;
    }

    json body = json::parse(req.body);
    std::string filename = body["file"];

    if (device.change_active_file(filename)) {
        device.update_version(); // 更新版本号
        Log(HTTP_SERVER_TYPE , "device : %d, Active file set to: %s",device_id.c_str(), filename.c_str());
        res.set_content(json{{"success", true}, {"message", "Active file updated"}}.dump(), "application/json");
    } else {
        res.set_content(json{{"success", false}, {"message", "File not found or update failed"}}.dump(), "application/json");
        res.status = 404;
    }
}

// 下载文件
void handle_download(const httplib::Request& req, httplib::Response& res) {
    std::string device_id = req.path_params.at("device_id");
    std::string filename = req.path_params.at("file_name");
    OTA_Device device;
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET ");
    res.set_header("Access-Control-Allow-Headers", "*");

    if (!device.OTA_device_verify(device_id.c_str())) {
        res.status = 404;
        res.set_content(json{{"success", false}, {"message", "Device directory not found"}}.dump(), "application/json");
        return;
    }
    
    std::string filepath = std::string(device.OTA_Device_ID_str) + "/" + filename;
    
    if (!fs::exists(filepath)) {
        res.status = 404;
        res.set_content(json{{"success", false}, {"message", "File not found"}}.dump(), "application/json");
        return;
    }

    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        res.status = 500;
        res.set_content(json{{"success", false}, {"message", "Internal server error"}}.dump(), "application/json");
        return;
    }

    // 添加版本头信息
    res.set_header("X-Firmware-Version", 
                  std::to_string(device.Bversion) + "." + 
                  std::to_string(device.Sversion));
                  
    res.set_content(std::string((std::istreambuf_iterator<char>(file)), 
                               std::istreambuf_iterator<char>()), 
                   "application/octet-stream");
}

// 处理DELETE请求的预检OPTIONS请求
void handle_delete_options(const httplib::Request& req, httplib::Response& res) {
    // 动态获取请求源
    const std::string origin = req.has_header("Origin") ? 
                              req.get_header_value("Origin") : "";
    if (!origin.empty()) {
        res.set_header("Access-Control-Allow-Origin", origin);
    } else {
        res.set_header("Access-Control-Allow-Origin", "*");
    }
    
    // 允许的请求方法，必须包含DELETE和OPTIONS
    res.set_header("Access-Control-Allow-Methods", "DELETE, OPTIONS");
    // 明确指定允许的请求头，避免使用通配符*
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    // 预检请求的有效期(秒)，24小时
    res.set_header("Access-Control-Max-Age", "86400");
    
    res.status = 204; // No Content
}

// 删除文件
void handle_delete(const httplib::Request& req, httplib::Response& res) {
    std::string device_id = req.path_params.at("device_id");
    std::string filename = req.path_params.at("file_name");
    OTA_Device device;
    
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "DELETE");
    res.set_header("Access-Control-Allow-Headers", "*");

    if (!device.OTA_device_verify(device_id.c_str())) {
        res.set_content(json{{"success", false}, {"message", "Device directory not found"}}.dump(), "application/json");
        res.status = 404;
        return;
    }

    if (device.delete_file(filename)) {
        std::string filepath = std::string(device.OTA_Device_ID_str) + "/" + filename;
        if (fs::exists(filepath)) {
            fs::remove(filepath);
            Log(HTTP_SERVER_TYPE, "device : %d, File deleted: %s", device_id.c_str(), filename.c_str());
        }
        res.set_content(json{{"success", true}, {"message", "File deleted"}}.dump(), "application/json");
    } else {
        res.set_content(json{{"success", false}, {"message", "File not found in info"}}.dump(), "application/json");
        res.status = 404;
    }
}
void start_HTTP_server(void) {
    httplib::Server svr;
    
    // 路由配置
    svr.Options("/ota/:device_id/set-active", handle_active_options);
    svr.Options("/ota/:device_id/delete/:file_name", handle_delete_options);
    svr.Post("/ota/:device_id/upload", handle_upload);
    svr.Get("/ota/:device_id/files", handle_get_files);
    svr.Post("/ota/:device_id/set-active", handle_set_active);
    svr.Get("/ota/:device_id/download/:file_name", handle_download);
    svr.Delete("/ota/:device_id/delete/:file_name", handle_delete);

    Log(HTTP_SERVER_TYPE, "Server running on HTTP_SERVER_PORT %d", HTTP_SERVER_PORT);
    svr.listen("0.0.0.0", HTTP_SERVER_PORT);

    return;
}