cmake_minimum_required(VERSION 3.5.0)

project(Hailo_inference_async)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# HailoRT
find_package(HailoRT 5 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(OpenCV REQUIRED opencv4)


add_executable(infer_app
    main.cpp
    hailo_infer.cpp
    toolbox.cpp
    utils.cpp
)

target_include_directories(infer_app PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(infer_app PRIVATE
    HailoRT::libhailort
    ${OpenCV_LIBRARIES}
    pthread
)
