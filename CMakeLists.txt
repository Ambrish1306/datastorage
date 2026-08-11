cmake_minimum_required(VERSION 3.16)

project(datastorage LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
    )
endif()

include(CTest)

add_library(datastorage_core
    src/core/Config.cpp
    src/core/Schema.cpp
    src/record/Record.cpp
    src/partition/Partitioner.cpp
    src/storage/Store.cpp
    src/serialization/Serializer.cpp
    src/network/Transport.cpp
    src/loader/Loader.cpp
    src/statistics/Statistics.cpp
    src/verification/Verification.cpp
)

target_include_directories(datastorage_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(datastorage
    src/main.cpp
)

target_link_libraries(datastorage PRIVATE datastorage_core)

if(BUILD_TESTING)
    add_executable(config_schema_test tests/config_schema_test.cpp)
    target_link_libraries(config_schema_test PRIVATE datastorage_core)
    add_test(NAME config_schema_test COMMAND config_schema_test)

    add_executable(record_parser_test tests/record_parser_test.cpp)
    target_link_libraries(record_parser_test PRIVATE datastorage_core)
    add_test(NAME record_parser_test COMMAND record_parser_test)

    add_executable(partitioner_test tests/partitioner_test.cpp)
    target_link_libraries(partitioner_test PRIVATE datastorage_core)
    add_test(NAME partitioner_test COMMAND partitioner_test)

    add_executable(store_test tests/store_test.cpp)
    target_link_libraries(store_test PRIVATE datastorage_core)
    add_test(NAME store_test COMMAND store_test)

    add_executable(serializer_test tests/serializer_test.cpp)
    target_link_libraries(serializer_test PRIVATE datastorage_core)
    add_test(NAME serializer_test COMMAND serializer_test)

    add_executable(transport_test tests/transport_test.cpp)
    target_link_libraries(transport_test PRIVATE datastorage_core)
    add_test(NAME transport_test COMMAND transport_test)

    add_executable(loader_test tests/loader_test.cpp)
    target_link_libraries(loader_test PRIVATE datastorage_core)
    add_test(NAME loader_test COMMAND loader_test)

    add_executable(statistics_test tests/statistics_test.cpp)
    target_link_libraries(statistics_test PRIVATE datastorage_core)
    add_test(NAME statistics_test COMMAND statistics_test)

    add_executable(verification_test tests/verification_test.cpp)
    target_link_libraries(verification_test PRIVATE datastorage_core)
    add_test(NAME verification_test COMMAND verification_test)

    add_executable(unit_loader_test tests/unit_loader_test.cpp)
    target_link_libraries(unit_loader_test PRIVATE datastorage_core)
    add_test(NAME unit_loader_test COMMAND unit_loader_test)

    add_executable(node_id_flexibility_test tests/node_id_flexibility_test.cpp)
    target_link_libraries(node_id_flexibility_test PRIVATE datastorage_core)
    add_test(NAME node_id_flexibility_test COMMAND node_id_flexibility_test)

    add_executable(integration_e2e_test tests/integration_e2e_test.cpp)
    target_link_libraries(integration_e2e_test PRIVATE datastorage_core)
    add_test(NAME integration_e2e_test COMMAND integration_e2e_test)

    add_test(NAME cli_verification_test
        COMMAND bash -lc
        "tmpdir=$(mktemp -d); \
        cat > \"$tmpdir/cluster.ini\" <<'EOF'\n[cluster]\nnode_count=2\n\n[node.1]\nid=1\ninput_file=$tmpdir/node1.csv\n\n[node.2]\nid=2\ninput_file=$tmpdir/node2.csv\nEOF\n        cat > \"$tmpdir/schema.ini\" <<'EOF'\n[schema]\nkey_field=id\nfield_count=2\n\n[field.1]\nname=id\ntype=int32\n\n[field.2]\nname=name\ntype=string\nEOF\n        printf '1,alice\\n2,bob\\n' > \"$tmpdir/node1.csv\"; \
        printf '3,charlie\\n4,dana\\n' > \"$tmpdir/node2.csv\"; \
        ${CMAKE_BINARY_DIR}/datastorage --cluster \"$tmpdir/cluster.ini\" --schema \"$tmpdir/schema.ini\" > \"$tmpdir/output.txt\"; \
        grep -q 'Verification result:' \"$tmpdir/output.txt\"; \
        grep -q 'Records checked:' \"$tmpdir/output.txt\"; \
        grep -q 'Incorrect owner:' \"$tmpdir/output.txt\"; \
        rm -rf \"$tmpdir\""
    )

    add_executable(benchmark_100mb_test tests/benchmark_100mb_test.cpp)
    target_link_libraries(benchmark_100mb_test PRIVATE datastorage_core)
    add_test(NAME benchmark_100mb_test COMMAND benchmark_100mb_test)
endif()
