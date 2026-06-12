cmake_minimum_required(VERSION 3.10)

project(CReactor)

file(GLOB ser "./reactor.cpp")

file(GLOB cli "./client.cpp")

# 可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ../${PROJECT_SOURCE_DIR}/bindx64
)

add_executable(CServer ${ser})
add_executable(CClient ${cli})

target_link_libraries(CServer
    PRIVATE
    Threads::Threads
)

target_link_libraries(CClient
    PRIVATE
    Threads::Threads
)