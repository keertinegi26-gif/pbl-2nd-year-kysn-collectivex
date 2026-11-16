#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "api.h"
#include "graph.h"
#include "allocation.h"
#include "patient.h"
#include "route.h"
#include "demo.h"

using namespace std;

static const int TOTAL_LOCATIONS = 10;

static string esc(const string& s) {
    string out;
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if ((unsigned char)c < 0x20) out += ' ';
        else out += c;
    }
    return out;
}

static string cleanText(string s) {
    for (char& c : s) {
        if (c == '\n' || c == '\r') c = ' ';
    }
    size_t start = s.find_first_not_of(' ');
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(' ');
    return s.substr(start, end - start + 1);
}

static string reply(bool ok, const string& message, const string& extra = "") {
    string out = string("{\"ok\":") + (ok ? "true" : "false") +
                 ",\"message\":\"" + esc(message) + "\"";
    if (!extra.empty()) out += "," + extra;
    return out + "}";
}

static string pathJson(const vector<int>& path) {
    string out = "[";
    for (size_t i = 0; i < path.size(); i++) {
        if (i > 0) out += ",";
        out += to_string(path[i]);
    }
    return out + "]";
}

static string planJson(const DispatchPlan& p) {
    ostringstream o;
    o << "\"plan\":{"
      << "\"requestId\":" << p.request.requestId
      << ",\"patient\":\"" << esc(p.request.patientName) << "\""
      << ",\"severity\":\"" << p.request.severityLabel() << "\""
      << ",\"priority\":" << p.request.priority
      << ",\"ambulanceId\":" << p.ambulanceId
      << ",\"ambulanceLocation\":" << p.ambulanceLocation
      << ",\"hospitalId\":" << p.hospitalId
      << ",\"hospitalName\":\"" << esc(p.hospitalName) << "\""
      << ",\"pickupPath\":" << pathJson(p.pickupPath)
      << ",\"dropPath\":" << pathJson(p.dropPath)
      << ",\"pickupDistance\":" << p.pickupDistance
      << ",\"dropDistance\":" << p.dropDistance
      << ",\"totalDistance\":" << p.totalDistance()
      << ",\"etaMinutes\":" << p.etaMinutes()
      << ",\"hospitalOverridden\":" << (p.hospitalOverridden ? "true" : "false")
      << "}";
    return o.str();
}

static string stateJson(const Graph& graph, const Allocation& allocation) {
    ostringstream o;
    o << "{\"ok\":true,\"nodes\":" << graph.getTotalLocations() << ",\"roads\":[";

    auto roads = graph.getRoadList();
    for (size_t i = 0; i < roads.size(); i++) {
        if (i > 0) o << ",";
        o << "[" << roads[i][0] << "," << roads[i][1] << "," << roads[i][2] << "]";
    }

    o << "],\"ambulances\":[";
    const auto& ambulances = allocation.getAmbulances();
    for (size_t i = 0; i < ambulances.size(); i++) {
        const Ambulance& a = ambulances[i];
        if (i > 0) o << ",";
        o << "{\"id\":" << a.getId()
          << ",\"driver\":\"" << esc(a.getDriverName()) << "\""
          << ",\"location\":" << a.getCurrentLocation()
          << ",\"available\":" << (a.isAvailable() ? "true" : "false")
          << ",\"dropLocation\":" << a.getDropLocation() << "}";
    }

    o << "],\"hospitals\":[";
    const auto& hospitals = allocation.getHospitals();
    for (size_t i = 0; i < hospitals.size(); i++) {
        const Hospital& h = hospitals[i];
        if (i > 0) o << ",";
        o << "{\"id\":" << h.getId()
          << ",\"name\":\"" << esc(h.getName()) << "\""
          << ",\"location\":" << h.getLocationId() << "}";
    }

    o << "],\"pending\":[";
    auto pending = allocation.getPendingSorted();
    for (size_t i = 0; i < pending.size(); i++) {
        const EmergencyRequest& r = pending[i];
        if (i > 0) o << ",";
        o << "{\"id\":" << r.requestId
          << ",\"name\":\"" << esc(r.patientName) << "\""
          << ",\"location\":" << r.patientLocation
          << ",\"hospitalId\":" << r.hospitalId
          << ",\"priority\":" << r.priority
          << ",\"severity\":\"" << r.severityLabel() << "\"}";
    }

    o << "]}";
    return o.str();
}

static string handle(const string& line, Graph& graph, Allocation& allocation,
                     ostringstream& captured) {
    istringstream in(line);
    string cmd;
    in >> cmd;

    if (cmd == "STATE") {
        return stateJson(graph, allocation);
    }

    if (cmd == "DEMO") {
        if (!allocation.getAmbulances().empty() || !graph.getRoadList().empty()) {
            return reply(false, "Demo can only be loaded on an empty system. Reset first.");
        }
        loadDemoData(graph, allocation);
        return reply(true, "Demo data loaded.");
    }

    if (cmd == "RESET") {
        graph = Graph(TOTAL_LOCATIONS);
        allocation = Allocation();
        return reply(true, "System reset.");
    }

    if (cmd == "ROAD") {
        int from, to, distance;
        if (!(in >> from >> to >> distance)) return reply(false, "Bad road details.");
        if (!graph.isValidLocation(from) || !graph.isValidLocation(to) || distance <= 0) {
            return reply(false, "Invalid road details.");
        }
        graph.addRoad(from, to, distance);
        return reply(true, "Road added.");
    }

    if (cmd == "AMBULANCE") {
        int id, location, available;
        if (!(in >> id >> location >> available)) return reply(false, "Bad ambulance details.");
        string driver;
        getline(in >> ws, driver);
        driver = cleanText(driver);

        if (id <= 0 || !graph.isValidLocation(location) ||
            (available != 0 && available != 1)) {
            return reply(false, "Invalid ambulance details.");
        }

        bool ok = allocation.addAmbulance(Ambulance(id, driver, location, available == 1));
        return reply(ok, ok ? "Ambulance added." : cleanText(captured.str()));
    }

    if (cmd == "HOSPITAL") {
        int id, location;
        if (!(in >> id >> location)) return reply(false, "Bad hospital details.");
        string name;
        getline(in >> ws, name);
        name = cleanText(name);

        if (!graph.isValidLocation(location)) return reply(false, "Invalid location.");

        bool ok = allocation.addHospital(Hospital(id, name, location));
        return reply(ok, ok ? "Hospital added." : cleanText(captured.str()));
    }

    if (cmd == "PATIENT") {
        int id, location, priority, hospitalId;
        if (!(in >> id >> location >> priority >> hospitalId)) {
            return reply(false, "Bad patient details.");
        }
        string name;
        getline(in >> ws, name);
        name = cleanText(name);

        Patient patient(id, name, location, priority);
        if (!patient.isValid() || !graph.isValidLocation(location)) {
            return reply(false, "Invalid patient details.");
        }

        EmergencyRequest request(id, name, location, hospitalId, priority);
        bool ok = allocation.addEmergencyRequest(request);
        return reply(ok, ok ? "Emergency request added." : cleanText(captured.str()));
    }

    if (cmd == "DISPATCH") {
        DispatchPlan plan = allocation.dispatchNext(graph);
        if (!plan.success) return reply(false, plan.message);
        return reply(true, plan.message, planJson(plan));
    }

    if (cmd == "COMPLETE") {
        int id;
        if (!(in >> id)) return reply(false, "Bad ambulance ID.");
        bool ok = allocation.completeTrip(id);
        return reply(ok, ok ? "Patient dropped at hospital. Ambulance is free."
                            : cleanText(captured.str()));
    }

    return reply(false, "Unknown command.");
}

void runJsonApi() {
    Graph graph(TOTAL_LOCATIONS);
    Allocation allocation;
    string line;

    cout << reply(true, "ResQroute backend ready.") << endl;

    while (getline(cin, line)) {
        if (cleanText(line).empty()) continue;

        // existing classes print errors with cout; capture them so stdout stays pure JSON
        ostringstream captured;
        streambuf* original = cout.rdbuf(captured.rdbuf());
        string out = handle(line, graph, allocation, captured);
        cout.rdbuf(original);

        cout << out << endl;
    }
}
