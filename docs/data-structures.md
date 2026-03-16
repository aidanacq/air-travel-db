# Section V: Data Structures Documentation

## 1. C++ Data Structures for Entity Collections

### Airlines
- **Primary store:** `std::unordered_map<int, Airline>` — maps OpenFlights Airline ID (integer) to the full `Airline` struct. Provides O(1) average-case lookup by ID.
- **IATA index:** `std::unordered_map<std::string, int>` — maps 2-character IATA code strings (e.g. `"AA"`) to the airline's integer ID. Provides O(1) IATA-to-entity resolution.
- **Ordered iteration:** When a sorted report is needed (all airlines by IATA code), entries are copied into a `std::map<std::string, const Airline*>` which sorts by key automatically.

### Airports
- **Primary store:** `std::unordered_map<int, Airport>` — maps OpenFlights Airport ID to the full `Airport` struct. Same O(1) lookup as airlines.
- **IATA index:** `std::unordered_map<std::string, int>` — maps 3/4-character IATA codes (e.g. `"SFO"`) to integer IDs.
- **Ordered iteration:** Same `std::map` approach as airlines when IATA-sorted output is needed.

### Routes
- **Primary store:** `std::vector<Route>` — a flat, contiguous array of all Route structs. Routes have no unique single-field ID, so positional indexing (vector index) serves as the identifier.
- **Airline index:** `std::unordered_map<int, std::vector<size_t>>` — maps an airline ID to a vector of indices into the routes vector. Finding "all routes for airline X" is O(1) lookup + O(R) iteration over that airline's routes.
- **Source airport index:** `std::unordered_map<int, std::vector<size_t>>` — maps a source airport ID to route indices. Enables efficient "routes departing from airport X".
- **Destination airport index:** `std::unordered_map<int, std::vector<size_t>>` — maps a destination airport ID to route indices. Enables efficient "routes arriving at airport X".

## 2. How Routes Are Linked to Airlines and Airports

Routes reference Airlines and Airports via **numeric OpenFlights IDs** (integers). Each Route struct stores three linking fields:

- `airlineId` (int) — the OpenFlights integer ID of the airline operating this route.
- `sourceAirportId` (int) — the OpenFlights integer ID of the departure airport.
- `destAirportId` (int) — the OpenFlights integer ID of the arrival airport.

To resolve a route's airline or airports, the DataManager performs an O(1) lookup into the corresponding `unordered_map<int, Airline>` or `unordered_map<int, Airport>`. The route index maps (airline→routes, airport→routes) enable reverse lookups.

Smart pointers were not used for route linkage because the entities live in stable `unordered_map` containers whose iterators are not invalidated by insertions to other buckets, and the integer-ID approach keeps Route structs small (no pointer overhead) and trivially serializable from the CSV data.

## 3. Algorithm: Airline Report (IATA Code → Airport List Ordered by Route Count)

**Input:** An IATA Airline code `XX`
**Output:** All airports touched by `XX` routes, ordered by descending route count.

### Steps:

1. **Resolve IATA to ID:** Look up `_airlineByIata["XX"]` to get the airline's integer ID. — O(1).

2. **Retrieve route indices:** Look up `_routesByAirlineId[airlineId]` to get a `vector<size_t>` of all route indices for this airline. — O(1).

3. **Count airports:** Iterate over each route index. For each route, increment a counter in a `std::map<int, int>` (airport ID → count) for both the route's `sourceAirportId` and `destAirportId`. — O(R), where R is the number of routes for this airline.

4. **Sort by count:** Copy the map entries into a `vector<pair<int,int>>` and sort by count descending using `std::sort` with a custom comparator. — O(A log A), where A is the number of unique airports.

5. **Build output:** For each (airportId, count) pair, look up the Airport entity in `_airports[airportId]` (O(1) each) and serialize to JSON.

**Total complexity:** O(R + A log A), dominated by the sort step for large A.

**Example output for `AA`:**
```
DFW    Dallas Fort Worth International Airport    364
CLT    Charlotte Douglas International Airport    267
ORD    Chicago O'Hare International Airport       248
```

## 4. Scalability Analysis — Changing Assumptions

### 4a. Each Airline/Airport record increases 1000×
- **Current:** Structs stored inline in `unordered_map` values. At 1000× size, each entity might be several MB.
- **Change needed:** Store entities behind `std::shared_ptr` or use a memory-mapped file as backing store. Add an LRU cache so only frequently accessed entities remain in RAM. Consider columnar storage if only a subset of fields is typically needed.

### 4b. Number of Airlines and Airports increases 1000×
- **Current:** ~6K airlines, ~8K airports fit entirely in RAM with hash-map O(1) lookups.
- **At 1000×:** ~6M airlines, ~8M airports. Hash maps still provide O(1) amortized lookup but memory usage becomes significant (especially with IATA index duplication).
- **Change needed:** Replace `unordered_map` with a disk-backed B-tree or LSM-tree (e.g., RocksDB). Add pagination to all report endpoints since returning millions of records in one response is impractical.

### 4c. Number of Routes increases 1000×
- **Current:** ~67K routes in a flat vector with three index maps.
- **At 1000×:** ~67M routes. The vector and index maps consume significant memory. Route index rebuilds after a single delete become expensive (O(67M)).
- **Change needed:** Replace `vector<Route>` + index maps with a graph database or adjacency-list representation using compressed sparse row (CSR) format. Pre-compute materialized views for common reports (e.g., airport-route-count). Use incremental index updates instead of full rebuilds.

### 4d. Retrieval-to-update ratio changes from 100:1 to 1:100
- **Current design** is optimized for reads: full index rebuilds are acceptable because updates are rare.
- **At 1:100 write ratio:** Index rebuilds after every write become the bottleneck.
- **Change needed:**
  - Replace `std::vector`-based route storage with a balanced BST (`std::set` or `std::multiset`) for O(log n) insert/delete without shifting elements.
  - Replace full index rebuilds with incremental updates: on route insert, append to the relevant index vectors; on route delete, use a tombstone/lazy-delete approach and compact periodically.
  - Add write-ahead logging (WAL) for crash safety.
  - Consider a concurrent data structure (e.g., `tbb::concurrent_hash_map`) or reader-writer locks (`std::shared_mutex`) to allow concurrent reads during writes.
