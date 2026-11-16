#include <iostream>
#include "patient.h"
using namespace std;

Patient::Patient() {
    id = 0;
    name = "";
    locationId = -1;
    priority = 3;
}

Patient::Patient(int id, string name, int locationId, int priority) {
    this->id = id;
    this->name = name;
    this->locationId = locationId;
    this->priority = priority;
}

int Patient::getId() const {
    return id;
}

string Patient::getName() const {
    return name;
}

int Patient::getLocationId() const {
    return locationId;
}

int Patient::getPriority() const {
    return priority;
}

void Patient::setId(int id) {
    this->id = id;
}

void Patient::setName(string name) {
    this->name = name;
}

void Patient::setLocationId(int locationId) {
    this->locationId = locationId;
}

void Patient::setPriority(int priority) {
    this->priority = priority;
}

bool Patient::isValid() const {
    return id > 0 && !name.empty() && locationId >= 0 &&
           priority >= 1 && priority <= 3;
}

void Patient::display() const {
    cout << "Patient ID: " << id
         << " | Name: " << name
         << " | Location: " << locationId
         << " | Priority: " << priority << "\n";

}
