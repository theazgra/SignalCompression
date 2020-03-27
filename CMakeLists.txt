cmake_minimum_required(VERSION 3.10)
project(asc CXX)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC")
# set(CMAKE_CXX_STANDARD 17)


add_subdirectory(AzgraCppLibrary)

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fsanitize=address")

add_executable(asc src/main.cpp src/huffman.cpp src/lzss/lzss_token.cpp src/lzss/lzss.cpp src/move_to_front.cpp src/bwt.cpp src/rle.cpp)
target_compile_options(asc PRIVATE -Wall -Wpedantic)

target_link_libraries(asc PRIVATE azgra)
set_property(TARGET asc  PROPERTY CXX_STANDARD 17)

find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    message("------- OpenMP ENABLED -------")
    target_link_libraries(asc PRIVATE OpenMP::OpenMP_CXX)
endif()
