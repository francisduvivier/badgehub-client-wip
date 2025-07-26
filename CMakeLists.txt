cmake_minimum_required(VERSION 3.12.4)
project(lvgl C CXX)

# Set the correct FreeRTOS port for your system (e.g., Posix for WSL)
set(FREERTOS_PORT GCC_POSIX CACHE STRING "Port for FreeRTOS on Posix environment")

unset(USE_FREERTOS CACHE)

option(USE_FREERTOS "Enable FreeRTOS" OFF) # Turn this on to enable FreeRTOS

if(USE_FREERTOS)
    message(STATUS "FreeRTOS is enabled")
    add_library(freertos_config INTERFACE)
    target_include_directories(freertos_config SYSTEM INTERFACE ${PROJECT_SOURCE_DIR}/config)
    target_compile_definitions(freertos_config INTERFACE projCOVERAGE_TEST=0)
    add_subdirectory(FreeRTOS)
    include_directories(${PROJECT_SOURCE_DIR}/FreeRTOS/include)
    include_directories(${PROJECT_SOURCE_DIR}/FreeRTOS/portable/ThirdParty/GCC/Posix)
    include_directories(${PROJECT_SOURCE_DIR}/config)
    file(GLOB FREERTOS_SOURCES
        "${PROJECT_SOURCE_DIR}/FreeRTOS/*.c"
        "${PROJECT_SOURCE_DIR}/FreeRTOS/portable/MemMang/heap_4.c"
        "${PROJECT_SOURCE_DIR}/FreeRTOS/portable/ThirdParty/GCC/Posix/*.c"
    )
else()
    message(STATUS "FreeRTOS is disabled")
    set(FREERTOS_SOURCES "")
endif()

# Main include files of the project
include_directories(${PROJECT_SOURCE_DIR}/main/inc)

# Define options for LVGL
option(LV_USE_DRAW_SDL "Use SDL draw unit" OFF)
option(LV_USE_LIBPNG "Use libpng to decode PNG" OFF)
option(LV_USE_LIBJPEG_TURBO "Use libjpeg turbo to decode JPEG" OFF)
option(LV_USE_FFMPEG "Use libffmpeg to display video using lv_ffmpeg" OFF)
option(LV_USE_FREETYPE "Use freetype library" OFF)

# Set C and C++ standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the output path for the executable
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# Find required packages
find_package(SDL2 REQUIRED)
find_package(CURL REQUIRED)

# Add compile definitions
add_compile_definitions($<$<BOOL:${LV_USE_DRAW_SDL}>:LV_USE_DRAW_SDL=1>)
add_compile_definitions($<$<BOOL:${LV_USE_LIBPNG}>:LV_USE_LIBPNG=1>)
add_compile_definitions($<$<BOOL:${LV_USE_LIBJPEG_TURBO}>:LV_USE_LIBJPEG_TURBO=1>)
add_compile_definitions($<$<BOOL:${LV_USE_FFMPEG}>:LV_USE_FFMPEG=1>)

# Add subdirectories
add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS})
add_subdirectory(cjson)

# Create the main executable
if(USE_FREERTOS)
    add_executable(main
        ${PROJECT_SOURCE_DIR}/main/src/main.c
        ${PROJECT_SOURCE_DIR}/main/src/badgehub_client.c
        ${PROJECT_SOURCE_DIR}/main/src/app_list.c
        ${PROJECT_SOURCE_DIR}/main/src/app_card.c
        ${PROJECT_SOURCE_DIR}/main/src/app_detail.c # Add our new detail source file
        ${PROJECT_SOURCE_DIR}/main/src/freertos_main.cpp
        ${PROJECT_SOURCE_DIR}/main/src/mouse_cursor_icon.c
        ${PROJECT_SOURCE_DIR}/main/src/FreeRTOS_Posix_Port.c
        ${FREERTOS_SOURCES}
    )
    target_link_libraries(main freertos_config FreeRTOS)
else()
    add_executable(main
        ${PROJECT_SOURCE_DIR}/main/src/main.c
        ${PROJECT_SOURCE_DIR}/main/src/badgehub_client.c
        ${PROJECT_SOURCE_DIR}/main/src/app_list.c
        ${PROJECT_SOURCE_DIR}/main/src/app_card.c
        ${PROJECT_SOURCE_DIR}/main/src/app_detail.c # Add our new detail source file
    )
endif()

# Add include directories and link libraries
target_include_directories(main PRIVATE ${PROJECT_SOURCE_DIR}/cjson)
target_compile_definitions(main PRIVATE LV_CONF_INCLUDE_SIMPLE)

if(MSVC)
    target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES} cjson CURL::libcurl)
else()
    target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES} m pthread cjson CURL::libcurl)
endif()

if(USE_FREERTOS)
    target_link_libraries(main freertos_config)
endif()

# Custom target to run the executable
add_custom_target(run COMMAND ${EXECUTABLE_OUTPUT_PATH}/main DEPENDS main)

# Conditional library includes
if(LV_USE_DRAW_SDL)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
    find_package(SDL2_image REQUIRED)
    target_include_directories(lvgl PUBLIC ${SDL2_IMAGE_INCLUDE_DIRS})
    target_link_libraries(main ${SDL2_IMAGE_LIBRARIES})
endif()
if(LV_USE_LIBPNG)
    find_package(PNG REQUIRED)
    target_include_directories(lvgl PUBLIC ${PNG_INCLUDE_DIRS})
    target_link_libraries(main ${PNG_LIBRARIES})
endif()
if(LV_USE_LIBJPEG_TURBO)
    find_package(JPEG REQUIRED)
    target_include_directories(lvgl PUBLIC ${JPEG_INCLUDE_DIRS})
    target_link_libraries(main ${JPEG_LIBRARIES})
endif()
if(LV_USE_FFMPEG)
    target_link_libraries(main avformat avcodec avutil swscale z)
endif()
if(LV_USE_FREETYPE)
    find_package(Freetype REQUIRED)
    target_include_directories(lvgl PUBLIC ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(main ${FREETYPE_LIBRARIES})
endif()

# Windows console settings
if (WIN32)
    if (MSVC)
        target_link_options(main PRIVATE "/SUBSYSTEM:CONSOLE")
    else()
        target_link_options(main PRIVATE "-mconsole")
    endif()
endif()

# Debug build settings
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug mode enabled")
    # Add your debug compile options here
endif()
