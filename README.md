# ResQroute – Ambulance Allocation & Route Optimization (C++ / DSA)

Dispatches the nearest free ambulance to the most urgent patient, finds the
shortest road route to the patient, then to a hospital, and frees the
ambulance after drop-off.

## DSA used
| Structure / algorithm | Where |
|---|---|
| Graph (adjacency list) | `graph.cpp` – road network |
| Dijkstra's shortest path | `graph.cpp` – distances + parent array |
| Priority queue (max-heap) | `allocation.cpp` – Critical > Serious > Normal, FIFO inside a level |
| Path reconstruction from parent array | `route.cpp` |

## How a dispatch works (menu option 8)
1. Take the most urgent request from the priority queue.
2. Run Dijkstra once from the patient (roads are two-way).
3. Choose the available ambulance with the shortest distance.
4. Critical patients always go to the nearest hospital; Serious/Normal go to the chosen hospital (or nearest if 0).
5. Print both legs (ambulance -> patient, patient -> hospital), total km and ETA.
6. Option 9 completes the trip: the ambulance becomes available at the hospital's location.

If no ambulance is free, the request stays in the queue.

## Build & run
    make
    ./resqroute        (Windows: resqroute.exe)
Use menu option 11 to load demo data.

Or without make:
    g++ -std=c++11 -o resqroute main.cpp graph.cpp patient.cpp ambulance.cpp hospital.cpp route.cpp allocation.cpp input_output.cpp

## Web front end
    make
    python server.py          then open http://localhost:8000

Architecture: `frontend/index.html` (browser)  <-HTTP/JSON->  `server.py` (bridge)  <-stdin/stdout->  `resqroute --json` (C++).
The bridge starts the C++ program once, so the queue and ambulance state stay in memory.

API (all JSON): `GET /api/state`, `POST /api/patient | ambulance | hospital | road | dispatch | complete | demo | reset`.
Any other front end (React, Flutter, mobile) can call the same endpoints.
