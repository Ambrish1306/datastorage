# Architecture

This project is organized around a small set of responsibilities:

- configuration loading and validation
- schema modeling
- record parsing
- ownership calculation
- local storage
- mock transport and serialization
- statistics and verification

The dependency flow is intentionally one-way: configuration feeds orchestration, orchestration drives parsing and ownership, and the verification layer checks final state without changing the storage logic.
