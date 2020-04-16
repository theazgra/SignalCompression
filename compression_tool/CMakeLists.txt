cmake_minimum_required(VERSION 3.16)
project(compression_tool)

set(CMAKE_CXX_STANDARD 20)

add_executable(compression_tool main.cpp compressors.cpp)

find_package (Threads REQUIRED)
target_link_libraries (${PROJECT_NAME} PRIVATE ${CMAKE_THREAD_LIBS_INIT})

find_package(ZLIB)
if(ZLIB_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${ZLIB_LIBRARIES})
    message("LINKED ZLIB")
endif(ZLIB_FOUND)

find_package(LibLZMA)
if (LIBLZMA_FOUND)
    include_directories(${LIBLZMA_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBLZMA_LIBRARIES})
    message("LINKED LIBLZMA")
endif()

find_package(BZip2)
if(BZIP2_FOUND)
    include_directories(${BZIP2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${BZIP2_LIBRARIES})
    message("LINKED BZIP2")
endif(BZIP2_FOUND)

add_subdirectory(../AzgraCppLibrary AzgraCppLibraryObj)


target_link_libraries(${PROJECT_NAME} PRIVATE azgra)
set_property(TARGET ${PROJECT_NAME}  PROPERTY CXX_STANDARD 17)