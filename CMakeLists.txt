cmake_minimum_required(VERSION 3.22)
project(tda2_eve_simulation_remote)

#set(CMAKE_CXX_STANDARD 17)
#find_package(OpenCV 4 REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
add_compile_options(-Wmultichar -gdwarf-2 -m32)

add_definitions(-Wmultichar)
add_definitions(-DVCOP_HOST_EMULATION)

#FIND_PATH(MATHGL_INCLUDE_DIR NAMES mgl2/mgl.h
#        PATHS
#        /opt/local/include
#        /usr/include
#        /usr/local/include
#        )
#
#FIND_LIBRARY(MATHGL_LIB NAMES mgl
#        PATHS
#        /opt/local/lib
#        /usr/local/lib
#        /usr/lib
#        )
#
#FIND_LIBRARY(MATHGL_FLTK_LIB NAMES mgl-fltk
#        PATHS
#        /opt/local/lib
#        /usr/local/lib
#        /usr/lib
#        )
#
#IF (MATHGL_INCLUDE_DIR AND MATHGL_LIB)
#    SET(MATHGL_FOUND TRUE)
#    MESSAGE(STATUS "MATHGL found")
#    MESSAGE(STATUS "MATHGL Include dirs:" ${MATHGL_INCLUDE_DIR})
#    MESSAGE(STATUS "MATHGL Library:" ${MATHGL_LIB})
#    MESSAGE(STATUS "MATHGL Library:" ${MATHGL_FLTK_LIB})
#ELSE (MATHGL_INCLUDE_DIR AND MATHGL_LIB)
#    MESSAGE(STATUS "MATHGL was not found")
#ENDIF(MATHGL_INCLUDE_DIR AND MATHGL_LIB)

add_link_options(-m32)
include_directories( kernel/ vcop/ ti/ kernel/fft/inc)
#link_libraries(${MATHGL_LIB} ${MATHGL_FLTK_LIB} /usr/local/lib/libopencv_world.so)

#set(VCOP_KERNEL kernel/vcop_dcoffset_windowing_kernel.k)
set(VCOP_FFT_WRAP kernel/fft/vcop_fft_npt_16ix16o_32inter_wrapper.cpp kernel/fft/vcop_fft_npt_16ix16o_gen_twiddleFactor.cpp kernel/fft/vcop_fft_npt_16ix16o_wrapper.cpp)
add_executable(tda2_eve_simulation_remote main.cpp vcop/vcopmem.cpp vcop/file_io.cpp ${VCOP_FFT_WRAP})
