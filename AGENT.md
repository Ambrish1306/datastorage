# AGENT.md
# Distributed In-Memory Data Storage and Loader
# Engineering Guidelines for AI-Assisted Development

You should act live senior developer 

## 1. Project Objective

Build a production-quality C++14 distributed in-memory key-value
storage and data-loading system.

The system must:

1. Support a configurable cluster of 1-5 mock nodes.
2. Read schema and cluster configuration from configuration files.
3. Read large local input files efficiently.
4. Parse records according to the configured schema.
5. Determine the owner node for every record using deterministic
   partitioning/distribution logic.
6. Store records locally when the current node owns them.
7. Transfer records to the owning node through a mock network layer.
8. Maintain statistics for loaded records per node.
9. Provide verification that every record resides on its owner node.
10. Provide clear documentation, scripts, tests and reproducible builds.

The implementation should resemble production-quality senior-level
C++ code rather than a minimal interview solution.

---

# 2. Technology Requirements

## Language

- C++14 minimum.
- Prefer modern C++14 features where appropriate.
- Do not use C++17/20 features unless explicitly approved.
- Use STL extensively.
- Avoid unnecessary third-party dependencies.

## Build

- CMake is mandatory.
- Build must work on Linux.
- Compiler warnings must be enabled.
- Prefer:

  -Wall
  -Wextra
  -Wpedantic
  -Wconversion
  -Wshadow

Warnings should not be ignored without justification.

## Testing

Use a lightweight unit-testing framework such as GoogleTest only if
the project setup remains simple and reproducible.

Otherwise create a small internal test framework.

Tests must cover:

- configuration parsing
- schema validation
- record parsing
- hashing/distribution
- ownership calculation
- duplicate handling
- serialization/deserialization
- storage
- network mock
- statistics
- end-to-end loading
- verification

---

# 3. Core Engineering Principles

The code must follow these principles:

- SOLID
- DRY
- KISS
- YAGNI
- RAII
- Rule of Zero wherever practical
- Exception safety
- Const correctness
- Dependency inversion
- Interface-oriented design
- Single responsibility
- Composition over inheritance where appropriate

Avoid over-engineering.

Every abstraction must have a clear responsibility.

---

# 4. Architecture

Use clear separation between:

1. Configuration
2. Schema
3. Input/Data Reading
4. Parsing
5. Record Representation
6. Partitioning
7. Storage
8. Network
9. Serialization
10. Statistics
11. Verification
12. Application/Orchestration

---

# 5. Suggested Directory Structure

Use a structure similar to:

    distributed-storage/
    |
    +-- CMakeLists.txt
    +-- README.md
    +-- AGENT.md
    +-- LICENSE
    |
    +-- config/
    |   +-- cluster.ini
    |   +-- schema.ini
    |   +-- loader.ini
    |
    +-- docs/
    |   +-- architecture.md
    |   +-- design.md
    |   +-- algorithms.md
    |   +-- performance.md
    |   +-- scalability.md
    |   +-- verification.md
    |
    +-- include/
    |   +-- config/
    |   +-- schema/
    |   +-- storage/
    |   +-- network/
    |   +-- partition/
    |   +-- loader/
    |   +-- statistics/
    |   +-- verification/
    |
    +-- src/
    |   +-- config/
    |   +-- schema/
    |   +-- storage/
    |   +-- network/
    |   +-- partition/
    |   +-- loader/
    |   +-- statistics/
    |   +-- verification/
    |   +-- main.cpp
    |
    +-- tests/
    |   +-- unit/
    |   +-- integration/
    |   +-- fixtures/
    |
    +-- scripts/
    |   +-- build.sh
    |   +-- start_cluster.sh
    |   +-- stop_cluster.sh
    |   +-- generate_data.sh
    |   +-- run_loader.sh
    |   +-- verify.sh
    |   +-- run_tests.sh
    |
    +-- data/
    |   +-- node1/
    |   +-- node2/
    |   +-- node3/
    |   +-- node4/
    |   +-- node5/
    |
    +-- build/
    |
    +-- bin/

Do not mix source code, generated files and input data unnecessarily.

---

# 6. Configuration

Do not hard-code:

- number of nodes
- node IDs
- file paths
- schema
- field names
- field types
- hash algorithm
- network settings
- batch size
- buffer sizes
- thread counts
- logging configuration
- verification settings

Configuration must come from files.

Prefer simple INI-style configuration.

Example:

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

Schema example:

    [schema]
    field_count=3

    [field.1]
    name=id
    type=int32

    [field.2]
    name=name
    type=string

    [field.3]
    name=country
    type=string

The application must validate configuration before starting the load.

Invalid configuration should produce a clear error message and
non-zero exit status.

---

# 7. Data Model

Design a generic Record representation.

Do not create separate hard-coded structures for every possible
schema.

The schema should describe:

- field name
- field type
- field order

Supported types:

- string
- int32

Design the record representation so additional types can be added
later without modifying the entire system.

Potential future types:

- int64
- double
- boolean
- binary

Use strong types and avoid void pointers.

---

# 8. Partitioning

Every record must have exactly one owner node.

Ownership must be deterministic.

For example:

    hash(record.key) % node_count

The partitioning implementation must be isolated behind an interface.

Example conceptual interface:

    class IPartitioner
    {
    public:
        virtual ~IPartitioner() = default;

        virtual NodeId owner(const Record& record) const = 0;
    };

The application must not depend directly on a specific hashing
implementation.

This allows future implementations such as:

- modulo hashing
- consistent hashing
- rendezvous hashing
- range partitioning

without modifying the loader.

Document the selected algorithm and its trade-offs.

---

# 9. Key Handling

Every record must have a deterministic key.

The key definition must come from configuration or the schema rather
than being hard-coded.

The system must define what happens when:

- key is missing
- key is malformed
- duplicate key appears
- duplicate records appear
- record has invalid fields

These behaviors must be documented and configurable where practical.

---

# 10. Duplicate Handling

The input data may contain duplicate records.

Define duplicate semantics explicitly.

Recommended default:

    same key -> same logical record

When duplicate keys are encountered, the system should follow a
documented policy such as:

- last-write-wins
- first-write-wins
- reject duplicate

Do not silently ignore duplicates.

Statistics should distinguish:

- records read
- valid records
- invalid records
- duplicate records
- records stored
- records transferred

---

# 11. Storage Layer

Storage must be abstracted.

Do not make the loader directly manipulate std::unordered_map.

Create an abstraction such as:

    IKeyValueStore

Possible implementation:

    InMemoryKeyValueStore

The interface should support at minimum:

- put
- get
- contains
- size
- clear

The storage implementation must own its data.

Use RAII.

Avoid raw owning pointers.

Prefer:

- std::unique_ptr
- std::shared_ptr only where shared ownership is genuinely required
- references
- values

Do not use shared_ptr by default.

---

# 12. Network Layer

The assignment specifically requires mock POSIX/Berkeley socket
functions.

The business logic must not directly depend on socket calls.

Create an abstraction such as:

    INetworkTransport

and a mock implementation:

    MockSocketTransport

Conceptually:

    connect()
    send()
    recv()
    close()

These should simulate network communication.

The transport layer should allow the loader to behave as if records
are being sent between nodes.

The application must not know whether transport is:

- mock
- TCP
- Unix socket
- future real network implementation

This is an important example of Dependency Inversion.

---

# 13. Serialization

Records transferred between nodes must have an explicit serialized
representation.

Do not transfer C++ object memory directly.

Do not use:

    reinterpret_cast
    memcpy(record_object)

for serialization.

Define:

    serialize(record)
    deserialize(buffer)

Serialization must be:

- deterministic
- length-aware
- safe
- independent of machine pointer sizes
- independent of object memory layout

Document the wire format.

For example:

    [record length]
    [key length]
    [key]
    [field count]
    [field data...]

The format should allow future extension.

---

# 14. Network Efficiency

The implementation should avoid one network operation per record
where possible.

Prefer batching:

    Record
    Record
    Record
       |
       v
    Batch
       |
       v
    Serialize once
       |
       v
    send()

Batch size should be configurable.

Avoid unnecessary copies.

Prefer:

- std::move
- move constructors
- move assignment
- std::string move semantics
- std::vector move semantics

where ownership transfer is appropriate.

Do not use std::move blindly.

---

# 15. Memory Efficiency

The input files may be at least 100 MB.

Never load the entire input file into memory unless explicitly
required.

Use streaming:

    open file
    read line/record
    parse
    determine owner
    store/transfer
    release temporary memory
    continue

Memory consumption should therefore be approximately independent of
the complete input file size.

Avoid unnecessary copies of:

- input lines
- records
- serialized buffers

Large datasets must be handled incrementally.

---

# 16. Exception Safety

All code must follow RAII and exception-safety principles.

Use:

- std::ifstream
- std::ofstream
- STL containers
- smart pointers
- scoped resources

Avoid manual resource management.

Never write:

    new
    delete

unless there is a very strong reason.

Preferred:

    std::make_unique
    std::make_shared

For every public operation, consider:

- no-throw guarantee
- strong exception guarantee
- basic exception guarantee

At minimum, the application must not leak resources when exceptions
occur.

Exceptions should be caught at appropriate application boundaries.

Do not catch exceptions and silently ignore them.

Bad:

    catch (...)
    {
    }

Good:

    catch (const std::exception& ex)
    {
        log error;
        return failure;
    }

---

# 17. Error Handling

Errors must be explicit and actionable.

Examples:

    Failed to open input file:
    data/node1/input.csv

    Invalid schema type:
    float

    Invalid node count:
    7

    Invalid record:
    line 12345

Do not use magic return values such as:

    -1
    -2
    nullptr

when a stronger abstraction is possible.

Use exceptions for exceptional failures and explicit result/status
objects where failure is part of normal control flow.

---

# 18. Design Patterns

Use design patterns only where they solve an actual problem.

Expected useful patterns:

## Strategy Pattern

Partitioning:

    IPartitioner
       |
       +-- ModuloPartitioner

Potential future:

    ConsistentHashPartitioner

## Factory Pattern

Creating components from configuration:

    StorageFactory
    PartitionerFactory
    TransportFactory

## Dependency Injection

Inject dependencies such as:

- storage
- partitioner
- transport
- statistics collector
- configuration

Avoid global singletons.

## Adapter Pattern

If a mock socket API needs to conform to the transport interface,
use an adapter.

Do not introduce patterns merely to demonstrate knowledge.

---

# 19. Concurrency

Concurrency should be introduced only where it improves performance
without unnecessarily complicating correctness.

Potential design:

    Reader
       |
       v
    Parser
       |
       v
    Partition queues
       |
       +----> Node 1
       +----> Node 2
       +----> Node 3

If multithreading is implemented:

- document ownership
- document synchronization
- avoid unnecessary locks
- minimize contention
- prefer thread-local data where appropriate
- protect shared state explicitly

Do not use global mutable state.

If concurrency is not necessary for correctness, keep the first
implementation correct and measurable before optimizing it.

---

# 20. Statistics

The system must provide meaningful statistics.

At minimum:

    Total records read
    Valid records
    Invalid records
    Duplicate records
    Records stored locally
    Records transferred
    Records received
    Records loaded
    Records per node
    Bytes read
    Bytes transferred
    Total load duration
    Records/second

Example:

    Cluster Load Statistics
    -----------------------

    Node 1
      Records loaded : 1,234,567
      Records received: 800,321

    Node 2
      Records loaded : 1,230,451
      Records received: 810,221

    Node 3
      Records loaded : 1,229,992
      Records received: 809,468

    Total records     : 3,695,010
    Invalid records   : 120
    Duplicates        : 4,210
    Duration          : 12.42 sec
    Throughput        : 297,000 records/sec

Statistics must not change the correctness of the storage system.

---

# 21. Verification

Verification is mandatory.

After loading, verify:

1. Every stored record exists on exactly one node.
2. The node is the node calculated by the partitioning algorithm.
3. No record exists on an incorrect node.
4. Duplicate policy is respected.
5. Total records are consistent with statistics.

Provide a verification command/script.

Example:

    ./scripts/verify.sh config/cluster.ini

Expected:

    Verification started...

    Records checked : 3,695,010
    Incorrect owner : 0
    Missing records : 0
    Duplicate owners: 0

    VERIFICATION PASSED

---

# 22. Logging

Use structured and useful logging.

At minimum support:

    INFO
    WARN
    ERROR
    DEBUG

Do not print huge amounts of per-record logging by default.

Bad:

    Processing record 1
    Processing record 2
    Processing record 3
    ...

For large files this will destroy performance.

Detailed logging should be configurable.

---

# 23. Performance

Performance is an explicit grading criterion.

Measure before optimizing.

Important bottlenecks to analyze:

1. File I/O
2. Parsing
3. Hash calculation
4. Memory allocation
5. Serialization
6. Mock network transfer
7. Synchronization
8. Hash-table operations

Document benchmark results.

Do not make unsupported claims such as "O(1)" without explaining
average/worst-case behavior.

---

# 24. Complexity

Document algorithmic complexity.

For N input records and K nodes:

Partitioning:

    O(N)

Average hash-map insertion:

    O(1)

Total expected:

    O(N)

Memory should be approximately:

    O(number of records stored locally)

rather than:

    O(size of complete input dataset)

unless the verification strategy requires otherwise.

---

# 25. C++ Coding Standards

Prefer:

    constexpr
    enum class
    nullptr
    override
    final
    noexcept
    auto when it improves readability
    range-based loops
    uniform initialization
    std::unique_ptr
    std::make_unique
    std::move
    std::string_view is NOT allowed because project targets C++14

Use const wherever possible.

Prefer:

    const std::string&

over:

    std::string

when ownership is not required.

Prefer:

    const auto&

for read-only iteration.

Avoid excessive auto when the type is important for readability.

---

# 26. Ownership Rules

Ownership must always be obvious.

Use:

    T

when ownership/value semantics are appropriate.

Use:

    T&

when the caller owns the object.

Use:

    const T&

when read-only access is needed.

Use:

    std::unique_ptr<T>

for exclusive ownership.

Use:

    std::shared_ptr<T>

only for genuine shared ownership.

Do not use raw pointers for ownership.

Raw pointers may be used as non-owning observers only when clearly
documented.

---

# 27. Interface Rules

Interfaces should be small.

Prefer:

    class IPartitioner
    {
    public:
        virtual ~IPartitioner() = default;

        virtual NodeId owner(const Record&) const = 0;
    };

Avoid giant interfaces.

Each interface should represent one responsibility.

---

# 28. Naming

Use descriptive names.

Prefer:

    calculateOwnerNode()

over:

    calc()

Prefer:

    recordCount

over:

    cnt

Prefer:

    partitioner

over:

    p

Names should communicate intent.

Avoid unnecessary abbreviations.

---

# 29. Comments

Do not comment obvious code.

Bad:

    // Increment i
    ++i;

Good comments explain:

- why something is done
- algorithm decisions
- trade-offs
- non-obvious constraints
- performance considerations

Public interfaces should have documentation where useful.

---

# 30. Documentation Requirements

Maintain:

## README.md

Must explain:

- project purpose
- prerequisites
- build
- configuration
- running
- testing
- verification
- sample output

## docs/architecture.md

Explain:

- components
- responsibilities
- dependencies
- data flow
- node interaction

## docs/design.md

Explain:

- SOLID application
- design patterns
- ownership
- dependency injection
- exception safety

## docs/algorithms.md

Explain:

- partitioning algorithm
- hashing
- duplicate handling
- serialization
- complexity

## docs/performance.md

Explain:

- bottlenecks
- measurements
- optimizations
- memory usage
- throughput

## docs/scalability.md

Explain how the architecture could evolve from:

    1-5 nodes

to:

    100s of nodes

Discuss:

- consistent hashing
- partition metadata
- dynamic node membership
- backpressure
- batching
- parallel loading
- network congestion
- failure handling
- replication
- rebalancing

---

# 31. Scripts

Scripts must make the project easy to run.

Expected:

    ./scripts/build.sh
    ./scripts/generate_data.sh
    ./scripts/start_cluster.sh
    ./scripts/run_loader.sh
    ./scripts/verify.sh
    ./scripts/run_tests.sh

Scripts must:

- use set -e
- validate required arguments
- print useful errors
- avoid hard-coded absolute paths
- work from the repository root
- return meaningful exit codes

---

# 32. Testing Strategy

Tests should exist at multiple levels.

## Unit Tests

Test individual components.

Examples:

    SchemaParserTest
    RecordParserTest
    ModuloPartitionerTest
    InMemoryStoreTest
    SerializerTest
    MockTransportTest

## Integration Tests

Test:

    Reader -> Parser -> Partitioner -> Storage

and:

    Reader -> Parser -> Partitioner -> Network -> Storage

## End-to-End Test

Test:

    configuration
        ->
    cluster startup
        ->
    data loading
        ->
    statistics
        ->
    verification

---

# 33. Testability

Classes should be testable independently.

Avoid:

    global variables
    global state
    static mutable state
    hard-coded filesystem paths
    direct socket calls inside business logic
    direct calls to time/system APIs where avoidable

Use dependency injection.

For example:

    Loader(
        IDataReader&,
        IPartitioner&,
        ITransport&,
        IStatistics&
    );

This allows mocks to be injected during testing.

---

# 34. Security and Robustness

Although this is a mock project:

- validate configuration
- validate input
- validate lengths before deserialization
- avoid buffer overflows
- avoid unchecked allocations
- avoid unsafe casts
- never trust serialized lengths
- handle malformed records gracefully

Do not use:

    strcpy
    strcat
    sprintf

Prefer safe C++ abstractions.

---

# 35. Development Workflow

The implementation should proceed incrementally.

Phase 1:

    Configuration
    Schema
    Record
    Parser

Phase 2:

    Storage
    Partitioner

Phase 3:

    Mock network
    Serialization

Phase 4:

    Loader orchestration

Phase 5:

    Statistics
    Verification

Phase 6:

    Testing

Phase 7:

    Performance optimization

Phase 8:

    Documentation

Do not implement the entire system in one large change.

After each phase:

- build
- test
- review
- commit

---

# 36. AI Agent Rules

The AI agent MUST:

1. Understand the existing architecture before modifying code.
2. Reuse existing abstractions.
3. Avoid unnecessary new classes.
4. Avoid duplicate implementations.
5. Follow C++14.
6. Follow RAII.
7. Follow SOLID.
8. Maintain exception safety.
9. Avoid hard-coded configuration.
10. Write unit tests for non-trivial functionality.
11. Update documentation when architecture changes.
12. Update CMake when files are added.
13. Keep code compilable after each meaningful change.
14. Explain important design decisions.
15. Prefer simple, maintainable solutions.
16. Consider performance for large datasets.
17. Avoid unnecessary copies.
18. Avoid premature optimization.
19. Never silently swallow errors.
20. Never introduce global mutable state.

---

# 37. AI Agent Restrictions

The AI agent MUST NOT:

- copy solutions from external repositories
- introduce code without understanding its ownership/lifetime
- use C++17/20 features
- introduce raw owning pointers
- hard-code cluster topology
- hard-code schema
- hard-code input file locations
- hard-code node count
- hard-code network configuration
- use global mutable state
- create unnecessary singleton classes
- introduce design patterns solely for demonstration
- ignore compiler warnings
- disable tests to make builds pass
- remove failing tests instead of fixing the problem
- silently change externally visible behavior
- make unrelated refactoring changes
- add dependencies without justification

The solution must remain original and purpose-built for this project.

---

# 38. Change Management

Before making a significant change, explain:

1. What is changing?
2. Why is it needed?
3. Which component owns the responsibility?
4. What interfaces are affected?
5. What tests are required?
6. What performance impact is expected?

After implementation verify:

    cmake build
    unit tests
    integration tests
    end-to-end test
