cmake_minimum_required(VERSION 2.8)

project(ff-driver)

file(GLOB SOURCE_FILES_LIB "src/*.cpp" "src/*.h" )

add_library("ff-driver" SHARED ${SOURCE_FILES_LIB})

target_include_directories("ff-driver" PRIVATE "deps/asio/asio/include" "deps/rapidjson/include"  "include" )

target_compile_definitions ("ff-driver" PRIVATE "EXPORT_API=1")

file(GLOB SOURCE_FILES_CLI "cli/*.cpp" "cli/*.h" )

add_executable("ff-driver-cmd" ${SOURCE_FILES_CLI})

target_include_directories("ff-driver-cmd" PRIVATE "deps/asio/asio/include" "deps/rapidjson/include" "deps/args" "include" )

add_dependencies("ff-driver-cmd" "ff-driver")

target_link_libraries("ff-driver-cmd" PRIVATE "ff-driver")


