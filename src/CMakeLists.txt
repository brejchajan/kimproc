cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)

project(kimproc)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake)

find_package(Eigen3 REQUIRED)
find_package(X11 REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${X11_INCLUDE_DIR})
include_directories(../thirdparty/CImg-1.6.0)
include_directories(../include/argumentparser)
include_directories(../include)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

add_executable(kimproc main.cpp Convolution.cpp GaussianSampler.cpp HarrisCornerDetector.cpp SingleImageHazeRemoval.cpp)

IF(X11_FOUND)
	target_link_libraries(kimproc ${X11_LIBRARIES})
ENDIF()
