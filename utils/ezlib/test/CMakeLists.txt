
add_compile_options(-Wall -m32 -march=core2 -fno-builtin-malloc)

include_directories(${EZLIB_INCLUDE}
    ${LIBUV_INCLUDE}
    ${BOOST_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ez_test
    main.cpp
    uv_Loop.cpp
    ez_BackgroundWorker.cpp
    ez_BufferPool.cpp
    ez_FixReader.cpp
    ez_FixUtils.cpp
    ez_FixWriter.cpp
    ez_FixedString.cpp
    ez_IEvent.cpp
    ez_PriceUtil.cpp
    ez_StackAllocator.cpp
    ez_StrUtils.cpp
    ez_ReturnCode.cpp
    )
target_link_libraries(ez_test ez ${LIBUV_LIB} pthread rt)

set_property(TARGET ez_test APPEND_STRING PROPERTY LINK_FLAGS "-m32")