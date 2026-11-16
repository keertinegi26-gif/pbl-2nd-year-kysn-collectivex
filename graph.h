#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <utility>
using namespace std;

class Graph {
private:
    int totalLocations;
    vector<vector<pair<int, int>>> adjacencyList;

public:
    Graph();
    Graph(int totalLocations);

    void addRoad(int from, int to, int distance);
    bool isValidLocation(int location) const;
    int getTotalLocations() const;

    vector<int> dijkstra(int source, vector<int>& parent);
    void displayGraph() const;
    vector<vector<int>> getRoadList() const;   // each road once: [from, to, distance]
};

#endif 
