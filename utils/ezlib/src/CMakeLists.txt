
add_compile_options(-Wall -m32 -march=core2 -fno-builtin-malloc)

include_directories(${EZLIB_INCLUDE}
    ${LIBUV_INCLUDE}
    ${BOOST_INCLUDE_DIRS})

add_library(ez STATIC
    ${EZLIB_INCLUDE}/uv/Loop.hpp
    ${EZLIB_INCLUDE}/uv/Tcp.hpp
    ${EZLIB_INCLUDE}/uv/Timer.hpp
    ${EZLIB_INCLUDE}/BackgroundWorker.hpp
    ${EZLIB_INCLUDE}/BufferPool.hpp
    ${EZLIB_INCLUDE}/FixReader.hpp
    ${EZLIB_INCLUDE}/FixUtils.hpp
    ${EZLIB_INCLUDE}/FixWriter.hpp
    ${EZLIB_INCLUDE}/FixedString.hpp
    ${EZLIB_INCLUDE}/IEvent.hpp
    ${EZLIB_INCLUDE}/IJob.hpp
    ${EZLIB_INCLUDE}/LockQueue.hpp
    ${EZLIB_INCLUDE}/PriceUtil.hpp
    ${EZLIB_INCLUDE}/ProfilingTimer.hpp
    ${EZLIB_INCLUDE}/ReturnCode.hpp
    ${EZLIB_INCLUDE}/StackAllocator.hpp
    ${EZLIB_INCLUDE}/StrUtils.hpp
	uv/Loop.cpp
    uv/Tcp.cpp
    uv/Timer.cpp
    BackgroundWorker.cpp
    BufferPool.cpp
    FixReader.cpp
    PriceUtil.cpp
    StrUtils.cpp
	)
