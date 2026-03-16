#pragma once

#include <string>
#include "json.hpp"

using json = nlohmann::json;

namespace OpenFlights {

struct Airline {
    int id = -1;
    std::string name;
    std::string alias;
    std::string iata;
    std::string icao;
    std::string callsign;
    std::string country;
    bool active = false;

    json toJson() const {
        return json{
            {"id", id}, {"name", name}, {"alias", alias},
            {"iata", iata}, {"icao", icao}, {"callsign", callsign},
            {"country", country}, {"active", active}
        };
    }
};

struct Airport {
    int id = -1;
    std::string name;
    std::string city;
    std::string country;
    std::string iata;
    std::string icao;
    double latitude = 0.0;
    double longitude = 0.0;
    int altitude = 0;
    float timezone = 0.0f;
    std::string dst;
    std::string tz;
    std::string type;
    std::string source;

    json toJson() const {
        return json{
            {"id", id}, {"name", name}, {"city", city},
            {"country", country}, {"iata", iata}, {"icao", icao},
            {"latitude", latitude}, {"longitude", longitude},
            {"altitude", altitude}, {"timezone", timezone},
            {"dst", dst}, {"tz", tz}, {"type", type}, {"source", source}
        };
    }
};

struct Route {
    std::string airlineIata;
    int airlineId = -1;
    std::string sourceAirportIata;
    int sourceAirportId = -1;
    std::string destAirportIata;
    int destAirportId = -1;
    bool codeshare = false;
    int stops = 0;
    std::string equipment;

    json toJson() const {
        return json{
            {"airlineIata", airlineIata}, {"airlineId", airlineId},
            {"sourceAirportIata", sourceAirportIata}, {"sourceAirportId", sourceAirportId},
            {"destAirportIata", destAirportIata}, {"destAirportId", destAirportId},
            {"codeshare", codeshare}, {"stops", stops}, {"equipment", equipment}
        };
    }
};

} // namespace OpenFlights
