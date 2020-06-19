add_executable(bench bench.cpp get_objects.cpp)

include(thirdparty/nanobench)

target_link_libraries(bench nanobench dze::functional)
