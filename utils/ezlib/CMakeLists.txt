cmake_minimum_required(VERSION 3.3)

project(ezlib CXX)

# use c++14
set (CMAKE_CXX_STANDARD 14)

# setup boost lib
if(NOT Boost_FOUND)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_NO_SYSTEM_PATHS TRUE)
    if (Boost_NO_SYSTEM_PATHS)
        set(BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/boost")
        set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
        set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/lib")
    endif (Boost_NO_SYSTEM_PATHS)
    find_package(Boost 1.67 REQUIRED
        COMPONENTS date_time)  
endif()

# setup 3rd party lib
set(LIBUV_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/libuv)
set(LIBUV_INCLUDE ${LIBUV_ROOT}/include)

find_library(LIBUV_LIB NAMES libuv.a PATHS ${LIBUV_ROOT}/lib)

# set inlcude path
set(EZLIB_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src)
add_subdirectory(test)

# export include path to global
set_property(GLOBAL PROPERTY LIBUV_INCLUDE ${LIBUV_INCLUDE})
set_property(GLOBAL PROPERTY EZLIB_INCLUDE ${EZLIB_INCLUDE})