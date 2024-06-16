cmake_minimum_required(VERSION 3.8)

project(Tonic)

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(NOT IS_BIG_ENDIAN)
  add_compile_definitions(__LITTLE_ENDIAN__)
endif()
add_compile_definitions(_USE_MATH_DEFINES __OS_WINDOWS__)

option(ENABLE_WASAPI "Enable Windows Audio Session API support (windows only)" ON)
if(ENABLE_WASAPI)
    message("ENALBLING WASAPI")
    link_libraries(mfuuid mfplat wmcodecdspuuid ksuser)
    add_definitions(-D__WINDOWS_WASAPI__)
endif()

add_definitions(-w)

message(${CMAKE_CURRENT_SOURCE_DIR})
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../../src/Tonic SRC_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/stk/src STK_SRC_LIST)
add_library(TonicLib ${SRC_LIST} ${STK_SRC_LIST})
target_include_directories(TonicLib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../src/Tonic")
target_include_directories(TonicLib PRIVATE "${CMAKE_SOURCE_DIR}/../../submodules/stk/include")


if(POLICY CMP0074)
    #policy for <PackageName>_ROOT variables
    cmake_policy(SET CMP0074 NEW)
endif()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")
find_package(FFMPEG COMPONENTS avcodec avformat avutil swresample)
message(${FFMPEG_avcodec_INCLUDE_DIRS})
message(${FFMPEG_avcodec_LIBRARIES})
target_include_directories(TonicLib PUBLIC ${FFMPEG_avcodec_INCLUDE_DIRS})
target_link_libraries(TonicLib FFMPEG::avcodec FFMPEG::avformat FFMPEG::avutil)

FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/../../src/Tonic/*.h")
message(${headers})
INSTALL(FILES ${headers} DESTINATION include/Tonic)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../src/Tonic.h DESTINATION include)

FILE(GLOB stk_headers "${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/stk/include/*.h")
message(${stk_headers})
INSTALL(FILES ${stk_headers} DESTINATION include/stk)


INSTALL(TARGETS TonicLib ARCHIVE DESTINATION lib)