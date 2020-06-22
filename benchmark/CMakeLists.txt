add_executable(bench_nanobench bench_nanobench.cpp get_objects.cpp)

include(thirdparty/nanobench)

target_link_libraries(bench_nanobench nanobench dze::functional)

add_executable(bench_google_bench bench_google_bench.cpp get_objects.cpp)

include(thirdparty/google_benchmark)

target_link_libraries(bench_google_bench benchmark::benchmark dze::functional)
