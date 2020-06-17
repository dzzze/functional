add_executable(bench bench.cpp)

include(thirdparty/nanobench)

target_link_libraries(bench nanobench dze::functional)
