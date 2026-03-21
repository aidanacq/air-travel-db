#include "DataManager.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <map>
#include <queue>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace OpenFlights {

std::string DataManager::cleanField(const std::string& s) {
    if (s == "\\N" || s == "\\\\N" || s == "-" || s == "N/A") return "";
    return s;
}

int DataManager::safeStoi(const std::string& s, int defaultVal) {
    auto c = cleanField(s);
    if (c.empty()) return defaultVal;
    try { return std::stoi(c); } catch (...) { return defaultVal; }
}

double DataManager::safeStod(const std::string& s, double defaultVal) {
    auto c = cleanField(s);
    if (c.empty()) return defaultVal;
    try { return std::stod(c); } catch (...) { return defaultVal; }
}

float DataManager::safeStof(const std::string& s, float defaultVal) {
    auto c = cleanField(s);
    if (c.empty()) return defaultVal;
    try { return std::stof(c); } catch (...) { return defaultVal; }
}

std::vector<std::string> DataManager::parseCSVLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char ch = line[i];
        if (inQuotes) {
            if (ch == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    field += '"';
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                field += ch;
            }
        } else {
            if (ch == '"') {
                inQuotes = true;
            } else if (ch == ',') {
                fields.push_back(field);
                field.clear();
            } else if (ch != '\r') {
                field += ch;
            }
        }
    }
    fields.push_back(field);
    return fields;
}

double DataManager::haversine(double lat1, double lon1, double lat2, double lon2) {
    constexpr double R = 3959.0;
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;
    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(lat1 * M_PI / 180.0) * std::cos(lat2 * M_PI / 180.0) *
               std::sin(dLon / 2) * std::sin(dLon / 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return R * c;
}

// ── Loading ──────────────────────────────────────────────────────────────────

void DataManager::loadAirlines(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << path << "\n";
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        auto f = parseCSVLine(line);
        if (f.size() < 8) continue;
        Airline a;
        a.id = safeStoi(f[0]);
        if (a.id < 0) continue;
        a.name = cleanField(f[1]);
        a.alias = cleanField(f[2]);
        a.iata = cleanField(f[3]);
        a.icao = cleanField(f[4]);
        a.callsign = cleanField(f[5]);
        a.country = cleanField(f[6]);
        a.active = (f[7] == "Y");
        _airlines[a.id] = std::move(a);
        if (!_airlines[_airlines.size() > 0 ? a.id : 0].iata.empty()) {
            _airlineByIata[_airlines[a.id].iata] = a.id;
        }
    }
    std::cout << "Loaded " << _airlines.size() << " airlines\n";
}

void DataManager::loadAirports(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << path << "\n";
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        auto f = parseCSVLine(line);
        if (f.size() < 14) continue;
        Airport a;
        a.id = safeStoi(f[0]);
        if (a.id < 0) continue;
        a.name = cleanField(f[1]);
        a.city = cleanField(f[2]);
        a.country = cleanField(f[3]);
        a.iata = cleanField(f[4]);
        a.icao = cleanField(f[5]);
        a.latitude = safeStod(f[6]);
        a.longitude = safeStod(f[7]);
        a.altitude = safeStoi(f[8], 0);
        a.timezone = safeStof(f[9]);
        a.dst = cleanField(f[10]);
        a.tz = cleanField(f[11]);
        a.type = cleanField(f[12]);
        a.source = cleanField(f[13]);
        _airports[a.id] = std::move(a);
        if (!_airports[a.id].iata.empty()) {
            _airportByIata[_airports[a.id].iata] = a.id;
        }
    }
    std::cout << "Loaded " << _airports.size() << " airports\n";
}

void DataManager::loadRoutes(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << path << "\n";
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        auto f = parseCSVLine(line);
        if (f.size() < 9) continue;
        Route r;
        r.airlineIata = cleanField(f[0]);
        r.airlineId = safeStoi(f[1]);
        r.sourceAirportIata = cleanField(f[2]);
        r.sourceAirportId = safeStoi(f[3]);
        r.destAirportIata = cleanField(f[4]);
        r.destAirportId = safeStoi(f[5]);
        r.codeshare = (cleanField(f[6]) == "Y");
        r.stops = safeStoi(f[7], 0);
        r.equipment = cleanField(f[8]);
        _routes.push_back(std::move(r));
    }
    std::cout << "Loaded " << _routes.size() << " routes\n";
}

void DataManager::rebuildRouteIndices() {
    _routesByAirlineId.clear();
    _routesBySourceAirportId.clear();
    _routesByDestAirportId.clear();
    for (size_t i = 0; i < _routes.size(); ++i) {
        const auto& r = _routes[i];
        if (r.airlineId > 0) _routesByAirlineId[r.airlineId].push_back(i);
        if (r.sourceAirportId > 0) _routesBySourceAirportId[r.sourceAirportId].push_back(i);
        if (r.destAirportId > 0) _routesByDestAirportId[r.destAirportId].push_back(i);
    }
}

void DataManager::loadAll(const std::string& dataDir) {
    std::lock_guard<std::mutex> lock(_mutex);
    _airlines.clear();
    _airlineByIata.clear();
    _airports.clear();
    _airportByIata.clear();
    _routes.clear();

    loadAirlines(dataDir + "/airlines.dat");
    loadAirports(dataDir + "/airports.dat");
    loadRoutes(dataDir + "/routes.dat");
    rebuildRouteIndices();

    _nextAirlineId = 1;
    for (const auto& [id, _] : _airlines) {
        if (id >= _nextAirlineId) _nextAirlineId = id + 1;
    }
    _nextAirportId = 1;
    for (const auto& [id, _] : _airports) {
        if (id >= _nextAirportId) _nextAirportId = id + 1;
    }
    std::cout << "Data loaded. Next airline ID: " << _nextAirlineId
              << ", next airport ID: " << _nextAirportId << "\n";
}

// ── Entity Retrieval ─────────────────────────────────────────────────────────

json DataManager::getAirlineByIata(const std::string& iata) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _airlineByIata.find(iata);
    if (it == _airlineByIata.end()) {
        return json{{"error", "Airline not found"}, {"iata", iata}};
    }
    return _airlines[it->second].toJson();
}

json DataManager::getAirportByIata(const std::string& iata) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _airportByIata.find(iata);
    if (it == _airportByIata.end()) {
        return json{{"error", "Airport not found"}, {"iata", iata}};
    }
    return _airports[it->second].toJson();
}

// ── Reports ──────────────────────────────────────────────────────────────────

json DataManager::airlineRoutesReport(const std::string& airlineIata) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto ait = _airlineByIata.find(airlineIata);
    if (ait == _airlineByIata.end()) {
        return json{{"error", "Airline not found"}, {"iata", airlineIata}};
    }
    int airlineId = ait->second;
    const auto& airline = _airlines[airlineId];

    std::map<int, int> airportRouteCount;
    auto rit = _routesByAirlineId.find(airlineId);
    if (rit != _routesByAirlineId.end()) {
        for (size_t idx : rit->second) {
            const auto& route = _routes[idx];
            if (route.sourceAirportId > 0) airportRouteCount[route.sourceAirportId]++;
            if (route.destAirportId > 0) airportRouteCount[route.destAirportId]++;
        }
    }

    std::vector<std::pair<int, int>> sorted(airportRouteCount.begin(), airportRouteCount.end());
    std::sort(sorted.begin(), sorted.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    json airports = json::array();
    for (const auto& [apId, count] : sorted) {
        auto apIt = _airports.find(apId);
        if (apIt == _airports.end()) continue;
        auto entry = apIt->second.toJson();
        entry["routeCount"] = count;
        airports.push_back(entry);
    }

    return json{
        {"airline", airline.toJson()},
        {"airports", airports},
        {"totalAirports", airports.size()}
    };
}

json DataManager::airportRoutesReport(const std::string& airportIata) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto ait = _airportByIata.find(airportIata);
    if (ait == _airportByIata.end()) {
        return json{{"error", "Airport not found"}, {"iata", airportIata}};
    }
    int airportId = ait->second;
    const auto& airport = _airports[airportId];

    std::map<int, int> airlineRouteCount;
    auto srcIt = _routesBySourceAirportId.find(airportId);
    if (srcIt != _routesBySourceAirportId.end()) {
        for (size_t idx : srcIt->second) {
            if (_routes[idx].airlineId > 0) airlineRouteCount[_routes[idx].airlineId]++;
        }
    }
    auto dstIt = _routesByDestAirportId.find(airportId);
    if (dstIt != _routesByDestAirportId.end()) {
        for (size_t idx : dstIt->second) {
            if (_routes[idx].airlineId > 0) airlineRouteCount[_routes[idx].airlineId]++;
        }
    }

    std::vector<std::pair<int, int>> sorted(airlineRouteCount.begin(), airlineRouteCount.end());
    std::sort(sorted.begin(), sorted.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    json airlines = json::array();
    for (const auto& [alId, count] : sorted) {
        auto alIt = _airlines.find(alId);
        if (alIt == _airlines.end()) continue;
        auto entry = alIt->second.toJson();
        entry["routeCount"] = count;
        airlines.push_back(entry);
    }

    return json{
        {"airport", airport.toJson()},
        {"airlines", airlines},
        {"totalAirlines", airlines.size()}
    };
}

json DataManager::allAirlinesByIata() {
    std::lock_guard<std::mutex> lock(_mutex);
    std::map<std::string, const Airline*> sorted;
    for (const auto& [id, airline] : _airlines) {
        if (!airline.iata.empty()) {
            sorted[airline.iata] = &airline;
        }
    }
    json result = json::array();
    for (const auto& [iata, airline] : sorted) {
        result.push_back(airline->toJson());
    }
    return json{{"airlines", result}, {"total", result.size()}};
}

json DataManager::allAirportsByIata() {
    std::lock_guard<std::mutex> lock(_mutex);
    std::map<std::string, const Airport*> sorted;
    for (const auto& [id, airport] : _airports) {
        if (!airport.iata.empty()) {
            sorted[airport.iata] = &airport;
        }
    }
    json result = json::array();
    for (const auto& [iata, airport] : sorted) {
        result.push_back(airport->toJson());
    }
    return json{{"airports", result}, {"total", result.size()}};
}

// ── CRUD Airlines ────────────────────────────────────────────────────────────

json DataManager::insertAirline(const json& data) {
    std::lock_guard<std::mutex> lock(_mutex);
    Airline a;
    a.id = data.value("id", _nextAirlineId);
    a.iata = data.value("iata", std::string(""));

    if (_airlines.count(a.id)) {
        return json{{"error", "Airline ID already exists"}, {"id", a.id}};
    }
    if (!a.iata.empty() && _airlineByIata.count(a.iata)) {
        return json{{"error", "Airline IATA already exists"}, {"iata", a.iata}};
    }

    a.name = data.value("name", std::string(""));
    a.alias = data.value("alias", std::string(""));
    a.icao = data.value("icao", std::string(""));
    a.callsign = data.value("callsign", std::string(""));
    a.country = data.value("country", std::string(""));
    a.active = data.value("active", true);

    _airlines[a.id] = a;
    if (!a.iata.empty()) _airlineByIata[a.iata] = a.id;
    if (a.id >= _nextAirlineId) _nextAirlineId = a.id + 1;

    return json{{"success", true}, {"airline", a.toJson()}};
}

json DataManager::modifyAirline(const std::string& iata, const json& data) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _airlineByIata.find(iata);
    if (it == _airlineByIata.end()) {
        return json{{"error", "Airline not found"}, {"iata", iata}};
    }
    auto& a = _airlines[it->second];

    if (data.contains("name")) a.name = data["name"].get<std::string>();
    if (data.contains("alias")) a.alias = data["alias"].get<std::string>();
    if (data.contains("icao")) a.icao = data["icao"].get<std::string>();
    if (data.contains("callsign")) a.callsign = data["callsign"].get<std::string>();
    if (data.contains("country")) a.country = data["country"].get<std::string>();
    if (data.contains("active")) a.active = data["active"].get<bool>();

    return json{{"success", true}, {"airline", a.toJson()}};
}

json DataManager::removeAirline(const std::string& iata) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _airlineByIata.find(iata);
    if (it == _airlineByIata.end()) {
        return json{{"error", "Airline not found"}, {"iata", iata}};
    }
    int airlineId = it->second;

    _routes.erase(
        std::remove_if(_routes.begin(), _routes.end(),
            [airlineId](const Route& r) { return r.airlineId == airlineId; }),
        _routes.end()
    );
    rebuildRouteIndices();

    _airlines.erase(airlineId);
    _airlineByIata.erase(iata);

    return json{{"success", true}, {"removed", iata}};
}

// ── CRUD Airports ────────────────────────────────────────────────────────────

json DataManager::insertAirport(const json& data) {
    std::lock_guard<std::mutex> lock(_mutex);
    Airport a;
    a.id = data.value("id", _nextAirportId);
    a.iata = data.value("iata", std::string(""));

    if (_airports.count(a.id)) {
        return json{{"error", "Airport ID already exists"}, {"id", a.id}};
    }
    if (!a.iata.empty() && _airportByIata.count(a.iata)) {
        return json{{"error", "Airport IATA already exists"}, {"iata", a.iata}};
    }

    a.name = data.value("name", std::string(""));
    a.city = data.value("city", std::string(""));
    a.country = data.value("country", std::string(""));
    a.icao = data.value("icao", std::string(""));
    a.latitude = data.value("latitude", 0.0);
    a.longitude = data.value("longitude", 0.0);
    a.altitude = data.value("altitude", 0);
    a.timezone = data.value("timezone", 0.0f);
    a.dst = data.value("dst", std::string(""));
    a.tz = data.value("tz", std::string(""));
    a.type = data.value("type", std::string("airport"));
    a.source = data.value("source", std::string("User"));

    _airports[a.id] = a;
    if (!a.iata.empty()) _airportByIata[a.iata] = a.id;
    if (a.id >= _nextAirportId) _nextAirportId = a.id + 1;

    return json{{"success", true}, {"airport", a.toJson()}};
}

json DataManager::modifyAirport(const std::string& iata, const json& data) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _airportByIata.find(iata);
    if (it == _airportByIata.end()) {
        return json{{"error", "Airport not found"}, {"iata", iata}};
    }
    auto& a = _airports[it->second];

    if (data.contains("name")) a.name = data["name"].get<std::string>();
    if (data.contains("city")) a.city = data["city"].get<std::string>();
    if (data.contains("country")) a.country = data["country"].get<std::string>();
    if (data.contains("icao")) a.icao = data["icao"].get<std::string>();
    if (data.contains("latitude")) a.latitude = data["latitude"].get<double>();
    if (data.contains("longitude")) a.longitude = data["longitude"].get<double>();
    if (data.contains("altitude")) a.altitude = data["altitude"].get<int>();
    if (data.contains("timezone")) a.timezone = data["timezone"].get<float>();
    if (data.contains("dst")) a.dst = data["dst"].get<std::string>();
    if (data.contains("tz")) a.tz = data["tz"].get<std::string>();

    return json{{"success", true}, {"airport", a.toJson()}};
}

json DataManager::removeAirport(const std::string& iata) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _airportByIata.find(iata);
    if (it == _airportByIata.end()) {
        return json{{"error", "Airport not found"}, {"iata", iata}};
    }
    int airportId = it->second;

    _routes.erase(
        std::remove_if(_routes.begin(), _routes.end(),
            [airportId](const Route& r) {
                return r.sourceAirportId == airportId || r.destAirportId == airportId;
            }),
        _routes.end()
    );
    rebuildRouteIndices();

    _airports.erase(airportId);
    _airportByIata.erase(iata);

    return json{{"success", true}, {"removed", iata}};
}

// ── CRUD Routes ──────────────────────────────────────────────────────────────

json DataManager::insertRoute(const json& data) {
    std::lock_guard<std::mutex> lock(_mutex);
    Route r;
    r.airlineId = data.value("airlineId", -1);
    r.sourceAirportId = data.value("sourceAirportId", -1);
    r.destAirportId = data.value("destAirportId", -1);

    if (!_airlines.count(r.airlineId) && r.airlineId > 0) {
        return json{{"error", "Invalid airline ID"}, {"airlineId", r.airlineId}};
    }
    if (!_airports.count(r.sourceAirportId)) {
        return json{{"error", "Invalid source airport ID"}, {"sourceAirportId", r.sourceAirportId}};
    }
    if (!_airports.count(r.destAirportId)) {
        return json{{"error", "Invalid destination airport ID"}, {"destAirportId", r.destAirportId}};
    }

    if (r.airlineId > 0 && _airlines.count(r.airlineId)) {
        r.airlineIata = _airlines[r.airlineId].iata;
    } else {
        r.airlineIata = data.value("airlineIata", std::string(""));
    }
    r.sourceAirportIata = _airports.count(r.sourceAirportId)
        ? _airports[r.sourceAirportId].iata : data.value("sourceAirportIata", std::string(""));
    r.destAirportIata = _airports.count(r.destAirportId)
        ? _airports[r.destAirportId].iata : data.value("destAirportIata", std::string(""));
    r.codeshare = data.value("codeshare", false);
    r.stops = data.value("stops", 0);
    r.equipment = data.value("equipment", std::string(""));

    _routes.push_back(r);
    rebuildRouteIndices();

    return json{{"success", true}, {"route", r.toJson()}};
}

json DataManager::modifyRoute(int sourceAirportId, int destAirportId, int airlineId, const json& data) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = std::find_if(_routes.begin(), _routes.end(),
        [&](const Route& r) {
            return r.sourceAirportId == sourceAirportId &&
                   r.destAirportId == destAirportId &&
                   r.airlineId == airlineId;
        });
    if (it == _routes.end()) {
        return json{{"error", "Route not found"}};
    }

    if (data.contains("airlineId")) {
        int newAirlineId = data["airlineId"].get<int>();
        if (newAirlineId > 0 && !_airlines.count(newAirlineId)) {
            return json{{"error", "Invalid new airline ID"}, {"airlineId", newAirlineId}};
        }
        it->airlineId = newAirlineId;
        it->airlineIata = (newAirlineId > 0 && _airlines.count(newAirlineId))
            ? _airlines[newAirlineId].iata : "";
    }
    if (data.contains("sourceAirportId")) {
        int newSrcId = data["sourceAirportId"].get<int>();
        if (!_airports.count(newSrcId)) {
            return json{{"error", "Invalid new source airport ID"}, {"sourceAirportId", newSrcId}};
        }
        it->sourceAirportId = newSrcId;
        it->sourceAirportIata = _airports[newSrcId].iata;
    }
    if (data.contains("destAirportId")) {
        int newDstId = data["destAirportId"].get<int>();
        if (!_airports.count(newDstId)) {
            return json{{"error", "Invalid new destination airport ID"}, {"destAirportId", newDstId}};
        }
        it->destAirportId = newDstId;
        it->destAirportIata = _airports[newDstId].iata;
    }
    if (data.contains("stops")) it->stops = data["stops"].get<int>();
    if (data.contains("codeshare")) it->codeshare = data["codeshare"].get<bool>();
    if (data.contains("equipment")) it->equipment = data["equipment"].get<std::string>();

    rebuildRouteIndices();
    return json{{"success", true}, {"route", it->toJson()}};
}

json DataManager::removeRoute(int sourceAirportId, int destAirportId, int airlineId) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = std::find_if(_routes.begin(), _routes.end(),
        [&](const Route& r) {
            return r.sourceAirportId == sourceAirportId &&
                   r.destAirportId == destAirportId &&
                   r.airlineId == airlineId;
        });
    if (it == _routes.end()) {
        return json{{"error", "Route not found"}};
    }
    _routes.erase(it);
    rebuildRouteIndices();

    return json{{"success", true}, {"removed", true}};
}

// ── One-Hop Report ───────────────────────────────────────────────────────────

json DataManager::oneHopReport(const std::string& sourceIata, const std::string& destIata) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto srcIt = _airportByIata.find(sourceIata);
    auto dstIt = _airportByIata.find(destIata);
    if (srcIt == _airportByIata.end()) {
        return json{{"error", "Source airport not found"}, {"iata", sourceIata}};
    }
    if (dstIt == _airportByIata.end()) {
        return json{{"error", "Destination airport not found"}, {"iata", destIata}};
    }

    int srcId = srcIt->second;
    int dstId = dstIt->second;
    const auto& srcAirport = _airports[srcId];
    const auto& dstAirport = _airports[dstId];

    struct RoutePair {
        int intermediateAirportId;
        int airlineId1;
        int airlineId2;
        double leg1Miles;
        double leg2Miles;
        double totalMiles;
    };

    std::vector<RoutePair> pairs;
    std::map<int, int> intermediateCount;

    auto srcRoutes = _routesBySourceAirportId.find(srcId);
    if (srcRoutes == _routesBySourceAirportId.end()) {
        return json{
            {"source", srcAirport.toJson()},
            {"destination", dstAirport.toJson()},
            {"routePairs", json::array()},
            {"topIntermediates", json::array()},
            {"totalPairs", 0}
        };
    }

    std::unordered_map<int, std::vector<int>> intermediateToAirlinesSrcX;
    for (size_t idx : srcRoutes->second) {
        const auto& r = _routes[idx];
        if (r.stops == 0 && r.destAirportId > 0 && r.destAirportId != dstId) {
            intermediateToAirlinesSrcX[r.destAirportId].push_back(r.airlineId);
        }
    }

    for (const auto& [xId, srcAirlines] : intermediateToAirlinesSrcX) {
        auto xRoutes = _routesBySourceAirportId.find(xId);
        if (xRoutes == _routesBySourceAirportId.end()) continue;

        std::vector<int> dstAirlines;
        for (size_t idx : xRoutes->second) {
            const auto& r = _routes[idx];
            if (r.stops == 0 && r.destAirportId == dstId) {
                dstAirlines.push_back(r.airlineId);
            }
        }
        if (dstAirlines.empty()) continue;

        auto xIt = _airports.find(xId);
        if (xIt == _airports.end()) continue;
        const auto& xAirport = xIt->second;

        double leg1 = haversine(srcAirport.latitude, srcAirport.longitude,
                                xAirport.latitude, xAirport.longitude);
        double leg2 = haversine(xAirport.latitude, xAirport.longitude,
                                dstAirport.latitude, dstAirport.longitude);

        for (int al1 : srcAirlines) {
            for (int al2 : dstAirlines) {
                pairs.push_back({xId, al1, al2, leg1, leg2, leg1 + leg2});
                intermediateCount[xId]++;
            }
        }
    }

    std::sort(pairs.begin(), pairs.end(),
              [](const RoutePair& a, const RoutePair& b) { return a.totalMiles < b.totalMiles; });

    json pairsJson = json::array();
    for (const auto& p : pairs) {
        auto xIt = _airports.find(p.intermediateAirportId);
        std::string xName = xIt != _airports.end() ? xIt->second.name : "Unknown";
        std::string xIata = xIt != _airports.end() ? xIt->second.iata : "???";
        std::string al1Name, al2Name;
        auto a1It = _airlines.find(p.airlineId1);
        al1Name = a1It != _airlines.end() ? a1It->second.name : "Unknown";
        auto a2It = _airlines.find(p.airlineId2);
        al2Name = a2It != _airlines.end() ? a2It->second.name : "Unknown";

        pairsJson.push_back({
            {"intermediateIata", xIata},
            {"intermediateName", xName},
            {"intermediateId", p.intermediateAirportId},
            {"airline1", al1Name},
            {"airline1Id", p.airlineId1},
            {"airline2", al2Name},
            {"airline2Id", p.airlineId2},
            {"leg1Miles", std::round(p.leg1Miles * 10) / 10},
            {"leg2Miles", std::round(p.leg2Miles * 10) / 10},
            {"totalMiles", std::round(p.totalMiles * 10) / 10}
        });
    }

    std::vector<std::pair<int, int>> sortedIntermediates(intermediateCount.begin(), intermediateCount.end());
    std::sort(sortedIntermediates.begin(), sortedIntermediates.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    json topJson = json::array();
    int count = 0;
    for (const auto& [apId, cnt] : sortedIntermediates) {
        if (count++ >= 5) break;
        auto apIt = _airports.find(apId);
        if (apIt == _airports.end()) continue;
        topJson.push_back({
            {"airport", apIt->second.toJson()},
            {"routePairCount", cnt}
        });
    }

    return json{
        {"source", srcAirport.toJson()},
        {"destination", dstAirport.toJson()},
        {"routePairs", pairsJson},
        {"topIntermediates", topJson},
        {"totalPairs", pairs.size()}
    };
}

// ── Route Finder ─────────────────────────────────────────────────────────────

json DataManager::findRoutes(const std::string& srcIata, const std::string& dstIata,
                             const std::string& airlineIata,
                             const std::string& backupAirline1Iata,
                             const std::string& backupAirline2Iata,
                             int maxStops, bool exactStops, int numRoutes) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto srcIt = _airportByIata.find(srcIata);
    if (srcIt == _airportByIata.end())
        return json{{"error", "Source airport '" + srcIata + "' not found in the database."}};

    auto dstIt = _airportByIata.find(dstIata);
    if (dstIt == _airportByIata.end())
        return json{{"error", "Destination airport '" + dstIata + "' not found in the database."}};

    int srcId = srcIt->second;
    int dstId = dstIt->second;

    if (srcId == dstId)
        return json{{"error", "Source and destination airports must be different."}};

    if (maxStops < 0 || maxStops > 5)
        return json{{"error", "Number of stops must be between 0 and 5."}};

    if (numRoutes < 1 || numRoutes > 10)
        return json{{"error", "Number of routes must be between 1 and 10."}};

    bool anyAirline = airlineIata.empty();

    struct AirlineInfo { int id; std::string iata; std::string name; };
    std::vector<AirlineInfo> specifiedAirlines;

    if (!anyAirline) {
        auto alIt = _airlineByIata.find(airlineIata);
        if (alIt == _airlineByIata.end())
            return json{{"error", "Primary airline '" + airlineIata + "' not found in the database."}};
        specifiedAirlines.push_back({alIt->second, airlineIata, _airlines[alIt->second].name});

        if (!backupAirline1Iata.empty()) {
            auto ba1 = _airlineByIata.find(backupAirline1Iata);
            if (ba1 == _airlineByIata.end())
                return json{{"error", "Backup airline 1 '" + backupAirline1Iata + "' not found in the database."}};
            specifiedAirlines.push_back({ba1->second, backupAirline1Iata, _airlines[ba1->second].name});
        }

        if (!backupAirline2Iata.empty()) {
            auto ba2 = _airlineByIata.find(backupAirline2Iata);
            if (ba2 == _airlineByIata.end())
                return json{{"error", "Backup airline 2 '" + backupAirline2Iata + "' not found in the database."}};
            specifiedAirlines.push_back({ba2->second, backupAirline2Iata, _airlines[ba2->second].name});
        }
    }

    struct EdgeInfo { std::string alIata; std::string alName; };
    using AdjMap = std::unordered_map<int, std::unordered_map<int, EdgeInfo>>;

    auto buildAdjByAirlines = [&](const std::vector<int>& airlineIds) -> AdjMap {
        AdjMap adj;
        for (int airlineId : airlineIds) {
            auto rit = _routesByAirlineId.find(airlineId);
            if (rit == _routesByAirlineId.end()) continue;
            for (size_t routeIdx : rit->second) {
                const auto& route = _routes[routeIdx];
                if (route.stops != 0) continue;
                int s = route.sourceAirportId;
                int d = route.destAirportId;
                if (s <= 0 || d <= 0) continue;
                if (_airports.find(s) == _airports.end() || _airports.find(d) == _airports.end()) continue;
                if (adj[s].find(d) == adj[s].end()) {
                    std::string name = "Unknown";
                    auto ait = _airlines.find(route.airlineId);
                    if (ait != _airlines.end()) name = ait->second.name;
                    adj[s][d] = {route.airlineIata, name};
                }
            }
        }
        return adj;
    };

    auto buildAdjAll = [&]() -> AdjMap {
        AdjMap adj;
        for (const auto& route : _routes) {
            if (route.stops != 0) continue;
            int s = route.sourceAirportId;
            int d = route.destAirportId;
            if (s <= 0 || d <= 0) continue;
            if (_airports.find(s) == _airports.end() || _airports.find(d) == _airports.end()) continue;
            if (adj[s].find(d) == adj[s].end()) {
                std::string name = "Unknown";
                if (route.airlineId > 0) {
                    auto ait = _airlines.find(route.airlineId);
                    if (ait != _airlines.end()) name = ait->second.name;
                }
                adj[s][d] = {route.airlineIata, name};
            }
        }
        return adj;
    };

    struct FoundPath {
        std::vector<int> airportIds;
        std::vector<std::string> legAlIatas;
        std::vector<std::string> legAlNames;
        std::vector<double> legDists;
        double totalDist;
    };

    int maxLegs = maxStops + 1;

    auto runSearch = [&](const AdjMap& adj, int k) -> std::vector<FoundPath> {
        struct PState {
            double totalDist;
            int cur;
            std::vector<int> airports;
            std::vector<std::string> alIatas;
            std::vector<std::string> alNames;
            std::vector<double> dists;
        };

        auto cmp = [](const PState& a, const PState& b) { return a.totalDist > b.totalDist; };
        std::priority_queue<PState, std::vector<PState>, decltype(cmp)> pq(cmp);
        std::unordered_map<int, int> settleCount;

        pq.push({0.0, srcId, {srcId}, {}, {}, {}});
        std::vector<FoundPath> results;
        int iterations = 0;

        while (!pq.empty() && static_cast<int>(results.size()) < k && iterations++ < 200000) {
            auto st = pq.top();
            pq.pop();

            int legs = static_cast<int>(st.alIatas.size());

            if (st.cur == dstId) {
                bool accept = exactStops ? (legs == maxLegs) : (legs > 0);
                if (accept)
                    results.push_back({st.airports, st.alIatas, st.alNames, st.dists, st.totalDist});
                continue;
            }

            settleCount[st.cur]++;
            if (settleCount[st.cur] > k + 15) continue;
            if (legs >= maxLegs) continue;

            auto adjIt = adj.find(st.cur);
            if (adjIt == adj.end()) continue;
            const auto& curAp = _airports[st.cur];

            for (const auto& [nextId, edge] : adjIt->second) {
                bool inPath = false;
                for (int apId : st.airports) {
                    if (apId == nextId) { inPath = true; break; }
                }
                if (inPath) continue;

                const auto& nextAp = _airports[nextId];
                double dist = haversine(curAp.latitude, curAp.longitude,
                                        nextAp.latitude, nextAp.longitude);
                PState next;
                next.totalDist = st.totalDist + dist;
                next.cur = nextId;
                next.airports = st.airports;
                next.airports.push_back(nextId);
                next.alIatas = st.alIatas;
                next.alIatas.push_back(edge.alIata);
                next.alNames = st.alNames;
                next.alNames.push_back(edge.alName);
                next.dists = st.dists;
                next.dists.push_back(dist);
                pq.push(std::move(next));
            }
        }
        return results;
    };

    std::vector<FoundPath> finalResults;

    if (anyAirline) {
        AdjMap adj = buildAdjAll();
        if (adj.find(srcId) == adj.end())
            return json{{"error", "No routes depart from " + srcIata + " (" + _airports[srcId].name + ") in the database."}};

        bool dstReachable = false;
        for (const auto& [s, dests] : adj) {
            if (dests.find(dstId) != dests.end()) { dstReachable = true; break; }
        }
        if (!dstReachable)
            return json{{"error", "No routes arrive at " + dstIata + " (" + _airports[dstId].name + ") in the database."}};

        finalResults = runSearch(adj, numRoutes);
    } else {
        std::set<std::vector<int>> seenPaths;

        for (int tier = 0; tier < static_cast<int>(specifiedAirlines.size()) &&
             static_cast<int>(finalResults.size()) < numRoutes; ++tier) {
            std::vector<int> tierIds;
            for (int i = 0; i <= tier; ++i)
                tierIds.push_back(specifiedAirlines[i].id);

            AdjMap adj = buildAdjByAirlines(tierIds);

            int searchK = numRoutes + static_cast<int>(seenPaths.size());
            auto tierResults = runSearch(adj, searchK);

            for (const auto& r : tierResults) {
                if (static_cast<int>(finalResults.size()) >= numRoutes) break;
                if (seenPaths.find(r.airportIds) == seenPaths.end()) {
                    seenPaths.insert(r.airportIds);
                    finalResults.push_back(r);
                }
            }
        }
    }

    if (finalResults.empty()) {
        std::string stopsDesc = exactStops
            ? ("exactly " + std::to_string(maxStops))
            : ("up to " + std::to_string(maxStops));
        std::string suggestion = exactStops
            ? "Try switching to 'At most' mode, increasing the number of stops, or adding backup airlines."
            : "Try increasing the number of stops or adding backup airlines.";

        if (anyAirline) {
            return json{{"error", "No routes found between " + srcIata + " and " + dstIata +
                         " with " + stopsDesc + " stop(s) using any airline. " +
                         (exactStops ? "Try switching to 'At most' mode or adjusting the number of stops."
                                     : "Try increasing the number of stops.")}};
        }

        std::string alList = specifiedAirlines[0].name + " (" + specifiedAirlines[0].iata + ")";
        for (size_t i = 1; i < specifiedAirlines.size(); ++i)
            alList += ", " + specifiedAirlines[i].name + " (" + specifiedAirlines[i].iata + ")";

        if (maxStops == 0) {
            return json{{"error", "No direct flights between " + srcIata + " and " + dstIata +
                         " with " + alList + ". " + suggestion}};
        }
        return json{{"error", "No routes found between " + srcIata + " and " + dstIata +
                     " with " + stopsDesc + " stop(s) using " + alList + ". " + suggestion}};
    }

    const auto& srcAirport = _airports[srcId];
    const auto& dstAirport = _airports[dstId];
    double directDist = haversine(srcAirport.latitude, srcAirport.longitude,
                                  dstAirport.latitude, dstAirport.longitude);

    json airlinesJson = json::array();
    if (!anyAirline) {
        for (const auto& al : specifiedAirlines)
            airlinesJson.push_back({{"iata", al.iata}, {"name", al.name}, {"id", al.id}});
    }

    json routesJson = json::array();
    for (int r = 0; r < static_cast<int>(finalResults.size()); ++r) {
        const auto& path = finalResults[r];

        json airportsJson = json::array();
        for (int apId : path.airportIds)
            airportsJson.push_back(_airports[apId].toJson());

        json legsJson = json::array();
        for (int l = 0; l < static_cast<int>(path.legAlIatas.size()); ++l) {
            int fromId = path.airportIds[l];
            int toId = path.airportIds[l + 1];
            const auto& fromAp = _airports[fromId];
            const auto& toAp = _airports[toId];
            legsJson.push_back({
                {"fromIata", fromAp.iata}, {"fromName", fromAp.name},
                {"fromLat", fromAp.latitude}, {"fromLon", fromAp.longitude},
                {"toIata", toAp.iata}, {"toName", toAp.name},
                {"toLat", toAp.latitude}, {"toLon", toAp.longitude},
                {"airlineIata", path.legAlIatas[l]},
                {"airlineName", path.legAlNames[l]},
                {"distance", std::round(path.legDists[l] * 10.0) / 10.0}
            });
        }

        routesJson.push_back({
            {"rank", r + 1},
            {"airports", airportsJson},
            {"legs", legsJson},
            {"totalDistance", std::round(path.totalDist * 10.0) / 10.0},
            {"stops", static_cast<int>(path.legAlIatas.size()) - 1}
        });
    }

    return json{
        {"source", srcAirport.toJson()},
        {"destination", dstAirport.toJson()},
        {"directDistance", std::round(directDist * 10.0) / 10.0},
        {"airlines", airlinesJson},
        {"routes", routesJson},
        {"totalRoutes", finalResults.size()},
        {"maxStopsUsed", maxStops},
        {"exactStops", exactStops},
        {"anyAirline", anyAirline}
    };
}

// ── Stats ────────────────────────────────────────────────────────────────────

json DataManager::getStats() {
    std::lock_guard<std::mutex> lock(_mutex);
    return json{
        {"airlines", _airlines.size()},
        {"airports", _airports.size()},
        {"routes", _routes.size()}
    };
}

} // namespace OpenFlights
