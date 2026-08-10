# Distributed In-Memory Data Storage and Loader

This project is a C++14 mock distributed data-loading system built around a configurable cluster, schema-driven record parsing, deterministic ownership, and verification. It is designed to simulate a distributed storage workflow without requiring a real multi-node network.

## What the project does

The application:

- loads cluster configuration from INI files
- loads schema definitions from INI files
- reads local CSV-style data files from each configured node
- parses records according to the schema
- calculates the owning node using a hash-based partitioner
- stores records locally when the current node owns them
- transfers records through a mock transport layer when needed
- tracks read, store, transfer, and duplicate statistics
- verifies that every record is on its correct owner node

This is a mock cluster implementation, not a real TCP or inter-process distributed service. The "network" layer is simulated in memory and designed to model record movement between nodes.

## High-level architecture

![System Architecture](docs/datastorage.drawio.png)

## Prerequisites

- CMake 3.16+
- C++14-compatible compiler
- Unix-like environment or macOS
- Python 3 (used by the sample data generator)

## Quick start

### 1. Build the project

```bash
cmake -S . -B build
cmake --build build
```

### 2. Generate mock data

The project includes a script to create realistic per-node input files. You can set the size in MB.

```bash
./scripts/generate_data.sh 1
```

This creates files under:

- `data/node1/input.csv`
- `data/node2/input.csv`
- `data/node3/input.csv`

The generated files are intentionally larger than tiny examples and are suitable for exercising the loader pipeline.

### 3. Start the mock cluster workflow

```bash
./scripts/start_cluster.sh 1
```

This script:

- validates the config files
- regenerates mock input data
- runs the app with the cluster and schema configuration
- prints final statistics and verification results

### 4. Run the main program directly

```bash
./build/datastorage --cluster config/cluster.ini --schema config/schema.ini
```

You can also omit the arguments to use the defaults:

```bash
./build/datastorage
```

### 5. Run verification only

```bash
./scripts/verify.sh
```

### 6. Run all tests

```bash
ctest --test-dir build --output-on-failure
```

or:

```bash
./scripts/run_tests.sh
```

## Configuration

The project expects configuration files in INI format.

### Cluster config

Example: `config/cluster.ini`

```ini
[cluster]
node_count=3

[node.1]
id=1
input_file=data/node1/input.csv

[node.2]
id=2
input_file=data/node2/input.csv

[node.3]
id=3
input_file=data/node3/input.csv
```

The loader validates:

- positive node count
- valid node IDs
- non-empty input paths
- matching number of configured nodes and node_count
- duplicate IDs are rejected

### Schema config

Example: `config/schema.ini`

```ini
[schema]
field_count=3
key_field=id

[field.1]
name=id
type=int32

[field.2]
name=name
type=string

[field.3]
name=country
type=string
```

Supported field types:

- `string`
- `int32`

The key field is used for partitioning and duplicate detection.

## Data model

The record model is generic and schema-driven rather than hard-coded for one table structure.

The key design points are:

- schema describes field order, field names, and types
- record parsing is based on the schema definition
- each parsed record stores a key/value representation for ownership decisions
- additional field types can be added later without rewriting the whole pipeline

## Runtime behavior and workflow

The sequence is:

1. Read cluster config
2. Read schema config
3. Build node store and mock transport structures
4. Load each node's local CSV input file
5. Parse each row into a typed record
6. Compute record owner using deterministic hashing
7. Store the record on the owning node
8. If the current node is not the owner, queue the transfer via the mock transport
9. Count valid, invalid, duplicate, and transferred records
10. Run ownership verification across all node stores
11. Print the final summary

## Statistics and verification

The app prints statistics including:

- total records read
- valid records
- invalid records
- duplicate records
- records stored
- records transferred
- records received
- per-node record counts

The verification phase confirms:

- no incorrect owners
- no missing records
- no duplicate owners

Example output:

```text
Verification result: PASS
  Records checked: 34542
  Incorrect owner: 0
  Missing records: 0
  Duplicate owners: 0
```

## Benchmark

The repository includes a 100MB+ benchmark test that generates temporary benchmark files and verifies the full load pipeline.

```bash
./build/benchmark_100mb_test
```

This benchmark is also included in the CTest suite.

## Project structure

- `config/`: cluster and schema configuration files
- `data/`: generated mock node input data
- `include/`: public headers for the core components
- `src/`: implementation for config, parsing, storage, network, loader, verification
- `tests/`: unit and integration tests
- `scripts/`: build, generation, cluster start, verification helpers

## Architecture summary

- `core/Config`: cluster configuration parsing and validation
- `core/Schema`: schema parsing and validation
- `record/Record`: generic typed record representation and parser
- `partition/Partitioner`: deterministic node ownership
- `storage/Store`: node-local in-memory storage abstraction
- `network/Transport`: mock transport with queue semantics
- `serialization/Serializer`: length-aware binary format for transfer payloads
- `loader/Loader`: orchestration of the loading pipeline
- `statistics/Statistics`: metrics and counters
- `verification/Verification`: final ownership verification

## Notes

- This project intentionally simulates a distributed workflow in-process.
- It is designed for learning, validation, and benchmarking, not for production-grade multi-host deployment.
- The internal mock transport is suitable for demonstrating partitioning, ownership, and transfer logic.

## Quality gate

The project is considered healthy when:

- the project builds without errors
- all tests pass in CTest
- the benchmark passes
- verification reports zero incorrect or missing owners
- the generated workflow is reproducible from config and scripts


### Identified Bottlenecks. 

#### 1. ⚠️ **CSV Parsing** (MAJOR BOTTLENECK)

**Location:** [src/record/Record.cpp](src/record/Record.cpp)

**Issue:**
```cpp
while (std::getline(input, line)) {
    processRecord(line, node.id);  // Parse CSV per line
}
```

**Why it's slow:**
- **String allocations**: Each field creates a new string
- **Linear scanning**: Finds commas character-by-character
- **No vectorization**: Single-threaded character processing
- **Multiple passes**: Tokenize → trim → type-convert

**Impact:** ~30-40% of total runtime

**Measured complexity:**
- O(n × m) where n = record count, m = average field length
- ~50-100 CPU cycles per character

**Optimization opportunities:**
1. Use memory-mapped files (`mmap`) to avoid syscall overhead
2. SIMD vectorization for comma detection (AVX2: process 32 chars/instruction)
3. Pre-allocate field buffers (avoid per-record allocation)
4. Zero-copy parsing (views instead of string copies)

**Example improvement:**
```cpp
// Current: O(n) string copies
std::string field = line.substr(start, end - start);

// Better: O(1) string_view (C++17)
std::string_view field(&line[start], end - start);
```

---

#### 2. ⚠️ **Serialization** (MODERATE BOTTLENECK)

**Location:** [src/serialization/Serializer.cpp](src/serialization/Serializer.cpp)

**Issue:**
```cpp
output += encodeLength(keyBytes.size());  // Multiple string concatenations
output += keyBytes;
output += encodeLength(fieldCount);
```

**Why it's slow:**
- **String concatenation**: Each `+=` may reallocate
- **No reserve**: Doesn't pre-allocate output buffer
- **Byte-by-byte encoding**: No bulk operations

**Impact:** ~15-20% of total runtime (only for transferred records)

**Measured complexity:**
- O(k) reallocations where k = field count
- Worst case: O(n²) if reallocation each time

**Optimization:**
```cpp
// Pre-calculate total size
size_t totalSize = calculateSerializedSize(record);
output.reserve(totalSize);  // Single allocation

// Then append without reallocations
```

---
#### 3. ⚠️ **Single-Threaded Processing** (MAJOR SCALABILITY ISSUE)

**Current design:** All processing on one thread

**Why it limits throughput:**
- Modern CPUs have 8-64 cores (unused!)
- I/O bound operations block compute
- No pipeline parallelism

**Measured impact:**
- Single core at 100% CPU
- Other cores idle
- Linear scaling (not scalable)

**Optimization strategy:**
1. **Thread pool** for parallel file reading (one thread per node)
2. **Producer-consumer queue** between parsing and storage
3. **Lock-free data structures** for shared state
4. **NUMA-aware allocation** on multi-socket systems

---

