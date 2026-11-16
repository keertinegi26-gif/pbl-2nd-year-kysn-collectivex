
#include <iostream>
#include <climits>
#include <algorithm>
#include "graph.h"

using namespace std;

Graph::Graph() {                // Default constructor: initializes the graph with zero locations.
    totalLocations = 0;
}

Graph::Graph(int totalLocations) {                
    this->totalLocations = totalLocations;
    adjacencyList.resize(totalLocations);
}

bool Graph::isValidLocation(int location) const {
    return location >= 0 && location < totalLocations;
}

int Graph::getTotalLocations() const {
    return totalLocations;
}

void Graph::addRoad(int from, int to, int distance) {
    if (!isValidLocation(from) || !isValidLocation(to)) {
        cout << "Error: Invalid location for road.\n";
        return;
    }

    if (distance <= 0) {
        cout << "Error: Road distance must be positive.\n";
        return;
    }

    adjacencyList[from].push_back({to, distance});
    adjacencyList[to].push_back({from, distance});
}

vector<int> Graph::dijkstra(int source, vector<int>& parent) {
    vector<int> distance(totalLocations, INT_MAX);
    vector<bool> visited(totalLocations, false);

    parent.assign(totalLocations, -1);

    if (!isValidLocation(source)) {
        return distance;
    }

    distance[source] = 0;

    for (int count = 0; count < totalLocations - 1; count++) {
        int nearest = -1;
        int minimumDistance = INT_MAX;

        for (int i = 0; i < totalLocations; i++) {
            if (!visited[i] && distance[i] < minimumDistance) {
                minimumDistance = distance[i];
                nearest = i;
            }
        }

        if (nearest == -1) {
            break;
        }

        visited[nearest] = true;

        for (auto road : adjacencyList[nearest]) {
            int destination = road.first;
            int roadDistance = road.second;

            if (!visited[destination] &&
                distance[nearest] != INT_MAX &&
                distance[nearest] <= INT_MAX - roadDistance &&
                distance[nearest] + roadDistance < distance[destination]) {

                distance[destination] =
                    distance[nearest] + roadDistance;

                parent[destination] = nearest;
            }
        }
    }

    return distance;
}

void Graph::displayGraph() const {
    cout << "\n--- ResQroute Road Network ---\n";

    for (int i = 0; i < totalLocations; i++) {
        cout << "Location " << i << " -> ";

        if (adjacencyList[i].empty()) {
            cout << "No direct roads";
        } else {
            for (auto road : adjacencyList[i]) {
                cout << "(" << road.first
                     << ", " << road.second << ") ";
            }
        }

        cout << "\n";
    }
}


vector<vector<int>> Graph::getRoadList() const { 
    vector<vector<int>> roads;

    for (int i = 0; i < totalLocations; i++) {
        for (auto road : adjacencyList[i]) {
            if (i <= road.first) {
                roads.push_back({i, road.first, road.second});
            }
        }
    }

    return roads;
} 
