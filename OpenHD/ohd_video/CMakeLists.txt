cmake_minimum_required(VERSION 3.16.3)
project(OHDVideoLib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(OHDVideoLib STATIC) # initialized below
add_library(OHDVideoLib::OHDVideoLib ALIAS OHDVideoLib)

# Only needed when building this submodule manually
add_subdirectory(../ohd_common commonlib EXCLUDE_FROM_ALL)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

# From libcamera-apps !! begin
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA QUIET libcamera)
if(LIBCAMERA_FOUND)
    message(STATUS "libcamera library found:")
    message(STATUS "    version: ${LIBCAMERA_VERSION}")
    message(STATUS "    libraries: ${LIBCAMERA_LINK_LIBRARIES}")
    message(STATUS "    include path: ${LIBCAMERA_INCLUDE_DIRS}")
else()
    message(STATUS "LIBCAMERA features will be unavailable!")
endif()

# From libcamera-apps !! end

# FUCKING GSTREAMER !! begin
find_package(PkgConfig REQUIRED)
pkg_search_module(GST REQUIRED gstreamer-1.0>=1.4
        gstreamer-sdp-1.0>=1.4
        gstreamer-video-1.0>=1.4
        gstreamer-app-1.0>=1.4)
## xx
# https://stackoverflow.com/questions/64078308/how-to-write-a-cmake-file-which-successfully-links-gstreamer-basic-tutorial-5
pkg_search_module(gstreamer REQUIRED IMPORTED_TARGET gstreamer-1.0>=1.4)
pkg_search_module(gstreamer-sdp REQUIRED IMPORTED_TARGET gstreamer-sdp-1.0>=1.4)
pkg_search_module(gstreamer-app REQUIRED IMPORTED_TARGET gstreamer-app-1.0>=1.4)
pkg_search_module(gstreamer-video REQUIRED IMPORTED_TARGET gstreamer-video-1.0>=1.4)
target_link_libraries(OHDVideoLib PRIVATE
        PkgConfig::gstreamer
        PkgConfig::gstreamer-sdp
        PkgConfig::gstreamer-app
        PkgConfig::gstreamer-video)
## xx
# FUCKING GSTREAMER !! end
# We get fmt from speedlog
#find_package(fmt REQUIRED)
#SET(OHD_VIDEO_USED_LIBS ${OHD_VIDEO_USED_LIBS} fmt::fmt)

#find_package(spdlog REQUIRED)
#SET(OHD_VIDEO_USED_LIBS ${OHD_VIDEO_USED_LIBS} spdlog::spdlog)

find_package(Threads REQUIRED)

set(sources 
    "inc/camerastream.h"
    "inc/camera_discovery.h"
    "inc/gstreamerstream.h"
    "src/libcamera_detect.hpp"
    "inc/gst_helper.hpp"
    #inc/gst_recorder.h
    inc/gst_recording_demuxer.h
    "inc/ohd_video_air.h"
    "inc/camera.hpp"
    "inc/camera_enums.hpp"
    "inc/camera_holder.hpp"
    "inc/camera_settings.hpp"
    "inc/rtp_eof_helper.h"
    "inc/v_validate_settings.h"
    inc/ohd_video_ground.h
    inc/openhd-rpi-os-configure-vendor-cam.hpp

    "src/camera_discovery_helper.hpp"
    "src/ffmpeg_videosamples.hpp"

    "src/camerastream.cpp"
    "src/camera_discovery.cpp"
    "src/gstreamerstream.cpp"
    "src/ohd_video_air.cpp"
    "src/rtp_eof_helper.cpp"
    src/ohd_video_ground.cpp
    #src/gst_recorder.cpp
     src/gst_recording_demuxer.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})
target_sources(OHDVideoLib PRIVATE ${sources})
target_include_directories(OHDVideoLib 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>)

target_link_libraries(OHDVideoLib PUBLIC OHDCommonLib)

if(LIBCAMERA_FOUND)
    target_include_directories(OHDVideoLib PRIVATE ${LIBCAMERA_INCLUDE_DIRS})
    target_link_libraries(OHDVideoLib PRIVATE ${LIBCAMERA_LINK_LIBRARIES})
    target_compile_definitions(OHDVideoLib PUBLIC OPENHD_LIBCAMERA_PRESENT)
endif()

# disable compiler warning since we don't link against GCC before 4.4
target_compile_options(OHDVideoLib PRIVATE -Wno-packed-bitfield-compat)

target_link_libraries(OHDVideoLib PUBLIC ${OHD_VIDEO_USED_LIBS})
# link and include gstreamer
target_include_directories(OHDVideoLib PUBLIC ${GST_INCLUDE_DIRS})
target_link_libraries(OHDVideoLib PUBLIC ${GST_LIBRARIES})
# find and include all the other libs

target_link_libraries(OHDVideoLib PUBLIC Threads::Threads)

include(cmake/FindLibUSB.cmake)
target_include_directories(OHDVideoLib PRIVATE ${LIBUSB_1_INCLUDE_DIRS})
target_link_libraries(OHDVideoLib PRIVATE ${LIBUSB_1_LIBRARIES})

include(cmake/FindV4L2.cmake)
target_include_directories(OHDVideoLib PRIVATE ${V4L2_INCLUDE_DIR})
target_link_libraries(OHDVideoLib PRIVATE ${V4L2_LIBRARY})

# Executables for testing manually, note that some might need to run after discovery
add_executable(test_video test/test_video.cpp)
target_link_libraries(test_video OHDVideoLib)
add_executable(test_dummy_gstreamer test/test_dummy_gstreamer.cpp)
target_link_libraries(test_dummy_gstreamer OHDVideoLib)
