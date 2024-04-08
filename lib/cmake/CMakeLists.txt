cmake_minimum_required(VERSION 3.8)

project(Tonic)
message(${CMAKE_CURRENT_SOURCE_DIR})
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../../src/Tonic SRC_LIST)
add_library(TonicLib ${SRC_LIST})

add_definitions(-w)

if(POLICY CMP0074)
    #policy for <PackageName>_ROOT variables
    cmake_policy(SET CMP0074 NEW)
endif()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake-modules")
find_package(FFMPEG COMPONENTS avcodec avformat avutil swresample)
message(${FFMPEG_avcodec_INCLUDE_DIRS})
message(${FFMPEG_avcodec_LIBRARIES})
target_include_directories(TonicLib PUBLIC ${FFMPEG_avcodec_INCLUDE_DIRS})
target_link_libraries(TonicLib FFMPEG::avcodec FFMPEG::avformat FFMPEG::avutil)

FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/../../src/Tonic/*.h")
message(${headers})
INSTALL(FILES ${headers} DESTINATION include/Tonic)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../src/Tonic.h DESTINATION include)

INSTALL(TARGETS TonicLib ARCHIVE DESTINATION lib)