#include <iostream>
#include "ambulance.h"
using namespace std;

Ambulance::Ambulance() {
    id = 0;
    driverName = "";
    currentLocation = -1;
    available = true;
    dropLocation = -1;
}

Ambulance::Ambulance(int id, string driverName, int currentLocation, bool available) {
    this->id = id;
    this->driverName = driverName;
    this->currentLocation = currentLocation;
    this->available = available;
    this->dropLocation = -1;
}

int Ambulance::getId() const {
    return id;
}

string Ambulance::getDriverName() const {
    return driverName;
}

int Ambulance::getCurrentLocation() const {
    return currentLocation;
}

bool Ambulance::isAvailable() const {
    return available;
}

int Ambulance::getDropLocation() const {
    return dropLocation;
}

void Ambulance::assign(int hospitalLocation) {
    available = false;
    dropLocation = hospitalLocation;
}

void Ambulance::completeTrip() {
    if (dropLocation >= 0) {
        currentLocation = dropLocation;
    }
    dropLocation = -1;
    available = true;
}

void Ambulance::release(int newLocation) {
    available = true;
    currentLocation = newLocation;
    dropLocation = -1;
}

void Ambulance::display() const {
    cout << "Ambulance ID: " << id
         << " | Driver: " << driverName
         << " | Location: " << currentLocation
         << " | Status: " << (available ? "Available" : "Busy");

    if (!available && dropLocation >= 0) {
        cout << " (heading to hospital at node " << dropLocation << ")";
    }

    cout << "\n";
}
