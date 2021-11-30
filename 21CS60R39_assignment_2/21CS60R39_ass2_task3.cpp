#include <iostream>
using namespace std;
int INF = 1e8 + 7;
int main()
{
    int n, m;
    scanf("%d,%d", &n, &m);
    int warshal[n + 1][n + 1]; // it stores the initial graph as n*n matrix. and then same array is used for finding all pair shortest path

    for (int i = 0; i <= n; i++)
    {
        for (int j = 0; j <= n; j++)
        {
            if (i != j)
            {
                warshal[i][j] = INF;
            }
            else
            {
                warshal[i][j] = 0;
            }
        }
    }
    for (int i = 0; i < m; i++)
    {
        int start, end, weight;
        scanf("%d,%d,%d", &start, &end, &weight);
        warshal[start][end] = weight;
        warshal[end][start] = weight;
    }
    int infected[n + 1];// already infected nodes
    bool possible[n + 1]; // possible infected nodes
    for (int i = 0; i <= n; i++)
    {
        infected[i] = -1;
        possible[i] = false;
    }
    int inflist;
    cin >> inflist;
    for (int i = 0; i < inflist; i++)
    {
        int idx, time;
        scanf("%d,%d", &idx, &time);
        infected[idx] = time;
    }

    for (int k = 1; k <= n; k++)
    {
        for (int i = 1; i <= n; i++)
        {
            for (int j = 1; j <= n; j++)
            {
                if (warshal[i][k] == INF)
                    continue;
                if (warshal[k][j] == INF)
                    continue;
                if (warshal[i][j] > warshal[i][k] + warshal[k][j])
                {
                    warshal[i][j] = warshal[i][k] + warshal[k][j];
                }
            }
        }
    }

    for (int i = 1; i <= n; i++)
    {
        for (int j = 1; j <= n; j++)
        {
            if (infected[i] == -1 || infected[j] == -1 || infected[i] >= infected[j])
                continue;
            for (int k = 1; k <= n; k++)
            {
                if (infected[k] == -1 && warshal[i][k] + warshal[k][j] == warshal[i][j])
                {
                    possible[k] = true;
                }
            }
        }
    }
    for (int i = 1; i <= n; i++)
    {
        if (possible[i])
            cout << i << endl;
    }

    return 0;
}