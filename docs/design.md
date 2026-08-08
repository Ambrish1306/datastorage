# Design

## Overview

The project follows a layered, dependency-inverted design. Each component owns one responsibility and communicates through narrow interfaces instead of direct concrete coupling.

## Core responsibilities

- Configuration: reads INI configuration and validates ranges, IDs, and file paths.
- Schema: defines field names and supported field types.
- Record parser: transforms CSV rows into typed records using the schema.
- Partitioner: determines the owner node for a record using a deterministic hash function.
- Storage: persists records in node-local in-memory keyed maps.
- Transport: simulates network delivery with a mock socket abstraction.
- Loader: orchestrates reading, parsing, partitioning, storage, and transfer accounting.
- Verification: inspects the final stored state and validates ownership rules.

## Design patterns

### Strategy

The partitioner is defined as an interface, `IPartitioner`, and implemented by `ModuloPartitioner`. This allows future partitioners without changing the loader.

### Dependency injection

The loader accepts its configuration, schema, and partitioner explicitly. There are no hidden globals or singleton state.

### RAII

File streams, containers, and smart pointers manage lifetime automatically. The system avoids manual resource management and uses standard library ownership patterns.

## Exception safety

The application surfaces failures through exceptions at boundaries such as startup and file loading. The loader catches malformed records and records invalid counts instead of aborting the whole run. This keeps the data pipeline robust while retaining meaningful error reporting.

## Ownership

The loader owns node-local stores and transport instances. The storage layer owns its internal map, and the record model owns only its field data. No component crosses ownership boundaries unexpectedly.
