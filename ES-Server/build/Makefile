CXX = g++
# 默认编译选项（C++17）
CXXFLAGS_COMMON = -O0 -Wall
# WebSocket_server专用选项（C++14）
CXXFLAGS_WS = -std=c++14 $(CXXFLAGS_COMMON)
# 其他文件编译选项（C++17）
CXXFLAGS_OTHERS = -std=c++17 $(CXXFLAGS_COMMON)
# 链接选项
LDFLAGS = -pthread -lwebsockets -lrt

TARGET = server

# 源文件路径
SRC_DIRS = ../code/OTA_device/*.cpp ../code/TCP_server/*.cpp ../code/Log/*.cpp\
           ../code/WebSocket_server/*.cpp ../code/RW_tools/*.cpp ../code/HTTP_server/*.cpp 
		  

# 查找所有源文件
SRCS = $(shell find $(SRC_DIRS) -name '*.cpp') ../code/main.cpp
# 提取WebSocket_server源文件
WS_SRCS = $(filter ../code/WebSocket_server/%, $(SRCS))
# 其他源文件
OTHER_SRCS = $(filter-out ../code/WebSocket_server/%, $(SRCS))

# 生成对象文件列表
WS_OBJS = $(WS_SRCS:.cpp=.o)
OTHER_OBJS = $(OTHER_SRCS:.cpp=.o)
OBJS = $(WS_OBJS) $(OTHER_OBJS)

all: $(TARGET)

# 最终目标链接
$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

# WebSocket_server文件使用C++14编译
$(WS_OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS_WS) -c $< -o $@

# 其他文件使用C++17编译
$(OTHER_OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS_OTHERS) -c $< -o $@

# 声明伪目标
.PHONY: clean

clean:
	rm -f $(OBJS) $(TARGET)


