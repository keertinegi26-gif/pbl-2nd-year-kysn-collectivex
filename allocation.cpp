#include <iostream>
#include <climits>
#include <algorithm>
#include "allocation.h"
#include "route.h"
using namespace std;

// ---------------- EmergencyRequest ----------------

EmergencyRequest::EmergencyRequest() {
    requestId = 0;
    patientName = "";
    patientLocation = -1;
    hospitalId = 0;
    priority = 3;
    sequence = 0;
}

EmergencyRequest::EmergencyRequest(int requestId, string patientName,
                                   int patientLocation, int hospitalId,
                                   int priority) {
    this->requestId = requestId;
    this->patientName = patientName;
    this->patientLocation = patientLocation;
    this->hospitalId = hospitalId;
    this->priority = priority;
    this->sequence = 0;
}

string EmergencyRequest::severityLabel() const {
    if (priority == 1) return "Critical";
    if (priority == 2) return "Serious";
    return "Normal";
}

void EmergencyRequest::display() const {
    cout << "Request ID: " << requestId
         << " | Patient: " << patientName
         << " | Location: " << patientLocation
         << " | Hospital: ";

    if (hospitalId == 0) cout << "nearest";
    else cout << hospitalId;

    cout << " | Severity: " << severityLabel() << "\n";
}

// ---------------- DispatchPlan ----------------

DispatchPlan::DispatchPlan() {
    success = false;
    message = "";
    ambulanceId = -1;
    ambulanceLocation = -1;
    hospitalId = -1;
    hospitalName = "";
    pickupDistance = 0;
    dropDistance = 0;
    hospitalOverridden = false;
}

int DispatchPlan::totalDistance() const {
    return pickupDistance + dropDistance;
}

// Assumed average speeds (road distance unit = km).
// Critical cases are driven with siren at higher average speed.
double DispatchPlan::etaMinutes() const {
    double speed = 40.0;
    if (request.priority == 1) speed = 60.0;
    else if (request.priority == 2) speed = 50.0;

    return totalDistance() / speed * 60.0;
}

// ---------------- RequestOrder ----------------

bool RequestOrder::operator()(const EmergencyRequest& a,
                              const EmergencyRequest& b) const {
    if (a.priority != b.priority) {
        return a.priority > b.priority;   // smaller number = more urgent
    }
    return a.sequence > b.sequence;       // earlier arrival first
}

// ---------------- Allocation ----------------

Allocation::Allocation() {
    nextSequence = 1;
}

bool Allocation::addAmbulance(const Ambulance& ambulance) {
    for (const auto& a : ambulances) {
        if (a.getId() == ambulance.getId()) {
            cout << "Error: Ambulance ID already exists.\n";
            return false;
        }
    }

    ambulances.push_back(ambulance);
    return true;
}

bool Allocation::addHospital(const Hospital& hospital) {
    if (!hospital.isValid()) {
        cout << "Error: Invalid hospital details.\n";
        return false;
    }

    if (hasHospital(hospital.getId())) {
        cout << "Error: Hospital ID already exists.\n";
        return false;
    }

    hospitals.push_back(hospital);
    return true;
}

bool Allocation::hasHospital(int hospitalId) const {
    for (const auto& h : hospitals) {
        if (h.getId() == hospitalId) return true;
    }
    return false;
}

bool Allocation::addEmergencyRequest(EmergencyRequest request) {
    if (request.patientLocation < 0 || request.hospitalId < 0 ||
        request.priority < 1 || request.priority > 3) {
        cout << "Error: Invalid emergency request.\n";
        return false;
    }

    if (request.hospitalId != 0 && !hasHospital(request.hospitalId)) {
        cout << "Error: Hospital " << request.hospitalId << " does not exist.\n";
        return false;
    }

    request.sequence = nextSequence++;
    emergencyQueue.push(request);
    return true;
}

bool Allocation::hasPendingEmergency() const {
    return !emergencyQueue.empty();
}

/*
 * Dispatch logic
 * 1. Take the most urgent request (Critical > Serious > Normal, FIFO inside a level).
 * 2. Run Dijkstra ONCE from the patient. The roads are two-way, so this single
 *    run gives the distance from the patient to every ambulance and every hospital.
 * 3. Pick the available ambulance with the smallest distance.
 * 4. Hospital: Critical patients (or "nearest") go to the closest hospital;
 *    Serious/Normal patients go to the hospital they asked for.
 * 5. Pickup path = reverse of patient->ambulance path, drop path = patient->hospital.
 */
DispatchPlan Allocation::dispatchNext(Graph& graph) {
    DispatchPlan plan;

    if (emergencyQueue.empty()) {
        plan.message = "No pending emergencies.";
        return plan;
    }

    if (hospitals.empty()) {
        plan.message = "No hospitals added yet.";
        return plan;
    }

    EmergencyRequest request = emergencyQueue.top();

    vector<int> parent;
    vector<int> distance = graph.dijkstra(request.patientLocation, parent);

    // nearest available ambulance
    Ambulance* bestAmbulance = nullptr;
    int bestAmbulanceDist = INT_MAX;
    bool anyAvailable = false;

    for (auto& ambulance : ambulances) {
        if (!ambulance.isAvailable()) continue;
        anyAvailable = true;

        int loc = ambulance.getCurrentLocation();
        if (!graph.isValidLocation(loc) || distance[loc] == INT_MAX) continue;

        if (distance[loc] < bestAmbulanceDist) {
            bestAmbulanceDist = distance[loc];
            bestAmbulance = &ambulance;
        }
    }

    if (bestAmbulance == nullptr) {
        plan.request = request;
        plan.message = anyAvailable
            ? "No available ambulance can reach the patient. Request kept in queue."
            : "No available ambulance. Request kept in queue.";
        return plan;
    }

    // hospital choice
    const Hospital* chosen = nullptr;
    bool wantsNearest = (request.priority == 1 || request.hospitalId == 0);

    if (!wantsNearest) {
        for (const auto& h : hospitals) {
            if (h.getId() == request.hospitalId &&
                distance[h.getLocationId()] != INT_MAX) {
                chosen = &h;
            }
        }
        if (chosen == nullptr) {
            plan.hospitalOverridden = true;   // asked hospital unreachable
        }
    }

    if (chosen == nullptr) {
        int bestDist = INT_MAX;
        for (const auto& h : hospitals) {
            int d = distance[h.getLocationId()];
            if (d < bestDist) {
                bestDist = d;
                chosen = &h;
            }
        }
    }

    if (chosen == nullptr || distance[chosen->getLocationId()] == INT_MAX) {
        plan.request = request;
        plan.message = "No hospital is reachable from the patient. Request kept in queue.";
        return plan;
    }

    // build both legs
    vector<int> toAmbulance = Route::buildPath(
        request.patientLocation, bestAmbulance->getCurrentLocation(), parent);
    reverse(toAmbulance.begin(), toAmbulance.end());   // ambulance -> patient

    plan.pickupPath = toAmbulance;
    plan.dropPath = Route::buildPath(
        request.patientLocation, chosen->getLocationId(), parent);

    plan.pickupDistance = bestAmbulanceDist;
    plan.dropDistance = distance[chosen->getLocationId()];

    plan.request = request;
    plan.ambulanceId = bestAmbulance->getId();
    plan.ambulanceLocation = bestAmbulance->getCurrentLocation();
    plan.hospitalId = chosen->getId();
    plan.hospitalName = chosen->getName();

    // commit
    emergencyQueue.pop();
    bestAmbulance->assign(chosen->getLocationId());

    plan.success = true;
    plan.message = "Ambulance dispatched.";
    return plan;
}

bool Allocation::completeTrip(int ambulanceId) {
    for (auto& ambulance : ambulances) {
        if (ambulance.getId() == ambulanceId) {
            if (ambulance.isAvailable()) {
                cout << "That ambulance is not on a trip.\n";
                return false;
            }
            ambulance.completeTrip();
            return true;
        }
    }

    cout << "Ambulance not found.\n";
    return false;
}

void Allocation::displayAmbulances() const {
    cout << "\n--- Ambulances ---\n";

    if (ambulances.empty()) {
        cout << "No ambulances added.\n";
        return;
    }

    for (const auto& ambulance : ambulances) {
        ambulance.display();
    }
}

void Allocation::displayHospitals() const {
    cout << "\n--- Hospitals ---\n";

    if (hospitals.empty()) {
        cout << "No hospitals added.\n";
        return;
    }

    for (const auto& hospital : hospitals) {
        hospital.display();
    }
}

void Allocation::displayPendingRequests() const {
    cout << "\n--- Pending Emergency Requests (most urgent first) ---\n";

    if (emergencyQueue.empty()) {
        cout << "No pending requests.\n";
        return;
    }

    auto temp = emergencyQueue;   // copy so the real queue is untouched

    while (!temp.empty()) {
        temp.top().display();
        temp.pop();
    }
}

const vector<Ambulance>& Allocation::getAmbulances() const {
    return ambulances;
}

const vector<Hospital>& Allocation::getHospitals() const {
    return hospitals;
}

vector<EmergencyRequest> Allocation::getPendingSorted() const {
    vector<EmergencyRequest> result;
    auto temp = emergencyQueue;

    while (!temp.empty()) {
        result.push_back(temp.top());
        temp.pop();
    }

    return result;
}
//end
