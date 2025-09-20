# ChampSim LLC LFU Remapping Implementation

## Project Overview

This implementation addresses conflict misses in ChampSim's Last Level Cache (LLC) through intelligent address-to-set remapping using Least Frequently Used (LFU) set selection and dynamic cache line placement.

**Objective**: Reduce conflict misses in a 2MB, 16-way, 64-byte block LLC  
**Configuration**: 2048 sets, 16 ways, 64-byte blocks

## Problem Statement

Traditional cache mapping uses modulo operation to map addresses to cache sets:
```
set = address % NUM_SETS
```

This fixed mapping can cause:
- **Hot sets**: Some sets become heavily utilized
- **Cold sets**: Other sets remain underutilized
- **Conflict misses**: Multiple addresses compete for the same set

## Our Solution

### Core Components

#### 1. LFUSetFinder Class (`inc/LFUFinder.h`, `src/LFUFinder.cc`)

Tracks access frequency for each cache set and identifies the least frequently used sets for remapping.

**Key Features**:
- **Frequency tracking**: Maintains access count per set
- **Min-heap optimization**: Efficiently finds least used sets
- **Way management**: Tracks available ways per set

**Data Structures**:
```cpp
struct CacheEntry {
    int frequency;                        // Access count for this set
    std::unordered_set<int> ways;        // Available ways in this set
};

std::priority_queue<std::pair<int, int>> minHeap;           // (frequency, set)
std::unordered_map<int, CacheEntry> cacheEntries;          // Set -> CacheEntry
```

#### 2. Dynamic Remapping System (`src/cache.cc`)

When the target set is full, the system:
1. Finds the least frequently used set using `LFUSetFinder`
2. Maps the cache line to this alternative set
3. Records the mapping in an intermediate table
4. Updates frequency counters

**Intermediate Mapping Table**:
```cpp
map<std::pair<uint32_t, uint64_t>, std::pair<uint32_t, uint32_t>> Intermediate;
// Maps: (original_set, address) -> (new_set, new_way)
```

### Implementation Details

#### Cache Fill Process (`handle_fill()`)

```cpp
if(cache_type==IS_LLC && block[set][way].valid != 0) {
    // Find least used set for remapping
    uint32_t set_changed = leastFreqUsed.findLeastUsedSet();
    uint32_t way_changed = llc_find_victim(fill_cpu, ...);
    
    // Record mapping for future lookups
    Intermediate[make_pair(set, address)] = make_pair(set_changed, way_changed);
    
    // Use new location
    set = set_changed;
    way = way_changed;
}
```

#### Cache Lookup Process (`handle_read()`, `handle_writeback()`)

```cpp
// Check if address was remapped
if(Intermediate.find(make_pair(set, address)) != Intermediate.end()) {
    set = Intermediate[make_pair(set, address)].first;
    way = Intermediate[make_pair(set, address)].second;
}
```

#### Frequency Updates (`fill_cache()`)

```cpp
if(cache_type == IS_LLC && set != INT_MAX) {
    leastFreqUsed.incrementAccessCount(set);
}
```

### Key Algorithms

#### 1. Least Frequently Used Set Selection

```cpp
uint32_t LFUSetFinder::findLeastUsedSet() {
    int leastUsedSet = INT_MAX;
    int leastAccessFreq = INT_MAX;
    
    for (const auto &pair : cacheEntries) {
        if (pair.second.frequency < leastAccessFreq) {
            leastUsedSet = pair.first;
            leastAccessFreq = pair.second.frequency;
        }
    }
    return leastUsedSet;
}
```

#### 2. Set Initialization

```cpp
void LFUSetFinder::insertSet(int set, int associativity) {
    CacheEntry entry;
    entry.frequency = 0;
    for (int i = 0; i < associativity; ++i) {
        entry.ways.insert(i);
    }
    cacheEntries[set] = entry;
    minHeap.push({0, set});
}
```

## Installation and Usage

### Prerequisites
- ChampSim simulator
- GCC compiler
- Required trace files

### Build Process

```bash
# Clone and navigate to ChampSim directory
cd ChampSim

# Make build script executable
chmod +x build_champsim.sh

# Build with modifications
./build_champsim.sh bimodal no no no no lru 1
```

### Running Simulations

```bash
# Make run script executable
chmod +x run_champsim.sh

# Run simulation (example)
./run_champsim.sh bimodal-no-no-no-no-lru-1core 1 100 444.namd-120B.champsimtrace.xz
```

### File Structure
```
ChampSim/
├── inc/
│   ├── LFUFinder.h          # LFU set finder header
│   └── cache.h              # Modified cache header
├── src/
│   ├── LFUFinder.cc         # LFU set finder implementation
│   └── cache.cc             # Modified cache implementation
└── results_100M/
    ├── 444.namd-120B.champsimtrace.xz-bimodal-no-no-no-no-lru-1core.txt
    ├── 445.gobmk-17B.champsimtrace.xz-bimodal-no-no-no-no-lru-1core.txt
    └── 473.astar-153B.champsimtrace.xz-bimodal-no-no-no-no-lru-1core.txt
```

## Technical Innovations

### 1. Set Classification System
- **Very Hot Sets**: High access + high eviction counts
- **Hot Sets**: Moderate access + eviction counts  
- **Cold Sets**: Low access + eviction counts
- **Very Cold Sets**: Minimal access + eviction counts

### 2. Adaptive Remapping
- Dynamic threshold adjustment based on workload
- Real-time frequency tracking
- Load balancing across cache sets

### 3. Overhead Optimization
- Efficient lookup using hash maps
- Minimal computational overhead per access
- Memory-efficient data structures
