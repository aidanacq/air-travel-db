# OpenFlights Explorer — Air Travel Database Web Application

A C++17 web application that serves as a front-end for the OpenFlights Air Travel Database. Built as the Capstone project for CIS 22C Honor Cohort.

## Quick Start

### Prerequisites
- Windows 10+ with [w64devkit](https://github.com/skeeto/w64devkit/releases) (portable MinGW-w64) extracted to `C:\1projects\w64devkit`

### Build & Run
```powershell
.\build.ps1           # compile
.\airtraveldb.exe     # start server
```

Then open **http://localhost:8080** in your browser.

### Manual Build (if not using build.ps1)
```powershell
$env:PATH = "C:\1projects\w64devkit\bin;$env:PATH"
g++ -std=c++17 -O2 -I include src/DataManager.cpp src/main.cpp -o airtraveldb.exe -lws2_32 -DNOMINMAX -D_WIN32_WINNT=0x0A00
```

## Features

### Basic Functionality
- **Entity Lookup** — Search airlines and airports by IATA code
- **Airline Routes Report** — All airports served by an airline, ordered by route count
- **Airport Routes Report** — All airlines serving an airport, ordered by route count
- **All Airlines/Airports** — Paginated listings ordered by IATA code
- **Get ID** — Returns student name and ID

### Additional Functionality (all 4 extensions)
- **CRUD Operations** — Insert, modify, and delete airlines, airports, and routes with full legality checks (in-memory)
- **Get Code** — View all C++ source code with syntax highlighting
- **One-Hop Route Finder** — Find all single-connection route pairs between two airports with Haversine distance calculation and an interactive Leaflet.js map showing the top 5 intermediate cities
- **Enhanced UI** — Modern dark theme, animated stats, responsive design, tabbed navigation, toast notifications, About & Contact pages

## Architecture

| Layer | Technology |
|-------|-----------|
| HTTP Server | [cpp-httplib](https://github.com/yhirose/cpp-httplib) (header-only) |
| JSON | [nlohmann/json](https://github.com/nlohmann/json) (header-only) |
| Frontend | HTML5, CSS3, Vanilla JavaScript |
| Maps | [Leaflet.js](https://leafletjs.com) + OpenStreetMap |
| Syntax Highlight | [highlight.js](https://highlightjs.org) |

## Data Source

[OpenFlights.org](https://openflights.org/data.php) — copyright-free CSV files:
- `airlines.dat` — 6,161 airlines
- `airports.dat` — 7,698 airports
- `routes.dat` — 67,663 directional routes

## Project Structure

```
capstone/
├── include/
│   ├── httplib.h           # cpp-httplib HTTP server
│   ├── json.hpp            # nlohmann/json
│   ├── Models.hpp          # Airline, Airport, Route structs
│   └── DataManager.hpp     # Data manager class declaration
├── src/
│   ├── DataManager.cpp     # Data loading, indexing, CRUD, reports
│   └── main.cpp            # HTTP server + API endpoints
├── static/
│   ├── index.html          # Frontend SPA
│   ├── style.css           # Dark theme styles
│   └── app.js              # Frontend logic
├── data/
│   ├── airlines.dat
│   ├── airports.dat
│   └── routes.dat
├── docs/
│   ├── plans/
│   └── data-structures.md  # Section V documentation
├── CMakeLists.txt
├── build.ps1
└── README.md
```

## Notes

- All CRUD changes are **in-memory only** — restarting the server resets to original data.
- Update `src/main.cpp` lines in the `/api/id` endpoint with your actual student name and ID before presenting.
