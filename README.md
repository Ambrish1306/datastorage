# Distributed In-Memory Data Storage and Loader

This project implements a C++14 distributed in-memory key-value storage and load pipeline with a mock clustered transport, schema-driven record parsing, deterministic ownership, verification, and large-data benchmarking.

## Project purpose

The application loads configuration from INI files, reads input records from local files, parses them according to a schema, determines the owner node for each key, stores records locally, and verifies final ownership across the cluster. The design intentionally separates configuration, schema parsing, record parsing, partitioning, storage, transfer simulation, statistics, and verification.

## high-level architecture:
![System Architecture](docs/datastorage.drawio.png)

## Low-Level digram 
 

## Prerequisites

- CMake 3.16+
- C++14-compatible compiler
- Unix-like environment or macOS

## Build

```bash
cmake -S . -B build
cmake --build build
```
## Debug Build

```bash 
cd /Users/admin/Documents/myapp/datastorage
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --config Debug
```

You can also use the bundled script:

```bash
./scripts/build.sh
```

## Run

The project includes a simple executable that initializes configuration and validates startup state:

```bash
./build/datastorage
```

## Configuration

Example configuration files are in `config/cluster.ini` and `config/schema.ini`.

- cluster config defines node count and input file paths
- schema config defines field order, names, and types
- the key field is part of the schema and is used for ownership and duplicate handling

## Testing

Run the full validation suite:

```bash
ctest --test-dir build --output-on-failure
```

or:

```bash
./scripts/run_tests.sh
```

## Verification

The verification layer checks ownership, missing records, and duplicate ownership. The project also includes the benchmark harness to validate a >100 MB load.

```bash
./build/benchmark_100mb_test
```

or:

```bash
./scripts/verify.sh
```

## Generated sample data

The repository includes a small sample generation script:

```bash
./scripts/generate_data.sh
```

## Architecture summary

- `core/Config`: cluster configuration parsing and validation
- `core/Schema`: schema parsing and validation
- `record/Record`: generic typed record representation and parser
- `partition/Partitioner`: deterministic node ownership
- `storage/Store`: node-local in-memory storage abstraction
- `network/Transport`: mock transport with queue semantics
- `serialization/Serializer`: length-aware binary format for transfer payloads
- `loader/Loader`: orchestration of the loading pipeline
- `statistics/Statistics`: read/store/transfer metrics
- `verification/Verification`: final ownership verification

## Benchmark notes

The project includes a 100MB+ load benchmark that exercises the real pipeline end-to-end. Measured results in this environment indicate roughly 235k records/sec with verification passing.

## Documentation

Additional docs in the `docs/` directory cover:

- architecture
- design decisions
- algorithms
- performance
- scalability

## Quality gate

This project is considered complete when the build succeeds, all tests pass, the benchmark passes, and verification returns zero incorrect or missing owners.

