# Algorithms

## Partitioning

The current implementation uses a deterministic hash of the record key modulo the node count:

`owner = hash(record.key) % node_count`

The hash is computed over each character in the key to produce a stable 64-bit value. This yields a deterministic owner for each logical record and supports validation after loading.

## Duplicate handling

Duplicate semantics are defined by the key. If a record with the same key is already present in the owner store, it is treated as a duplicate and is not stored again. The system counts duplicates separately from invalid records and records stored.

## Serialization

Transport serialization is length-aware and deterministic. It does not transfer raw object memory. Instead, the wire format uses encoded lengths and explicit payload content so the repository can evolve without relying on machine-specific memory layout.

## Complexity

For N records and K nodes:

- partitioning: O(N)
- table insertion and lookup: average O(1)
- total expected runtime: O(N)
- memory footprint: O(number of records stored locally)

This remains practical for large streaming workloads because the loader reads lines incrementally and avoids loading the complete input dataset into memory.
