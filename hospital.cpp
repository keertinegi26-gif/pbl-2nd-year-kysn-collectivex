#include <iostream>
#include "hospital.h"
using namespace std;

Hospital::Hospital() {
    id = 0;
    name = "";
    locationId = -1;
}

Hospital::Hospital(int id, string name, int locationId) {
    this->id = id;
    this->name = name;
    this->locationId = locationId;
}

int Hospital::getId() const {
    return id;
}

string Hospital::getName() const {
    return name;
}

int Hospital::getLocationId() const {
    return locationId;
}

bool Hospital::isValid() const {
    return id > 0 && !name.empty() && locationId >= 0;
}

void Hospital::display() const {
    cout << "Hospital ID: " << id
         << " | Name: " << name
         << " | Location: " << locationId << "\n";
}
