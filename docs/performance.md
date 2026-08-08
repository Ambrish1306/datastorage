# Performance

## Benchmark results

The project includes a 100MB+ synthetic benchmark in `tests/benchmark_100mb_test.cpp`. The benchmark generates a >100 MB dataset, loads it through the configured pipeline, and verifies final ownership.

Measured results in the current environment:

- target payload: 104,857,600 bytes
- records loaded: 4,655,644
- records stored: 3,327,822
- verification passed: yes
- elapsed time: 19,813 ms
- throughput: 234,979 records/sec

## Bottlenecks considered

The main bottlenecks in this design are:

1. file I/O
2. parsing
3. hashing and key handling
4. map insertion
5. serialized transfer logic
6. verification pass

The implementation addresses the largest parser overhead by reusing the parser and reducing per-record stringstream work. This substantially lowers the cost of the main load loop without affecting correctness.

## Memory profile

The loader operates streaming-style: it reads one record at a time, parses it, computes ownership, stores or transfers it, and then releases the temporary record. This keeps memory use closer to the data actually retained for local storage than to the full file size.
