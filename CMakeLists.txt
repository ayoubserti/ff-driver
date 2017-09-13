cmake_minimum_required(VERSION 2.8)

project(ff-driver)

file(GLOB SOURCE_FILES "src/*.cpp" "src/*.h" )


add_executable("ff-driver-cmd" ${SOURCE_FILES})




#target_include_directories(${PROJECT_NAME} PRIVATE )

