cmake_minimum_required(VERSION 3.1.0)

project(piclock)

FILE (GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE (GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set (CMAKE_CXX_STANDARD 17)
set (THREADS_PREFER_PTHREAD_FLAG ON)
set (CMAKE_THREAD_PREFER_PTHREAD TRUE)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package (Threads REQUIRED)
find_package (Qt5 COMPONENTS Widgets REQUIRED)
find_package (Qt5 COMPONENTS Core REQUIRED)
find_package (Qt5 COMPONENTS Qmqtt REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Core Qt5::Qmqtt)

install (TARGETS {$PROJECT_NAME} DESTINATION bin)
