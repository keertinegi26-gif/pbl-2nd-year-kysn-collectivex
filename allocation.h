#ifndef ALLOCATION_H
#define ALLOCATION_H

#include <queue>
#include <vector>
#include <string>
#include "ambulance.h"
#include "hospital.h"
#include "graph.h"

using namespace std;

struct EmergencyRequest {
    int requestId;
    string patientName;
    int patientLocation;
    int hospitalId;   // 0 = choose the nearest hospital automatically
    int priority;     // 1 = Critical, 2 = Serious, 3 = Normal
    int sequence;     // arrival order, breaks ties inside the same priority

    EmergencyRequest();
    EmergencyRequest(int requestId, string patientName, int patientLocation,
                     int hospitalId, int priority);

    string severityLabel() const;
    void display() const;
};

// Result of dispatching one emergency
struct DispatchPlan {
    bool success;
    string message;

    EmergencyRequest request;
    int ambulanceId;
    int ambulanceLocation;
    int hospitalId;
    string hospitalName;

    vector<int> pickupPath;   // ambulance -> patient
    vector<int> dropPath;     // patient   -> hospital
    int pickupDistance;
    int dropDistance;
    bool hospitalOverridden;  // true if the asked hospital was unreachable

    DispatchPlan();
    int totalDistance() const;
    double etaMinutes() const;
};

// Orders the heap: Critical first, then earliest arrival
struct RequestOrder {
    bool operator()(const EmergencyRequest& a, const EmergencyRequest& b) const;
};

class Allocation {
private:
    priority_queue<EmergencyRequest, vector<EmergencyRequest>, RequestOrder> emergencyQueue;
    vector<Ambulance> ambulances;
    vector<Hospital> hospitals;
    int nextSequence;

public:
    Allocation();

    bool addAmbulance(const Ambulance& ambulance);
    bool addHospital(const Hospital& hospital);
    bool addEmergencyRequest(EmergencyRequest request);

    bool hasPendingEmergency() const;
    bool hasHospital(int hospitalId) const;

    DispatchPlan dispatchNext(Graph& graph);
    bool completeTrip(int ambulanceId);

    const vector<Ambulance>& getAmbulances() const;
    const vector<Hospital>& getHospitals() const;
    vector<EmergencyRequest> getPendingSorted() const;

    void displayAmbulances() const;
    void displayHospitals() const;
    void displayPendingRequests() const;
};

#endif
