#include <iostream>
#include <iomanip>
#include <vector>
#include "graph.h"
#include "main.h"
#include "input_output.h"
#include "allocation.h"
#include "route.h"
#include "demo.h"
#include "api.h"

using namespace std;

static const int TOTAL_LOCATIONS = 10;

static void printDispatchPlan(const DispatchPlan& plan) {
    cout << "\n=========== DISPATCH ===========\n";
    cout << "Patient   : " << plan.request.patientName
         << " (" << plan.request.severityLabel() << ")\n";
    cout << "Ambulance : " << plan.ambulanceId
         << " (starts at node " << plan.ambulanceLocation << ")\n";
    cout << "Hospital  : " << plan.hospitalName
         << " (ID " << plan.hospitalId << ")";

    if (plan.request.priority == 1) {
        cout << "  [nearest hospital - critical case]";
    }
    if (plan.hospitalOverridden) {
        cout << "  [requested hospital unreachable, nearest used]";
    }
    cout << "\n\n";

    cout << "Leg 1 (to patient)  : " << Route::pathToString(plan.pickupPath)
         << "  | " << plan.pickupDistance << " km\n";
    cout << "Leg 2 (to hospital) : " << Route::pathToString(plan.dropPath)
         << "  | " << plan.dropDistance << " km\n";
    cout << "Total distance      : " << plan.totalDistance() << " km\n";
    cout << fixed << setprecision(1)
         << "Estimated trip time : " << plan.etaMinutes() << " min\n";
    cout << "================================\n";
}

void MainApp::runConsoleMode() {
    Graph graph(TOTAL_LOCATIONS);
    Allocation allocation;
    bool demoLoaded = false;
    int choice;

    do {
        InputOutput::printMenu();
        choice = InputOutput::readInteger("");

        if (choice == 0) {
            InputOutput::printMessage("Exiting ResQroute.");
            break;
        }

        switch (choice) {
            case 1: {
                Patient patient = InputOutput::readPatient();

                if (!patient.isValid() ||
                    !graph.isValidLocation(patient.getLocationId())) {
                    InputOutput::printMessage("Invalid patient details.");
                    break;
                }

                int hospitalId = InputOutput::readInteger(
                    "Enter Hospital ID (0 = nearest): ");

                EmergencyRequest request(
                    patient.getId(),
                    patient.getName(),
                    patient.getLocationId(),
                    hospitalId,
                    patient.getPriority()
                );

                if (allocation.addEmergencyRequest(request)) {
                    InputOutput::printMessage("Emergency request added.");
                }
                break;
            }

            case 2: {
                int id = InputOutput::readInteger("Enter Ambulance ID: ");

                if (id <= 0) {
                    InputOutput::printMessage("Invalid Ambulance ID.");
                    break;
                }

                string driver = InputOutput::readText("Enter Driver Name: ");

                int location = InputOutput::readInteger(
                    "Enter Location (0-9): ");

                if (!graph.isValidLocation(location)) {
                    InputOutput::printMessage("Invalid location.");
                    break;
                }

                int available = InputOutput::readInteger(
                    "Available? (1/0): ");

                if (available != 0 && available != 1) {
                    InputOutput::printMessage(
                        "Enter 1 for available or 0 for unavailable.");
                    break;
                }

                if (allocation.addAmbulance(
                        Ambulance(id, driver, location, available == 1))) {
                    InputOutput::printMessage("Ambulance added.");
                }
                break;
            }

            case 3: {
                int id = InputOutput::readInteger("Enter Hospital ID: ");
                string name = InputOutput::readText("Enter Hospital Name: ");
                int location = InputOutput::readInteger(
                    "Enter Location (0-9): ");

                if (!graph.isValidLocation(location)) {
                    InputOutput::printMessage("Invalid location.");
                    break;
                }

                if (allocation.addHospital(Hospital(id, name, location))) {
                    InputOutput::printMessage("Hospital added.");
                }
                break;
            }

            case 4: {
                int from = InputOutput::readInteger("From Location (0-9): ");
                int to = InputOutput::readInteger("To Location (0-9): ");
                int distance = InputOutput::readInteger("Distance (km): ");

                if (!graph.isValidLocation(from) ||
                    !graph.isValidLocation(to) ||
                    distance <= 0) {
                    InputOutput::printMessage("Invalid road details.");
                    break;
                }

                graph.addRoad(from, to, distance);
                InputOutput::printMessage("Road added.");
                break;
            }

            case 5:
                graph.displayGraph();
                break;

            case 6:
                allocation.displayAmbulances();
                break;

            case 7:
                allocation.displayPendingRequests();
                break;

            case 8: {
                DispatchPlan plan = allocation.dispatchNext(graph);

                if (!plan.success) {
                    InputOutput::printMessage(plan.message);
                    break;
                }

                printDispatchPlan(plan);
                break;
            }

            case 9: {
                int id = InputOutput::readInteger("Enter Ambulance ID: ");

                if (allocation.completeTrip(id)) {
                    InputOutput::printMessage(
                        "Patient dropped at hospital. Ambulance is available again.");
                }
                break;
            }

            case 10:
                allocation.displayHospitals();
                break;

            case 11: {
                if (demoLoaded) {
                    InputOutput::printMessage("Demo data already loaded.");
                    break;
                }

                loadDemoData(graph, allocation);
                demoLoaded = true;
                InputOutput::printMessage(
                    "Demo loaded: 10 nodes, 14 roads, 3 ambulances, 2 hospitals.");
                break;
            }

            default:
                InputOutput::printMessage("Invalid choice.");
                break;
        }

    } while (true);
}

void MainApp::runJsonMode() {
    runJsonApi();
}

void MainApp::start() {
    MainApp app;
    app.runConsoleMode();
}

int main(int argc, char* argv[]) {
    if (argc > 1 && string(argv[1]) == "--json") {
        MainApp::runJsonMode();   // used by the web front end
    } else {
        MainApp::start();         // normal console menu
    }
    return 0;
}
