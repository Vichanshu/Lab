#include <bits/stdc++.h>
using namespace std;

#define int long long


using Node = tuple<int, int, int>;

int heuristic(int r, int c, int tr, int tc) {

    return abs(r - tr) + abs(c - tc);
}

vector<pair<int, int>> AStar(
    vector<vector<int>>& grid,
    pair<int, int> source,
    pair<int, int> target
) {
    int n = grid.size();
    int m = grid[0].size();


    vector<vector<int>> g(
        n, vector<int>(m, INT_MAX)
    );


    vector<vector<pair<int, int>>> parent(
        n, vector<pair<int, int>>(m, {-1, -1})
    );

    priority_queue<
        Node,
        vector<Node>,
        greater<Node>
    > pq;

    int sr = source.first;
    int sc = source.second;

    int tr = target.first;
    int tc = target.second;


    g[sr][sc] = 0;


    int h = heuristic(sr, sc, tr, tc);

    pq.push({h, sr, sc});


    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    while (!pq.empty()) {

        auto [f, r, c] = pq.top();
        pq.pop();


        if (r == tr && c == tc)
            break;

        for (int i = 0; i < 4; i++) {

            int nr = r + dr[i];
            int nc = c + dc[i];


            if (nr < 0 || nr >= n ||
                nc < 0 || nc >= m)
                continue;


            if (grid[nr][nc] == 1)
                continue;


            int newG = g[r][c] + 1;

            if (newG < g[nr][nc]) {

                g[nr][nc] = newG;


                parent[nr][nc] = {r, c};


                int h = heuristic(nr, nc, tr, tc);


                int newF = newG + h;

                pq.push({newF, nr, nc});
            }
        }
    }


    if (g[tr][tc] == INT_MAX) {
        cout << "No path exists\n";
        return {};
    }



    vector<pair<int, int>> path;

    int r = tr;
    int c = tc;

    while (r != -1 && c != -1) {

        path.push_back({r, c});

        auto [pr, pc] = parent[r][c];

        r = pr;
        c = pc;
    }


    reverse(path.begin(), path.end());

    cout << "Shortest distance = "
         << g[tr][tc] << endl;

    cout << "Path:\n";

    for (auto [r, c] : path) {
        cout << "(" << r << ", " << c << ") ";


        if (!(r == tr && c == tc))
            cout << "-> ";
    }

    cout << endl;

    return path;
}

signed main() {

    vector<vector<int>> grid = {
        {0, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0}
    };

    pair<int, int> source = {0, 0};
    pair<int, int> target = {4, 4};

    AStar(grid, source, target);
}