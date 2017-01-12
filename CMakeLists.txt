cmake_minimum_required (VERSION 2.6) 
find_package (Threads)

include_directories(include)
add_library(CSerialPort src/CSerialLinux.cpp)

#for testing
add_executable (test_serial test.cpp)
target_link_libraries (test_serial CSerialPort ${CMAKE_THREAD_LIBS_INIT})