# Node Identity Bug: Visual Example

## Configuration
```ini
[cluster]
node_count=3

[node.1]
id=10
input_file=data/node1/input.csv

[node.2]  
id=25
input_file=data/node2/input.csv

[node.3]
id=100
input_file=data/node3/input.csv
```

## Data Structures

```
Array Index:    0         1         2
               ───       ───       ───
stores_[]     |   |     |   |     |   |
              └───┘     └───┘     └───┘

Configured ID:  10        25        100
```

## Example Record Processing

**Scenario:** Node 10 reads record with key="alice"

### Step-by-Step

1. **Parse record**
   ```
   key = "alice"
   ```

2. **Calculate owner (partitioner)**
   ```cpp
   hash("alice") = 12345
   owner = 12345 % 3 = 0    // Array index!
   ```

3. **Get array positions**
   ```cpp
   ownerIndex = 0           // stores_[0]
   localIndex = 0           // node 10 is at position 0
   ```

4. **Store the record**
   ```cpp
   stores_[0]->put(record)  // Stored at position 0 ✅
   ```

5. **Check if transfer needed (THE BUG)**

   **BEFORE (buggy):**
   ```cpp
   if (owner != nodeId)     // if (0 != 10)  ❌
       // ALWAYS TRUE - even though both are position 0!
       recordsTransferred++
   ```
   
   **AFTER (fixed):**
   ```cpp
   ownerNodeId = clusterConfig_.nodes[0].id  // = 10
   if (ownerNodeId != nodeId)                // if (10 != 10)
       // FALSE - correctly identified as local!
       recordsTransferred++
   ```

---

## Complete Example: All 3 Scenarios

### Scenario A: Record owned by Node 10 (index 0)

```
Reading node: 10
Record key: "alice"
Hash: hash("alice") % 3 = 0

┌─────────────────────────────────────────────────┐
│ Partitioner output: owner = 0                   │
│ Array position: stores_[0]                      │
│ Owner node ID: clusterConfig_.nodes[0].id = 10  │
│ Reading node ID: 10                             │
│                                                  │
│ ❌ BUGGY: if (0 != 10) → TRUE → transferred++   │
│ ✅ FIXED: if (10 != 10) → FALSE → LOCAL         │
└─────────────────────────────────────────────────┘
```

### Scenario B: Record owned by Node 25 (index 1)

```
Reading node: 10
Record key: "bob"
Hash: hash("bob") % 3 = 1

┌─────────────────────────────────────────────────┐
│ Partitioner output: owner = 1                   │
│ Array position: stores_[1]                      │
│ Owner node ID: clusterConfig_.nodes[1].id = 25  │
│ Reading node ID: 10                             │
│                                                  │
│ ❌ BUGGY: if (1 != 10) → TRUE → transferred++   │
│ ✅ FIXED: if (25 != 10) → TRUE → REMOTE ✓       │
└─────────────────────────────────────────────────┘
```

### Scenario C: Record owned by Node 100 (index 2)

```
Reading node: 10
Record key: "charlie"
Hash: hash("charlie") % 3 = 2

┌─────────────────────────────────────────────────┐
│ Partitioner output: owner = 2                   │
│ Array position: stores_[2]                      │
│ Owner node ID: clusterConfig_.nodes[2].id = 100 │
│ Reading node ID: 10                             │
│                                                  │
│ ❌ BUGGY: if (2 != 10) → TRUE → transferred++   │
│ ✅ FIXED: if (100 != 10) → TRUE → REMOTE ✓      │
└─────────────────────────────────────────────────┘
```

---

## Statistics Impact

**Sample data:** 9 records read by node 10

**Expected distribution** (hash-based):
- ~3 records → Node 10 (local)
- ~3 records → Node 25 (remote, transfer)
- ~3 records → Node 100 (remote, transfer)

### Results

| Implementation | Local (owner=10) | Remote (owner≠10) | recordsTransferred |
|----------------|------------------|-------------------|--------------------|
| **BUGGY**      | Marked as remote | Marked as remote  | 9 ❌ (100%)        |
| **FIXED**      | Correctly local  | Correctly remote  | ~6 ✅ (67%)        |

---

## Key Insight

The bug confused **two different coordinate systems**:
1. **Array indices**: 0, 1, 2 (how we access stores_[])
2. **Node IDs**: 10, 25, 100 (what's in cluster.ini)

The fix: Always compare values in the **same coordinate system**.

```
BAD:  if (array_index != node_id)       // Comparing apples to oranges
GOOD: if (owner_node_id != node_id)     // Both are node IDs
```
