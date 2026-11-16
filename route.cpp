
#include <algorithm>
#include <string>
#include <vector>
#include "route.h"
using namespace std;

vector<int> Route::buildPath(int source, int destination, const vector<int>& parent) {
    vector<int> path;

    if (source < 0 || destination < 0 ||
        source >= static_cast<int>(parent.size()) ||
        destination >= static_cast<int>(parent.size())) {
        return path;
    }

    int current = destination;

    while (current != -1) {
        path.push_back(current);

        if (current == source) {
            break;
        }

        current = parent[current];
    }

    if (path.back() != source) {
        return {};
    }

    reverse(path.begin(), path.end());
    return path;
}

string Route::pathToString(const vector<int>& path) {
    if (path.empty()) {
        return "No route available";
    }

    string result;

    for (size_t i = 0; i < path.size(); i++) {
        if (i > 0) {
            result += " -> ";
        }

        result += to_string(path[i]);
    }

    return result;
}
