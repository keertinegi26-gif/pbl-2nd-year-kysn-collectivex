#ifndef PATIENT_H
#define PATIENT_H

#include <string>
using namespace std;

class Patient {
private:
    int id;
    string name;
    int locationId;
    int priority;

public:
    Patient();
    Patient(int id, string name, int locationId, int priority);

    int getId() const;
    string getName() const;
    int getLocationId() const;
    int getPriority() const;

    void setId(int id);
    void setName(string name);
    void setLocationId(int locationId);
    void setPriority(int priority);

    bool isValid() const;
    void display() const;
};

#endif