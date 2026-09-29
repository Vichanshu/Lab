#include <bits/stdc++.h>
using namespace std;

struct Location {
    string name;
    double x, y;
};


double distance(Location a, Location b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;

    return sqrt(dx * dx + dy * dy);
}


double routeCost(vector<int>& route, vector<Location>& locations) {

    double total = 0.0;


    total += distance(locations[0], locations[route[0]]);


    for (int i = 0; i < route.size() - 1; i++) {
        total += distance(
            locations[route[i]],
            locations[route[i + 1]]
        );
    }


    total += distance(
        locations[route.back()],
        locations[0]
    );

    return total;
}


void printRoute(vector<int>& route, vector<Location>& locations) {

    cout << "W -> ";

    for (int i : route) {
        cout << locations[i].name << " -> ";
    }

    cout << "W";
}

int main() {


    vector<Location> locations = {
        {"W", 0, 0},
        {"A", 2, 6},
        {"B", 5, 2},
        {"C", 6, 7},
        {"D", 8, 3},
        {"E", 1, 4},
        {"F", 7, 6},
        {"G", 3, 1}
    };



    vector<int> currentRoute = {
        1, 2, 3, 4, 5, 6, 7
    };


    random_device rd;
    mt19937 gen(rd());

    shuffle(currentRoute.begin(), currentRoute.end(), gen);

    double currentCost =
        routeCost(currentRoute, locations);

    cout << fixed << setprecision(2);

    cout << "Initial Route: ";
    printRoute(currentRoute, locations);

    cout << "\nInitial Cost: "
         << currentCost << "\n\n";




    int iteration = 0;

    while (true) {

        iteration++;

        vector<int> bestRoute = currentRoute;
        double bestCost = currentCost;


        for (int i = 0; i < currentRoute.size(); i++) {

            for (int j = i + 1;
                 j < currentRoute.size();
                 j++) {


                vector<int> neighbour = currentRoute;


                swap(neighbour[i], neighbour[j]);


                double cost =
                    routeCost(neighbour, locations);


                if (cost < bestCost) {
                    bestCost = cost;
                    bestRoute = neighbour;
                }
            }
        }



        if (bestCost >= currentCost) {
            iteration--;
            break;
        }


        currentRoute = bestRoute;
        currentCost = bestCost;

        cout << "Iteration " << iteration << ":\n";

        cout << "Route: ";
        printRoute(currentRoute, locations);

        cout << "\nCost: "
             << currentCost << "\n\n";
    }





    cout << "Final Route: ";
    printRoute(currentRoute, locations);

    cout << "\nFinal Cost: "
         << currentCost << "\n";

    cout << "Iterations: "
         << iteration << "\n";



    return 0;
}