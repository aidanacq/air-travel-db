# Air Travel Database Web Application - Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build a C++ web application that serves as a front-end for the OpenFlights Air Travel Database, with full CRUD, reporting, one-hop routing with maps, and a polished modern UI.

**Architecture:** A cpp-httplib HTTP server loads OpenFlights CSV data into in-memory STL containers indexed by both integer IDs and IATA codes. A REST JSON API serves all operations. A static HTML/CSS/JS frontend with Leaflet.js provides interactive maps and a modern single-page experience.

**Tech Stack:** C++17, cpp-httplib (HTTP server), nlohmann/json (JSON), Leaflet.js (maps), HTML5/CSS3/JS

---

## Feature Scope (Extra Credit on All Options)

### Basic Functionality (Section III)
- [x] 1.1 Airline lookup by IATA code
- [x] 1.2 Airport lookup by IATA code
- [x] 2.1a Airline routes report (airports ordered by # routes)
- [x] 2.1b Airport routes report (airlines ordered by # routes)
- [x] 2.2a All airlines ordered by IATA code
- [x] 2.2b All airports ordered by IATA code
- [x] 2.3 Get ID (student name & ID)

### Additional Functionality (Section IV) — All 4 Extensions
- [x] IV.1 CRUD for Airlines, Airports, Routes (in-memory with legality checks)
- [x] IV.2 Get Code (return viewable C++ source)
- [x] IV.3 One-hop report with Haversine distance + top 5 map display
- [x] IV.4 Enhanced Look & Feel (modern UI, maps, animations, About/Contact)

### Documentation (Section V)
- [x] Data structure descriptions for all 3 entities
- [x] Route linkage explanation
- [x] Algorithm description for airline report
- [x] Scalability analysis

---

## Tasks

### Task 1: Project Setup
- Create directory structure: include/, src/, static/, data/, docs/
- Download cpp-httplib header
- Download nlohmann/json header
- Download OpenFlights data files

### Task 2: Data Models
- Create Airline.hpp, Airport.hpp, Route.hpp
- Each with JSON serialization

### Task 3: DataManager
- CSV parser for all 3 entity types
- In-memory storage with STL containers
- Index structures for efficient lookup
- CRUD operations with legality checks
- One-hop route finder with Haversine distance

### Task 4: HTTP Server (main.cpp)
- All REST API endpoints
- Static file serving
- CORS support

### Task 5: Frontend
- Modern responsive HTML/CSS/JS SPA
- Entity lookup forms
- Report tables with sorting
- CRUD management interface
- One-hop route finder with Leaflet map
- About & Contact pages
- Get Code & Get ID features

### Task 6: Build System
- CMakeLists.txt for cross-platform build
- Build script for convenience

### Task 7: Documentation
- Section V data structures writeup
