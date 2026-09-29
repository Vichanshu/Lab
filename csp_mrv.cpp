#include <bits/stdc++.h>
using namespace std;

vector<char> colors = {'R', 'G', 'B'};

vector<string> variables = {"A", "B", "C", "D", "E"};

map<string, vector<string>> neighbours = {
    {"A", {"B", "C"}},
    {"B", {"A", "C", "D", "E"}},
    {"C", {"A", "B", "E"}},
    {"D", {"B", "E"}},
    {"E", {"B", "C", "D"}}
};

map<string, char> assignment;

bool isSafe(string variable, char color) {

    for (string neighbour : neighbours[variable]) {

        if (assignment.count(neighbour) &&
            assignment[neighbour] == color) {
            return false;
        }
    }

    return true;
}

// Count available colors for a variable
int remainingColors(string variable) {

    int count = 0;

    for (char color : colors) {

        if (isSafe(variable, color))
            count++;
    }

    return count;
}


// ===============================
// MRV IS USED HERE
// ===============================
string selectMRVVariable() {

    string selected = "";
    int minimum = INT_MAX;

    for (string variable : variables) {

        if (assignment.count(variable))
            continue;

        int available = remainingColors(variable);

        if (available < minimum) {

            minimum = available;
            selected = variable;
        }
    }

    return selected;
}


bool backtrack() {

    // All variables assigned
    if (assignment.size() == variables.size())
        return true;

    // ===============================
    // MRV IS USED HERE
    // Choose variable with the
    // minimum remaining values.
    // ===============================
    string variable = selectMRVVariable();

    cout << "\nSelected using MRV: "
         << variable << endl;

    for (char color : colors) {

        cout << "Trying "
             << variable << " = "
             << color << endl;

        if (isSafe(variable, color)) {

            assignment[variable] = color;

            if (backtrack())
                return true;

            cout << "Backtracking from "
                 << variable << " = "
                 << color << endl;

            assignment.erase(variable);
        }
    }

    return false;
}


int main() {

    cout << "Backtracking with MRV\n";

    if (backtrack()) {

        cout << "\nFinal Assignment:\n";

        for (string variable : variables) {

            cout << variable << " = "
                 << assignment[variable] << endl;
        }
    }

    return 0;
}