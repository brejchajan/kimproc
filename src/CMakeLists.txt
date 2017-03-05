cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)

project(kimproc)

#SETTING THE ENVIRONMENT
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake)

# check c++11 / c++0x
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "-std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "-std=c++0x")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11
support. Please use a different C++ compiler.")
endif()

#FIND PACKAGES
find_package(Eigen3 REQUIRED)
find_package(X11 REQUIRED)

#INCLUDE DIRECTORIES
include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${X11_INCLUDE_DIR})
include_directories(../thirdparty/CImg-1.6.0)
include_directories(../include/argumentparser)
include_directories(../include)

#ADDITIONAL CXX FLAGS
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

#EXECUTABLE DEFINITION
add_executable(kimproc main.cpp Convolution.cpp GaussianSampler.cpp HarrisCornerDetector.cpp SingleImageHazeRemoval.cpp GradientStitcher.cpp)

#X11 LINK
IF(X11_FOUND)
	target_link_libraries(kimproc ${X11_LIBRARIES})
ENDIF()

target_link_libraries(kimproc pthread)

#INSTALLATION
install(TARGETS kimproc DESTINATION bin)
