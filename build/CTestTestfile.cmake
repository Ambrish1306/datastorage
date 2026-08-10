# CMake generated Testfile for 
# Source directory: /Users/admin/Documents/myapp/datastorage
# Build directory: /Users/admin/Documents/myapp/datastorage/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(config_schema_test "/Users/admin/Documents/myapp/datastorage/build/config_schema_test")
set_tests_properties(config_schema_test PROPERTIES  _BACKTRACE_TRIPLES "/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;50;add_test;/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;0;")
add_test(record_parser_test "/Users/admin/Documents/myapp/datastorage/build/record_parser_test")
set_tests_properties(record_parser_test PROPERTIES  _BACKTRACE_TRIPLES "/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;54;add_test;/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;0;")
add_test(partitioner_test "/Users/admin/Documents/myapp/datastorage/build/partitioner_test")
set_tests_properties(partitioner_test PROPERTIES  _BACKTRACE_TRIPLES "/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;58;add_test;/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;0;")
add_test(store_test "/Users/admin/Documents/myapp/datastorage/build/store_test")
set_tests_properties(store_test PROPERTIES  _BACKTRACE_TRIPLES "/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;62;add_test;/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;0;")
add_test(serializer_test "/Users/admin/Documents/myapp/datastorage/build/serializer_test")
set_tests_properties(serializer_test PROPERTIES  _BACKTRACE_TRIPLES "/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;66;add_test;/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;0;")
add_test(transport_test "/Users/admin/Documents/myapp/datastorage/build/transport_test")
set_tests_properties(transport_test PROPERTIES  _BACKTRACE_TRIPLES "/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;70;add_test;/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;0;")
add_test(loader_test "/Users/admin/Documents/myapp/datastorage/build/loader_test")
set_tests_properties(loader_test PROPERTIES  _BACKTRACE_TRIPLES "/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;74;add_test;/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;0;")
add_test(statistics_test "/Users/admin/Documents/myapp/datastorage/build/statistics_test")
set_tests_properties(statistics_test PROPERTIES  _BACKTRACE_TRIPLES "/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;78;add_test;/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;0;")
add_test(verification_test "/Users/admin/Documents/myapp/datastorage/build/verification_test")
set_tests_properties(verification_test PROPERTIES  _BACKTRACE_TRIPLES "/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;82;add_test;/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;0;")
add_test(unit_loader_test "/Users/admin/Documents/myapp/datastorage/build/unit_loader_test")
set_tests_properties(unit_loader_test PROPERTIES  _BACKTRACE_TRIPLES "/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;86;add_test;/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;0;")
add_test(integration_e2e_test "/Users/admin/Documents/myapp/datastorage/build/integration_e2e_test")
set_tests_properties(integration_e2e_test PROPERTIES  _BACKTRACE_TRIPLES "/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;90;add_test;/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;0;")
add_test(cli_verification_test "bash" "-lc" [[tmpdir=$(mktemp -d);         cat > "$tmpdir/cluster.ini" <<'EOF'
[cluster]
node_count=2

[node.1]
id=1
input_file=$tmpdir/node1.csv

[node.2]
id=2
input_file=$tmpdir/node2.csv
EOF
        cat > "$tmpdir/schema.ini" <<'EOF'
[schema]
key_field=id
field_count=2

[field.1]
name=id
type=int32

[field.2]
name=name
type=string
EOF
        printf '1,alice\n2,bob\n' > "$tmpdir/node1.csv";         printf '3,charlie\n4,dana\n' > "$tmpdir/node2.csv";         /Users/admin/Documents/myapp/datastorage/build/datastorage --cluster "$tmpdir/cluster.ini" --schema "$tmpdir/schema.ini" > "$tmpdir/output.txt";         grep -q 'Verification result:' "$tmpdir/output.txt";         grep -q 'Records checked:' "$tmpdir/output.txt";         grep -q 'Incorrect owner:' "$tmpdir/output.txt";         rm -rf "$tmpdir"]])
set_tests_properties(cli_verification_test PROPERTIES  _BACKTRACE_TRIPLES "/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;92;add_test;/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;0;")
add_test(benchmark_100mb_test "/Users/admin/Documents/myapp/datastorage/build/benchmark_100mb_test")
set_tests_properties(benchmark_100mb_test PROPERTIES  _BACKTRACE_TRIPLES "/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;106;add_test;/Users/admin/Documents/myapp/datastorage/CMakeLists.txt;0;")
