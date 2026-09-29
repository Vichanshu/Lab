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

    // Warehouse -> first delivery location
    total += distance(locations[0], locations[route[0]]);

    // Between delivery locations
    for (int i = 0; i < route.size() - 1; i++) {
        total += distance(locations[route[i]], locations[route[i + 1]]);
    }

    // Last delivery location -> Warehouse
    total += distance(locations[route.back()], locations[0]);

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

    // Locations
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

    // Random number generator
    mt19937 gen(42);



    vector<int> currentRoute = {1, 2, 3, 4, 5, 6, 7};

    shuffle(currentRoute.begin(), currentRoute.end(), gen);

    double currentCost = routeCost(currentRoute, locations);

    // Maintain best route found
    vector<int> bestRoute = currentRoute;
    double bestCost = currentCost;



    double temperature = 100.0;
    double alpha = 0.95;
    double minimumTemperature = 0.1;

    uniform_real_distribution<double> randomProbability(0.0, 1.0);

    int iteration = 0;

    cout << fixed << setprecision(2);

    // Initial route
    cout << "Initial Route: ";
    printRoute(currentRoute, locations);

    cout << "\nInitial Cost: " << currentCost << "\n\n";




    while (temperature >= minimumTemperature) {

        iteration++;



        vector<int> neighbour = currentRoute;

        int i = gen() % neighbour.size();
        int j = gen() % neighbour.size();

        while (i == j) {
            j = gen() % neighbour.size();
        }

        swap(neighbour[i], neighbour[j]);



        double newCost = routeCost(neighbour, locations);

        double deltaE = newCost - currentCost;

        bool accepted = false;



        if (deltaE < 0) {

            currentRoute = neighbour;
            currentCost = newCost;

            accepted = true;

            cout << "Better route -> Accepted";

        }



        else {

            double probability = exp(-deltaE / temperature);

            double r = randomProbability(gen);

            if (r < probability) {

                currentRoute = neighbour;
                currentCost = newCost;

                accepted = true;

                cout << "Worse route -> Accepted";

            }
            else {

                cout << "Worse route -> Rejected";
            }

            cout << " | P = " << probability;
            cout << " | r = " << r;
        }



        if (currentCost < bestCost) {

            bestCost = currentCost;
            bestRoute = currentRoute;
        }




        cout << "\nIteration: " << iteration;
        cout << "\nTemperature: " << temperature;

        cout << "\nCurrent Route: ";
        printRoute(currentRoute, locations);

        cout << "\nCurrent Cost: " << currentCost;

        cout << "\nDelta E: " << deltaE;

        cout << "\n-----------------------------\n";




        temperature = alpha * temperature;
    }




    cout << "\n====================================\n";

    cout << "Best Route: ";
    printRoute(bestRoute, locations);

    cout << "\nBest Cost: " << bestCost;

    cout << "\nNumber of Iterations: " << iteration;

    cout << "\n====================================\n";

    return 0;
}