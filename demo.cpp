#include "demo.h"

void loadDemoData(Graph& graph, Allocation& allocation) {
    int roads[][3] = {
        {0, 1, 4}, {0, 2, 3}, {1, 2, 2}, {1, 3, 5}, {2, 4, 6},
        {3, 4, 2}, {3, 5, 4}, {4, 6, 3}, {5, 6, 1}, {5, 7, 6},
        {6, 8, 5}, {7, 8, 2}, {7, 9, 3}, {8, 9, 4}
    };

    for (auto& r : roads) {
        graph.addRoad(r[0], r[1], r[2]);
    }

    allocation.addAmbulance(Ambulance(1, "Ravi", 0, true));
    allocation.addAmbulance(Ambulance(2, "Aman", 4, true));
    allocation.addAmbulance(Ambulance(3, "Sita", 8, true));

    allocation.addHospital(Hospital(1, "City General", 5));
    allocation.addHospital(Hospital(2, "Metro Care", 9));
}
