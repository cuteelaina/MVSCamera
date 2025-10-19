cmake_minimum_required(VERSION 3.21)
project(MVSCamera CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)

find_package(OpenCV)
message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

add_library(MVSCamera SHARED
    src/Camera.cpp
    src/CameraException.cpp
    src/CameraInfoList.cpp
    src/CameraManager.cpp
    src/CameraNoExcept.cpp
    src/HandleManager.cpp
)
target_include_directories(MVSCamera PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(MVSCamera PUBLIC ${OpenCV_LIBS})
target_include_directories(MVSCamera PUBLIC /opt/MVS/include)
target_link_libraries(MVSCamera PUBLIC MvCameraControl)
target_link_directories(MVSCamera PUBLIC /opt/MVS/lib/64)

add_executable(capture test/Capture.cpp)
target_link_libraries(capture PRIVATE MVSCamera)
add_executable(list_devices test/ListDevice.cpp)
target_link_libraries(list_devices PRIVATE MVSCamera)

enable_testing()
add_test(NAME ListDevices COMMAND $<TARGET_FILE:list_devices>)
add_test(NAME Capture COMMAND $<TARGET_FILE:capture>)

install(TARGETS MVSCamera
    EXPORT MVSCamera
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(FILES include/MVSCamera.h 
    DESTINATION include
)

install(EXPORT MVSCamera
    FILE MVSCameraConfig.cmake
    DESTINATION lib/cmake/MVSCamera
)
