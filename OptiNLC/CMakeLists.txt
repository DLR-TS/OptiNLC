cmake_minimum_required(VERSION 3.13)
project(OptiNLC)

set(PACKAGE_VENDOR "DLR TS-KOS-SAI")
set(PACKAGE_DESCRIPTION "${PROJECT_NAME}")
set(PACKAGE_MAINTAINER "DLR TS-KOS-SAI")
set(PACKAGE_MAINTAINER_EMAIL "opensource-ts@dlr.de")

set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -Wextra -O3")
add_definitions(-w) #suppress warnings and only show compilation errors

#add_library(${PROJECT_NAME} INTERFACE)
#add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

include(GNUInstallDirs)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Packing)

set(PROJECT_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include")

find_package(Eigen3 REQUIRED)
find_package(osqp REQUIRED)
find_package(Boost REQUIRED)
find_package(Catch2 REQUIRED)


set(LINKED_LIBRARIES
    Eigen3::Eigen
    osqp::osqp
    ${Boost_LIBRARIES}
)
set(INCLUDE_DIRS
    ${PROJECT_INCLUDE_PATH}
    ${Boost_INCLUDE_DIRS}
)



add_executable(OptiNLC src/OptiNLC_test.cpp)
# target_compile_options(OptiNLC PRIVATE -fpermissive)#REQUIRED for osqp
target_include_directories(
    OptiNLC
    PRIVATE
    ${INCLUDE_DIRS}
)
target_link_libraries(
    OptiNLC
    PRIVATE
    ${LINKED_LIBRARIES}
)

file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
add_executable(${PROJECT_NAME}_TestRunner ${TEST_SOURCES})
#target_compile_options(${PROJECT_NAME}_TestRunner PRIVATE -fpermissive)#REQUIRED for osqp
target_link_libraries(
    ${PROJECT_NAME}_TestRunner
    PRIVATE
    ${LINKED_LIBRARIES}
    Catch2::Catch2
)
target_include_directories(
    ${PROJECT_NAME}_TestRunner
    PRIVATE
    ${INCLUDE_DIRS}
    ${Catch2_INCLUDE_DIRS}
)
include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME}_TestRunner)

include(PackagingInstallTargets)
