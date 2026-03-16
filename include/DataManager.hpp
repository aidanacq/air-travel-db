#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "Models.hpp"

namespace OpenFlights {

class DataManager {
public:
    void loadAll(const std::string& dataDir);

    json getAirlineByIata(const std::string& iata);
    json getAirportByIata(const std::string& iata);

    json airlineRoutesReport(const std::string& airlineIata);
    json airportRoutesReport(const std::string& airportIata);
    json allAirlinesByIata();
    json allAirportsByIata();

    json insertAirline(const json& data);
    json modifyAirline(const std::string& iata, const json& data);
    json removeAirline(const std::string& iata);

    json insertAirport(const json& data);
    json modifyAirport(const std::string& iata, const json& data);
    json removeAirport(const std::string& iata);

    json insertRoute(const json& data);
    json removeRoute(int sourceAirportId, int destAirportId, int airlineId);

    json oneHopReport(const std::string& sourceIata, const std::string& destIata);

    json getStats();

private:
    std::unordered_map<int, Airline> _airlines;
    std::unordered_map<std::string, int> _airlineByIata;
    std::unordered_map<int, Airport> _airports;
    std::unordered_map<std::string, int> _airportByIata;
    std::vector<Route> _routes;

    std::unordered_map<int, std::vector<size_t>> _routesByAirlineId;
    std::unordered_map<int, std::vector<size_t>> _routesBySourceAirportId;
    std::unordered_map<int, std::vector<size_t>> _routesByDestAirportId;

    int _nextAirlineId = 1;
    int _nextAirportId = 1;
    std::mutex _mutex;

    void loadAirlines(const std::string& path);
    void loadAirports(const std::string& path);
    void loadRoutes(const std::string& path);
    void rebuildRouteIndices();

    static std::vector<std::string> parseCSVLine(const std::string& line);
    static int safeStoi(const std::string& s, int defaultVal = -1);
    static double safeStod(const std::string& s, double defaultVal = 0.0);
    static float safeStof(const std::string& s, float defaultVal = 0.0f);
    static double haversine(double lat1, double lon1, double lat2, double lon2);
    static std::string cleanField(const std::string& s);
};

} // namespace OpenFlights
