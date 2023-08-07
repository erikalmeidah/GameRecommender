cmake_minimum_required(VERSION 3.23)
project(GameRecommender)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

find_package(jsoncpp CONFIG REQUIRED)

add_executable(GameRecommender main.cpp bptree.cpp gameData.cpp node.cpp)

target_link_libraries(GameRecommender PRIVATE ${CURL_LIBRARIES})
target_link_libraries(GameRecommender PRIVATE ${JSONCPP_LIBRARIES})

target_include_directories(GameRecommender PRIVATE ${CURL_INCLUDE_DIRS})
target_include_directories(GameRecommender PRIVATE ${JSONCPP_INCLUDE_DIRS})
