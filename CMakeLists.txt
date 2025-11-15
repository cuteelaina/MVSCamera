cmake_minimum_required(VERSION 3.10)
project(MVSCamera CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
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

add_executable(capture utils/Capture.cpp)
target_link_libraries(capture PRIVATE MVSCamera)
add_executable(test_camera utils/TestCamera.cpp)
target_link_libraries(test_camera PRIVATE MVSCamera)
add_executable(list_devices utils/ListDevice.cpp)
target_link_libraries(list_devices PRIVATE MVSCamera)
add_executable(calibration utils/Calibration.cpp)
target_link_libraries(calibration PRIVATE MVSCamera)
add_executable(undistort utils/Undistort.cpp)
target_link_libraries(undistort PRIVATE MVSCamera)

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

install(CODE "
    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/setup_env.sh\" \"export MVSCamera_DIR=\\\"${CMAKE_INSTALL_PREFIX}\\\"\")
    execute_process(
        COMMAND chmod +x \"\${CMAKE_INSTALL_PREFIX}/setup_env.sh\"
    )"
)
