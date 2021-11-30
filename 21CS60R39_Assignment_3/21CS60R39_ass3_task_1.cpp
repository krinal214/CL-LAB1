#include <iostream>
#include "togasat.hpp"
using namespace std;

int code[4];  // stores a hidden code
int colormap[9] = {0};
int breakcode[4]; // used by codebreaker to store it's guesses.
int B = 0;
int W = 0;
int totalvar = 32;
bool stop = false;
int permutation[5][7] = {{0, 0, 0, 0, 0, 0, 0}, {4, 1, 2, 3, 4, 0, 0}, {6, 12, 13, 14, 23, 24, 34}, {4, 123, 124, 134, 234, 0, 0}, {0, 0, 0, 0, 0, 0, 0}};
int inverse[5][7] = {{0, 0, 0, 0, 0, 0, 0}, {4, 234, 134, 124, 123, 0, 0}, {6, 34, 24, 23, 14, 13, 12}, {4, 4, 3, 2, 1, 0, 0}, {0, 0, 0, 0, 0, 0, 0}};
char ch[] = {'R', 'B', 'G', 'Y', 'O', 'P', 'W', 'K'}; // binding 1-8 color to characters
vector<vector<int>> clauses; // storing the clauses incrementely

//for generating random numbers
void randoms(int a[], int lower, int upper, int count)
{
    srand(time(0));
    for (int i = 0; i < count; i++)
    {
        int num = (rand() % (upper - lower + 1)) + lower;
        while (true)
        {
            bool unique = true;
            for (int j = 0; j < i; j++)
            {
                if (a[j] == num)
                {
                    num = (rand() % (upper - lower + 1)) + lower;
                    unique = false;
                }
            }
            if (unique)
                break;
        }
        a[i] = num;
    }
}

//for invoking a SAT solver
void solve(vector<vector<int>> &v, togasat::Solver &solver)
{
    for (int i = 0; i < v.size(); i++)
    {
        if (v[i].size() == 0)
            continue;
        solver.addClause(v[i]);
    }

    bool status = solver.solve();
}

//codemaker will generate a hidden code
void codemaker()
{

    randoms(code, 1, 8, 4);
    for (int i = 0; i < 4; i++)
    {
        colormap[code[i]] = 1;
        cout << ch[code[i] - 1] << " ";
    }
    cout << endl;
}

//compares array of colors provided by codebreaker with hidden code
void codecheker(int a[])
{
    B = 0;
    W = 0;
    int tempmap[9] = {0};
    for (int i = 0; i < 4; i++)
    {
        cout << ch[a[i] - 1] << " ";
        if (code[i] == a[i])
            B++;
        tempmap[a[i]] = 1;
    }

    for (int i = 0; i <= 8; i++)
    {
        W += min(colormap[i], tempmap[i]);
    }

    W -= B;

    cout << " W : " << W << " B: " << B << endl;
}


//Initialize the game: setting up the below rules
//from variable 1-32(combination of colors and position), exactly 4 unique colors on diffent position should be true,rest of the variables must be false.
void initialize()
{
    togasat::Solver solver;
    vector<int> clause;
    int k = 1;
    for (int i = 1; i <= 4; i++)
    {
        for (int j = k; j <= k + 7; j++)
        {
            clause.push_back(j);
        }
        clauses.push_back(clause);
        clause.clear();
        for (int j = k; j <= k + 7; j++)
        {

            for (int t = k; t <= k + 7; t++)
            {
                if (t == j)
                    continue;
                else
                {
                    clause.push_back(-j);
                    clause.push_back(-t);
                    clauses.push_back(clause);
                    clause.clear();
                }
            }
            int offset = j - k + 1;
            for (int t = offset; t <= 32; t = t + 8)
            {
                if (t == j)
                    continue;
                clause.push_back(-j);
                clause.push_back(-t);
                clauses.push_back(clause);
                clause.clear();
            }
        }
        k = k + 8;
    }
    solve(clauses, solver);
}

/*It will generate a color array
Based on feedback received,it will add new clauses.
Thus, function will be called repeateadly untill it cracks the hidden code.
*/
void codebreaker()
{
    vector<int> clause;
    togasat::Solver solver;

    int j = 0;

    solve(clauses, solver);

    for (int i = 0; i < 32; i++)
    {
        if (solver.assigns[i] == 0)
        {

            breakcode[j] = (i + 1) % 8;
            if (breakcode[j] == 0)
                breakcode[j] = 8;

            j++;
        }
    }
    codecheker(breakcode);

    for (int i = 0; i < 4; i++)
    {
        breakcode[i] = (i * 8) + breakcode[i];
    }
    if (B == 4)
    {
        stop = true;
        return;
    }
    int variable = totalvar;
    
    //for blank(color must be changed)
    for (int i = 0; i < 4; i++)
    {
        variable++;
        clause.push_back(variable);
        int p = breakcode[i] % 8;
        if (p == 0)
            p = 8;
        for (int j = p; j <= 32; j = j + 8)
        {
            clause.push_back(j);
        }
        clauses.push_back(clause);
        clause.clear();

        for (int j = p; j <= 32; j = j + 8)
        {
            clause.push_back(-variable);
            clause.push_back(-j);
            clauses.push_back(clause);
            clause.clear();
        }
    }
    //for white(color is present,though it is not in correct place)
    for (int i = 0; i < 4; i++)
    {

        variable++;
        clause.push_back(-variable);
        int p = breakcode[i] % 8;
        if (p == 0)
            p = 8;

        for (int j = p; j <= 32; j = j + 8)
        {
            if (j == breakcode[i])
                continue;

            clause.push_back(j);
        }
        clauses.push_back(clause);
        clause.clear();
        for (int j = p; j <= 32; j = j + 8)
        {

            if (j == breakcode[i])
                continue;
            clause.push_back(variable);
            clause.push_back(-j);
            clauses.push_back(clause);
            clause.clear();
        }
    }

    //black(color is at it's correct position)
    for (int i = 0; i < 4; i++)
    {
        variable++;
        clause.push_back(-variable);
        clause.push_back(breakcode[i]);
        clauses.push_back(clause);
        clause.clear();

        clause.push_back(variable);
        clause.push_back(-breakcode[i]);
        clauses.push_back(clause);
        clause.clear();
    }
    int blank = 4 - (B + W);

    if (blank == 4)
    {
        int start = totalvar + 1;
        for (int i = start; i < start + 4; i++)
        {
            clause.push_back(i);
            clauses.push_back(clause);
            clause.clear();
        }
    }
    else if (W == 4)
    {
        int start = totalvar + 5;
        for (int i = start; i < start + 4; i++)
        {
            clause.push_back(i);
            clauses.push_back(clause);
            clause.clear();
        }
    }
    else
    {
        vector<int> clause1;
        vector<int> clause2;
        int array[] = {blank, W, B};
        for (int k = 0; k < 3; k++)
        {

            int size = permutation[array[k]][0];
            int disp = totalvar + (4 * k);
            for (int i = 1; i <= size; i++)
            {
                variable++;
                clause1.push_back(variable);

                int x = permutation[array[k]][i];
                while (x > 0)
                {
                    int m = x % 10;
                    x = x / 10;
                    clause.push_back(-variable);
                    clause.push_back(disp + m);
                    clauses.push_back(clause);
                    clause.clear();
                    clause1.push_back(-(disp + m));
                }
                x = inverse[array[k]][i];
                while (x > 0)
                {
                    int m = x % 10;
                    x = x / 10;
                    clause.push_back(-variable);
                    clause.push_back(-(disp + m));
                    clauses.push_back(clause);
                    clause.clear();
                    clause1.push_back(disp + m);
                }
                clauses.push_back(clause1);

                clause1.clear();
                clause2.push_back(variable);
            }
            if (clause2.size() != 0)
            {
                clauses.push_back(clause2);
                clause2.clear();
            }
        }
    }

    totalvar = variable;
}
int main()
{
    initialize();
    codemaker();
    while (true)
    {
        codebreaker();
        if (stop)
            break;
    }
   
     
    return 0;
}