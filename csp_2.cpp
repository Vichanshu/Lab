#include <bits/stdc++.h>
using namespace std;

vector<string> courses = {"AI", "DBMS", "OS", "CN"};

// Current domain of every course
map<string, vector<int>> domain;

// Assignment
map<string, int> assignment;


// Check whether assigning this slot is allowed
bool isSafe(string course, int slot) {

    if (course == "AI") {

        if (assignment.count("DBMS") &&
            assignment["DBMS"] == slot)
            return false;

        if (assignment.count("OS") &&
            assignment["OS"] == slot)
            return false;
    }

    if (course == "DBMS") {

        if (assignment.count("AI") &&
            assignment["AI"] == slot)
            return false;

        if (assignment.count("CN") &&
            assignment["CN"] == slot)
            return false;
    }

    if (course == "OS") {

        if (assignment.count("AI") &&
            assignment["AI"] == slot)
            return false;

        if (assignment.count("CN") &&
            assignment["CN"] == slot)
            return false;
    }

    if (course == "CN") {

        if (assignment.count("DBMS") &&
            assignment["DBMS"] == slot)
            return false;

        if (assignment.count("OS") &&
            assignment["OS"] == slot)
            return false;
    }

    return true;
}


// ==========================================
// MRV
// Choose the course with the smallest
// remaining domain.
// ==========================================
string selectMRV() {

    string selected = "";
    int minimum = INT_MAX;

    for (string course : courses) {

        if (assignment.count(course))
            continue;

        int count = 0;

        for (int slot : domain[course]) {

            if (isSafe(course, slot))
                count++;
        }

        if (count < minimum) {

            minimum = count;
            selected = course;
        }
    }

    return selected;
}


// ==========================================
// Forward Checking
//
// After assigning a course, remove the
// selected slot from courses that conflict
// with it.
// ==========================================
bool forwardCheck(string course, int slot) {

    if (course == "AI") {

        // AI and DBMS cannot be same
        vector<int> temp;

        for (int x : domain["DBMS"]) {
            if (x != slot)
                temp.push_back(x);
        }

        domain["DBMS"] = temp;


        // AI and OS cannot be same
        temp.clear();

        for (int x : domain["OS"]) {
            if (x != slot)
                temp.push_back(x);
        }

        domain["OS"] = temp;
    }


    if (course == "DBMS") {

        // DBMS and AI
        vector<int> temp;

        for (int x : domain["AI"]) {
            if (x != slot)
                temp.push_back(x);
        }

        domain["AI"] = temp;


        // DBMS and CN
        temp.clear();

        for (int x : domain["CN"]) {
            if (x != slot)
                temp.push_back(x);
        }

        domain["CN"] = temp;
    }


    if (course == "OS") {

        // OS and AI
        vector<int> temp;

        for (int x : domain["AI"]) {
            if (x != slot)
                temp.push_back(x);
        }

        domain["AI"] = temp;


        // OS and CN
        temp.clear();

        for (int x : domain["CN"]) {
            if (x != slot)
                temp.push_back(x);
        }

        domain["CN"] = temp;
    }


    if (course == "CN") {

        // CN and DBMS
        vector<int> temp;

        for (int x : domain["DBMS"]) {
            if (x != slot)
                temp.push_back(x);
        }

        domain["DBMS"] = temp;


        // CN and OS
        temp.clear();

        for (int x : domain["OS"]) {
            if (x != slot)
                temp.push_back(x);
        }

        domain["OS"] = temp;
    }


    // If any unassigned course has no
    // possible value, we must backtrack.
    for (string c : courses) {

        if (!assignment.count(c) &&
            domain[c].empty()) {

            return false;
        }
    }

    return true;
}


// Print current domains
void printDomains() {

    for (string course : courses) {

        if (assignment.count(course))
            continue;

        cout << course << " = { ";

        for (int x : domain[course])
            cout << x << " ";

        cout << "}\n";
    }
}


// ==========================================
// Backtracking + MRV + Forward Checking
// ==========================================
bool solve() {

    // All courses assigned
    if (assignment.size() == courses.size())
        return true;


    // -------------------------------
    // MRV
    // -------------------------------
    string course = selectMRV();

    cout << "\nSelected using MRV: "
         << course << endl;


    // Try every value in its domain
    vector<int> originalDomain = domain[course];

    for (int slot : originalDomain) {

        if (!isSafe(course, slot))
            continue;

        cout << "Trying "
             << course << " = "
             << slot << endl;


        // Save current domains so that
        // we can restore them if we backtrack.
        map<string, vector<int>> oldDomain = domain;

        // Make assignment
        assignment[course] = slot;


        // -------------------------------
        // Forward Checking
        // -------------------------------
        if (forwardCheck(course, slot)) {

            cout << "Domains after Forward Checking:\n";
            printDomains();

            if (solve())
                return true;
        }


        // -------------------------------
        // Backtracking
        // -------------------------------
        cout << "Backtracking from "
             << course << " = "
             << slot << endl;

        assignment.erase(course);

        // Restore old domains
        domain = oldDomain;
    }

    return false;
}


int main() {

    // Initial domains
    domain["AI"] = {1, 2, 3};
    domain["DBMS"] = {1, 2, 4};
    domain["OS"] = {2, 3, 4};
    domain["CN"] = {1, 3, 4};

    cout << "Course Scheduling using "
         << "MRV + Forward Checking\n";

    cout << "\nInitial Domains:\n";
    printDomains();


    if (solve()) {

        cout << "\n============================\n";
        cout << "Final Timetable\n";
        cout << "============================\n";

        for (string course : courses) {

            cout << course << " -> "
                 << assignment[course] << endl;
        }
    }

    return 0;
}