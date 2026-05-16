cmake_minimum_required(VERSION 3.5.0)

project(infer_usb)

set(CMAKE_CXX_STANDARD 17)

find_package(HailoRT 5 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(OpenCV REQUIRED opencv4)

add_executable(infer_usb inference_usb.cpp)

target_include_directories(infer_usb PRIVATE
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(infer_usb PRIVATE
    HailoRT::libhailort
    ${OpenCV_LIBRARIES}
    pthread
)