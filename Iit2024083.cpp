#include <bits/stdc++.h>
using namespace std;

int treasures_found = 0;
int total_treasures = 5;
int steps = 0;
int steps_to_first = -1;
int steps_to_all = -1;
int revisited_count = 0;
vector<string> percept_sequence;

void dls(pair<int,int> curr, int depth, int limit, vector<vector<char>> &arr, vector<vector<int>> &visited) {
    if (depth > limit || treasures_found == total_treasures) {
        return;
    }

    visited[curr.first][curr.second] = 1;

    if (arr[curr.first][curr.second] == 'T') {
        percept_sequence.push_back("grab the item");
        treasures_found++;
        arr[curr.first][curr.second] = '.'; 
        
        if (treasures_found == 1 && steps_to_first == -1) {
            steps_to_first = steps;
        }
        if (treasures_found == total_treasures && steps_to_all == -1) {
            steps_to_all = steps;
            return;
        }
    }

    int dr[] = {-1, 0, 1, 0};
    int dc[] = {0, 1, 0, -1};

    for (int i = 0; i < 4; i++) {
        if (treasures_found == total_treasures) break;

        int nr = curr.first + dr[i];
        int nc = curr.second + dc[i];

        if (nr < 0 || nr >= 15 || nc < 0 || nc >= 15 || arr[nr][nc] == 'X') {
            percept_sequence.push_back("change the path");
        } 
        else if (!visited[nr][nc]) {
            percept_sequence.push_back("move ahead");
            steps++;
            
            dls({nr, nc}, depth + 1, limit, arr, visited);
            
            if (treasures_found < total_treasures) {
                percept_sequence.push_back("move ahead (backtrack)");
                steps++;
                revisited_count++;
            }
        }
    }
}

int main() {
    int n = 15, m = 15;
    vector<vector<char>> arr(n, vector<char>(m, '.'));
    
    vector<pair<int,int>> obstacles = {
        {0,0}, {0,1}, {0,12}, {1,2}, {1,11}, {2,4}, {2,5}, {2,10}, 
        {3,6}, {3,10}, {4,10}, {9,3}, {9,4}, {10,7}, {10,8}, {10,9}
    };
    
    vector<pair<int,int>> treasures = {
        {0,2}, {1,11}, {7,0}, {13,1}, {13,10} 
    };

    for (auto it : obstacles) {
        arr[it.first][it.second] = 'X';
    }
    for (auto it : treasures) {
        arr[it.first][it.second] = 'T';
    }

    pair<int,int> start = {7, 7};
    vector<vector<int>> visited(n, vector<int>(m, 0));
    
    int depth_limit = 225;
    
    dls(start, 0, depth_limit, arr, visited);

    cout << "1. Percept Sequence (Showing first 25 and last 5 percepts to save space):\n";
    for (int i = 0; i < min(25, (int)percept_sequence.size()); i++) {
        cout << percept_sequence[i] << " -> ";
    }
    cout << "...\n...\n";
    for (int i = max(0, (int)percept_sequence.size() - 5); i < percept_sequence.size(); i++) {
        cout << percept_sequence[i] << (i == percept_sequence.size() - 1 ? "" : " -> ");
    }
    cout << "\n\n";

    cout << "2. Number of steps to find the 1st treasure: " << steps_to_first << "\n";
    cout << "3. Number of steps to find all treasures: " << steps_to_all << "\n";
    cout << "4. Number of cells repeated/revisited: " << revisited_count << "\n";

    return 0;
}
