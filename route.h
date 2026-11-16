#ifndef ROUTE_H
#define ROUTE_H

#include <string>
#include <vector>
#include <map>
#include <utility>

using namespace std;

class Route {
public:
    static vector<int> buildPath(
        int source,
        int destination,
        const vector<int>& parent
    );

    static string pathToString(const vector<int>& path);
};

#endif