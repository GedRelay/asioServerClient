cmake_minimum_required(VERSION 3.16)
project(server)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(output_name server)  # 生成的可执行文件名称

# 项目基础目录配置
set(project_root ${CMAKE_CURRENT_SOURCE_DIR})  # 设置项目根目录
set(include_path ${project_root}/include)
set(proto_path ${project_root}/proto)  # proto目录
set(src_path ${project_root}/src)  # 设置源文件目录
set(bin_path ${project_root}/bin)  # 设置可执行文件目录
include_directories(${include_path})
include_directories(${proto_path})


# boost配置
find_package(Boost REQUIRED COMPONENTS system thread)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})


#jsoncpp配置
include_directories(/usr/include/jsoncpp/)
link_directories(/usr/lib/x86_64-linux-gnu/)


# protobuf配置
include_directories(/usr/local/protobuf3_21_12/include/)
link_directories(/usr/local/protobuf3_21_12/lib/)


# 链接静态库
link_libraries(jsoncpp)


# 生成可执行文件
file(GLOB src_list ${src_path}/*.cpp)
file(GLOB stc_list2 ${proto_path}/*.cc)
set(src_list ${src_list} ${stc_list2})
set(EXECUTABLE_OUTPUT_PATH ${bin_path})
add_executable(${output_name} ${src_list})


# 链接库
target_link_libraries(${output_name} pthread)
target_link_libraries(${output_name} protobuf)