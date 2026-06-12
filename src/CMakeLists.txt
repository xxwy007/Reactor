cmake_minimum_required(VERSION 3.10)

# 可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ../${PROJECT_SOURCE_DIR}/bindx64
)


file(GLOB ser 
    "./Reactor.cpp" 
    "./Server.cpp"
    "./ThreadPool.cpp"
)

file(GLOB cli "./Client.cpp")

add_executable(Server ${ser})
add_executable(Client ${cli})

target_link_libraries(Server
    PRIVATE
    Threads::Threads
)

target_link_libraries(Client
    PRIVATE
    Threads::Threads
)