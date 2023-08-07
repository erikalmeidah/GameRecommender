cmake_minimum_required(VERSION 3.23)
project(GameRecommender)

set(CMAKE_CXX_STANDARD 14)

# Add the following line to find and link libcurl
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

# Add the following lines to find and link JSONcpp
find_package(jsoncpp CONFIG REQUIRED)
include_directories(${JSONCPP_INCLUDE_DIRS})

add_executable(GameRecommender main.cpp bptree.cpp gameData.cpp node.cpp)

# Link libcurl to your executable
target_link_libraries(GameRecommender PRIVATE ${CURL_LIBRARIES} jsoncpp_lib)
