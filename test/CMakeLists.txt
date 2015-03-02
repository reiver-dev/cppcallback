cmake_minimum_required(VERSION 3.1)

project("DelegateTest")

file(GLOB SOURCE *.cpp)

add_executable("delegate_test" ${SOURCE})

target_include_directories("delegate_test" PUBLIC "../")
target_compile_options("delegate_test" PUBLIC -O3 -std=c++1y -Wall -Wextra -pedantic -fno-rtti -fno-exceptions -fstack-protector-all)
target_link_libraries("delegate_test" PUBLIC -nodefaultlibs -lc)
