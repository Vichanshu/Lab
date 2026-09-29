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

bool backtrack(int index) {

    if (index == variables.size())
        return true;

    string variable = variables[index];

    for (char color : colors) {

        cout << "Trying " << variable
             << " = " << color << endl;

        if (isSafe(variable, color)) {

            assignment[variable] = color;

            if (backtrack(index + 1))
                return true;

            // Backtracking
            cout << "Backtracking from "
                 << variable << " = "
                 << color << endl;

            assignment.erase(variable);
        }
    }

    return false;
}

int main() {

    cout << "Ordinary Backtracking\n\n";

    if (backtrack(0)) {

        cout << "\nFinal Assignment:\n";

        for (string variable : variables) {
            cout << variable << " = "
                 << assignment[variable] << endl;
        }
    }

    return 0;
}