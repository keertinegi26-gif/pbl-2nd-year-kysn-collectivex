#include <iostream>
#include <sstream>
#include <string>
#include "input_output.h"

using namespace std;

int InputOutput::readInteger(const string& message) {
    string line;
    int value;
    char extra;

    while (true) {
        cout << message;

        if (!getline(cin, line)) {
            return 0;
        }

        stringstream ss(line);

        if (ss >> value && !(ss >> extra)) {
            return value;
        }

        cout << "Invalid input. Please enter a number.\n";
    }
}

string InputOutput::readText(const string& message) {
    string value;

    cout << message;
    getline(cin >> ws, value);

    return value;
}

Patient InputOutput::readPatient() {
    int id = readInteger("Enter Patient ID: ");
    string name = readText("Enter Patient Name: ");
    int location = readInteger("Enter Patient Location: ");
    int priority = readInteger(
        "Enter Priority (1-Critical, 2-Serious, 3-Normal): "
    );

    return Patient(id, name, location, priority);
}

void InputOutput::printMenu() {
    cout << "\n========== ResQroute ==========\n";
    cout << "1.  Add Patient / Emergency Request\n";
    cout << "2.  Add Ambulance\n";
    cout << "3.  Add Hospital\n";
    cout << "4.  Add Road\n";
    cout << "5.  Display Road Network\n";
    cout << "6.  Display Ambulances\n";
    cout << "7.  Display Pending Requests\n";
    cout << "8.  Dispatch Next Emergency\n";
    cout << "9.  Complete Trip (patient dropped at hospital)\n";
    cout << "10. Display Hospitals\n";
    cout << "11. Load Demo Data\n";
    cout << "0.  Exit\n";
    cout << "Enter choice: ";
}

void InputOutput::printMessage(const string& message) {
    cout << message << "\n";
}
//end
