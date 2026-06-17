cmake_minimum_required(VERSION 3.5.0)
project(my_detection_app)
set(CMAKE_CXX_STANDARD 17)

find_package(HailoRT 5 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(OpenCV REQUIRED opencv4)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0)

set(SYSROOT /media/drive_b/sysroots/armv8a-poky-linux)

add_executable(my_app main.cpp)

target_include_directories(my_app PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
    ${SYSROOT}/usr/include/hailo
    ${SYSROOT}/usr/include
)

target_link_directories(my_app PRIVATE
    ${SYSROOT}/usr/lib
)

target_link_libraries(my_app PRIVATE
    HailoRT::libhailort
    ${OpenCV_LIBRARIES}
    ${GST_LIBRARIES}
    hailo_analytics            
    medialib                   
    pthread
)
target_compile_definitions(my_app PRIVATE
    PERFETTO_NOT_FOUND
)
