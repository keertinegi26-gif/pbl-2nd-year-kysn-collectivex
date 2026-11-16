#ifndef AMBULANCE_H
#define AMBULANCE_H

#include <string>
using namespace std;

class Ambulance {
private:
    int id;
    string driverName;
    int currentLocation;
    bool available;
    int dropLocation;   // hospital node of the trip in progress (-1 if none)

public:
    Ambulance();
    Ambulance(int id, string driverName, int currentLocation, bool available);

    int getId() const;
    string getDriverName() const;
    int getCurrentLocation() const;
    bool isAvailable() const;
    int getDropLocation() const;

    void assign(int hospitalLocation);   // start a trip, becomes busy
    void completeTrip();                 // patient dropped, free at hospital
    void release(int newLocation);
    void display() const;
};

#endif
