cmake_minimum_required(VERSION 3.10)

# 可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ../${PROJECT_SOURCE_DIR}/bindx64
)

set(core_sources
    Channel.cpp
    EventLoop.cpp
    Reactor.cpp
    RpcDispatcher.cpp
    ThreadPool.cpp
)

file(GLOB common_src "common/*")

add_executable(Server ${core_sources} ${common_src} test/Server.cpp)
add_executable(Client ${core_sources} ${common_src} test/Client.cpp)

target_link_libraries(Server
    PRIVATE
    Threads::Threads
)

target_link_libraries(Client
    PRIVATE
    Threads::Threads
)