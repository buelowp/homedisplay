cmake_minimum_required (VERSION 3.1.0)

project (homedisplay)

FILE (GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
FILE (GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

set (CMAKE_CXX_STANDARD 17)
set (THREADS_PREFER_PTHREAD_FLAG ON)
set (CMAKE_THREAD_PREFER_PTHREAD TRUE)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)

if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set (CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package (Threads REQUIRED)
find_package (Qt6 COMPONENTS Widgets REQUIRED)
find_package (Qt6 COMPONENTS Core REQUIRED)
find_package (Qt6 COMPONENTS QtMqtt REQUIRED)
find_package (Qt6 COMPONENTS Sql REQUIRED)

add_executable (${PROJECT_NAME} ${SOURCES})

target_link_libraries (${PROJECT_NAME} Qt6::Widgets Qt6::Core Qt6::Sql Qt6::QtMqtt -lsunset -ltsl2561)

install (TARGETS ${PROJECT_NAME} DESTINATION bin)
