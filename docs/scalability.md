# Scalability

This repository is intentionally designed for a small cluster, but the architecture supports extension to larger deployments.

## Current range

The existing cluster configuration is intentionally limited to 1-5 nodes, which matches the project’s mock environment and verification targets.

## Future growth

A larger deployment could evolve through:

- consistent hashing instead of modulo hashing
- explicit partition metadata exchanged between nodes
- dynamic membership updates and rebalancing
- batching for transport payloads
- parallel loading with synchronized ownership tracking
- backpressure and network congestion control

## Design principle

The system isolates the partitioning strategy behind `IPartitioner` and keeps all data flow behind explicit configuration, which makes broader node counts and alternative allocation algorithms feasible without redesigning the loader.
