cmake_minimum_required(VERSION 3.8)

project(Tonic)

option(REALTIME "Realtime support" ON)
option(ENABLE_WASAPI "Enable Windows Audio Session API support (windows only)" ON)
option(ENABLE_ASIO "Enable ASIO (windows only)" ON)
option(ENABLE_CORE "Enable CoreAudio API support (mac only)" ON)

#========================================#
#========== Realtime Support ============#
#========================================#
if(REALTIME)
    if(ENABLE_JACK)
        find_library(JACK_LIBRARY jack) # find_package(JACK) # TODO: NEED FindJACK.cmake
        if(JACK_LIBRARY)
            message("Jack API found: ${JACK_LIBRARY}")
            link_libraries(${JACK_LIBRARY})
            add_definitions(-D__UNIX_JACK__)
        else()
            message(WARNING "JACK support requires the jack library!")
        endif()
    endif()

    message("${CMAKE_SYSTEM_NAME}")
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)
    link_libraries(Threads::Threads)
    if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
        # TODO: Finish Linux configuration, include different audio API supports
        #==============    LINUX       ================#
        message("Linux DETECTED!")
        if(ENABLE_ALSA)
            find_package(ALSA REQUIRED)
            if(ALSA_FOUND)
                include_directories(${ALSA_INCLUDE_DIRS})
                link_libraries(${ALSA_LIBRARIES})
                add_definitions(-D__LINUX_ALSA__)
            endif()
        endif()
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
        #==============    MAC OS    ================#
        message("Macintosh DETECTED!")
        find_library(COREAUDIO_LIB CoreAudio)
        find_library(COREFOUNDATION_LIB CoreFoundation)
        #find_package(CoreAudio REQUIRED)
        include_directories(${COREAUDIO_INCLUDE_DIRS})
        add_definitions(-D__MACOSX_CORE__)
        link_libraries(${COREAUDIO_LIB} ${COREAUDIO_FOUNDATION_LIB})
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
        # TODO: MORE SUPPORT (e.g., MSYS)?
        # Tested under MSYS2 with Mingw64 toolchain
        #==============      WINDOWS      ================#
        message("Windows DETECTED!")
        link_libraries(winmm ole32 wsock32)
        add_definitions(-D__WINDOWS_MM__)

        if(ENABLE_ASIO)
            message("ENALBING ASIO")
            include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/stk/src/include")
			aux_source_directory("${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/stk/src/include" EXTRA_SRC_LIST)
            add_definitions(-D__WINDOWS_ASIO__)
        endif()

        if(ENABLE_WASAPI)
            message("ENALBING WASAPI")
            link_libraries(mfuuid mfplat wmcodecdspuuid ksuser)
            add_definitions(-D__WINDOWS_WASAPI__)
        endif()
        
        if(ENABLE_DS)
            message("ENALBING Directsound")
            link_libraries(dsound)
            add_definitions(-D__WINDOWS_DS__)
        endif()
    else()
        message("CMAKE_SYSTEM_NAME:" ${CMAKE_SYSTEM_NAME})
        message(FATAL_ERROR "Unknown system type for realtime support.")
    endif()
endif()

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(NOT IS_BIG_ENDIAN)
  add_compile_definitions(__LITTLE_ENDIAN__)
endif()
add_compile_definitions(_USE_MATH_DEFINES)

add_definitions(-w)

message(${CMAKE_CURRENT_SOURCE_DIR})
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../../src/Tonic SRC_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/stk/src STK_SRC_LIST)
add_library(TonicLib ${SRC_LIST} ${STK_SRC_LIST} ${EXTRA_SRC_LIST})
target_include_directories(TonicLib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../submodules/stk/include")
target_include_directories(TonicLib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../src/Tonic")
target_compile_features(TonicLib PRIVATE cxx_std_20)


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
