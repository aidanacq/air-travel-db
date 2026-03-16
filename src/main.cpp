#include "httplib.h"
#include "json.hpp"
#include "DataManager.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

using json = nlohmann::json;

static std::string readFileToString(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) return "";
    return std::string(std::istreambuf_iterator<char>(ifs), {});
}

int main() {
    OpenFlights::DataManager db;
    std::cout << "Loading OpenFlights data...\n";
    db.loadAll("data");
    std::cout << "Data loaded successfully.\n";

    httplib::Server svr;

    svr.set_mount_point("/", "static");

    svr.Options(R"(.*)", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 204;
    });

    auto setCors = [](httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
    };

    // ── Entity Retrieval ─────────────────────────────────────────────────────

    svr.Get(R"(/api/airline/([A-Za-z0-9]+))", [&](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        auto result = db.getAirlineByIata(req.matches[1]);
        res.status = result.contains("error") ? 404 : 200;
        res.set_content(result.dump(), "application/json");
    });

    svr.Get(R"(/api/airport/([A-Za-z0-9]+))", [&](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        auto result = db.getAirportByIata(req.matches[1]);
        res.status = result.contains("error") ? 404 : 200;
        res.set_content(result.dump(), "application/json");
    });

    // ── Reports ──────────────────────────────────────────────────────────────

    svr.Get(R"(/api/report/airline-routes/([A-Za-z0-9]+))", [&](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        auto result = db.airlineRoutesReport(req.matches[1]);
        res.status = result.contains("error") ? 404 : 200;
        res.set_content(result.dump(), "application/json");
    });

    svr.Get(R"(/api/report/airport-routes/([A-Za-z0-9]+))", [&](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        auto result = db.airportRoutesReport(req.matches[1]);
        res.status = result.contains("error") ? 404 : 200;
        res.set_content(result.dump(), "application/json");
    });

    svr.Get("/api/report/airlines", [&](const httplib::Request&, httplib::Response& res) {
        setCors(res);
        res.set_content(db.allAirlinesByIata().dump(), "application/json");
    });

    svr.Get("/api/report/airports", [&](const httplib::Request&, httplib::Response& res) {
        setCors(res);
        res.set_content(db.allAirportsByIata().dump(), "application/json");
    });

    // ── Student ID ───────────────────────────────────────────────────────────

    svr.Get("/api/id", [&](const httplib::Request&, httplib::Response& res) {
        setCors(res);
        json result = {
            {"name", "Aidan Acquistapace"},
            {"studentId", "20529765"},
            {"course", "CIS 22C Honor Cohort"},
            {"project", "Air Travel Database Capstone"}
        };
        res.set_content(result.dump(), "application/json");
    });

    // ── Stats ────────────────────────────────────────────────────────────────

    svr.Get("/api/stats", [&](const httplib::Request&, httplib::Response& res) {
        setCors(res);
        res.set_content(db.getStats().dump(), "application/json");
    });

    // ── Get Code ─────────────────────────────────────────────────────────────

    svr.Get("/api/code", [&](const httplib::Request&, httplib::Response& res) {
        setCors(res);
        std::vector<std::string> files = {
            "include/Models.hpp",
            "include/DataManager.hpp",
            "src/DataManager.cpp",
            "src/main.cpp"
        };
        std::string code;
        for (const auto& f : files) {
            auto content = readFileToString(f);
            if (content.empty()) {
                content = readFileToString("../" + f);
            }
            if (!content.empty()) {
                code += "// ========== " + f + " ==========\n\n";
                code += content;
                code += "\n\n";
            }
        }
        res.set_content(json{{"code", code}, {"files", files}}.dump(), "application/json");
    });

    // ── CRUD Airlines ────────────────────────────────────────────────────────

    svr.Post("/api/airline", [&](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        try {
            auto data = json::parse(req.body);
            auto result = db.insertAirline(data);
            res.status = result.contains("error") ? 409 : 201;
            res.set_content(result.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        }
    });

    svr.Put(R"(/api/airline/([A-Za-z0-9]+))", [&](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        try {
            auto data = json::parse(req.body);
            auto result = db.modifyAirline(req.matches[1], data);
            res.status = result.contains("error") ? 404 : 200;
            res.set_content(result.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        }
    });

    svr.Delete(R"(/api/airline/([A-Za-z0-9]+))", [&](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        auto result = db.removeAirline(req.matches[1]);
        res.status = result.contains("error") ? 404 : 200;
        res.set_content(result.dump(), "application/json");
    });

    // ── CRUD Airports ────────────────────────────────────────────────────────

    svr.Post("/api/airport", [&](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        try {
            auto data = json::parse(req.body);
            auto result = db.insertAirport(data);
            res.status = result.contains("error") ? 409 : 201;
            res.set_content(result.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        }
    });

    svr.Put(R"(/api/airport/([A-Za-z0-9]+))", [&](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        try {
            auto data = json::parse(req.body);
            auto result = db.modifyAirport(req.matches[1], data);
            res.status = result.contains("error") ? 404 : 200;
            res.set_content(result.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        }
    });

    svr.Delete(R"(/api/airport/([A-Za-z0-9]+))", [&](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        auto result = db.removeAirport(req.matches[1]);
        res.status = result.contains("error") ? 404 : 200;
        res.set_content(result.dump(), "application/json");
    });

    // ── CRUD Routes ──────────────────────────────────────────────────────────

    svr.Post("/api/route", [&](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        try {
            auto data = json::parse(req.body);
            auto result = db.insertRoute(data);
            res.status = result.contains("error") ? 400 : 201;
            res.set_content(result.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        }
    });

    svr.Put("/api/route", [&](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        try {
            auto data = json::parse(req.body);
            int srcId = data.value("sourceAirportId", -1);
            int dstId = data.value("destAirportId", -1);
            int airlineId = data.value("airlineId", -1);
            auto updates = data.value("updates", json::object());
            auto result = db.modifyRoute(srcId, dstId, airlineId, updates);
            res.status = result.contains("error") ? 404 : 200;
            res.set_content(result.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        }
    });

    svr.Delete("/api/route", [&](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        int srcId = std::stoi(req.get_param_value("srcId"));
        int dstId = std::stoi(req.get_param_value("dstId"));
        int airlineId = std::stoi(req.get_param_value("airlineId"));
        auto result = db.removeRoute(srcId, dstId, airlineId);
        res.status = result.contains("error") ? 404 : 200;
        res.set_content(result.dump(), "application/json");
    });

    // ── One-Hop Report ───────────────────────────────────────────────────────

    svr.Get("/api/onehop", [&](const httplib::Request& req, httplib::Response& res) {
        setCors(res);
        auto src = req.get_param_value("src");
        auto dst = req.get_param_value("dst");
        if (src.empty() || dst.empty()) {
            res.status = 400;
            res.set_content(json{{"error", "src and dst parameters required"}}.dump(), "application/json");
            return;
        }
        auto result = db.oneHopReport(src, dst);
        res.status = result.contains("error") ? 404 : 200;
        res.set_content(result.dump(), "application/json");
    });

    // ── Start Server ─────────────────────────────────────────────────────────

    int port = 8080;
    const char* portEnv = std::getenv("PORT");
    if (portEnv) port = std::stoi(portEnv);

    std::cout << "\n====================================\n";
    std::cout << "  Air Travel DB - OpenFlights Explorer\n";
    std::cout << "  http://localhost:" << port << "\n";
    std::cout << "====================================\n\n";

    if (!svr.listen("0.0.0.0", port)) {
        std::cerr << "Failed to start server on port " << port << "\n";
        return 1;
    }

    return 0;
}
