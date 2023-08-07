cmake_minimum_required(VERSION 3.23)
project(GameRecommender)

set(CMAKE_CXX_STANDARD 14)

add_executable(GameRecommender main.cpp bptree.cpp gameData.cpp node.cpp)
