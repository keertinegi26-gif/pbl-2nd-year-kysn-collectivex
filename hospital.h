#ifndef HOSPITAL_H
#define HOSPITAL_H

#include <string>
using namespace std;

class Hospital {
private:
    int id;
    string name;
    int locationId;

public:
    Hospital();
    Hospital(int id, string name, int locationId);

    int getId() const;
    string getName() const;
    int getLocationId() const;

    bool isValid() const;
    void display() const;
};

#endif
