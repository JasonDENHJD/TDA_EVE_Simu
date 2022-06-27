cmake_minimum_required(VERSION 3.22)
project(tda2_eve_simulation_remote)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
add_compile_options(-Wmultichar -gdwarf-2 -m32)


add_definitions(-DVCOP_HOST_EMULATION)


add_link_options(-m32)
add_executable(tda2_eve_simulation_remote ti_file_io.c vcop_peak_detection_config.c vcop_peak_detection_tb.cpp configparser.cpp)
#target_link_libraries(${PROJECT_NAME} "-Wl,-no-whole-archive"  ${PROJECT_SOURCE_DIR}/libeveprivnatckernels.eve.lib_pc.lib)