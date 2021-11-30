//Course allocation need not be unique.

#include <iostream>
#include <glpk.h>
using namespace std;

int x[1000001];
int y[1000001];
double ar[1000001];

int main()
{
    glp_prob *lp1;
    lp1 = glp_create_prob();
    glp_set_prob_name(lp1, "task1");
    glp_set_obj_dir(lp1, GLP_MAX);
    int n, m;
    scanf("%d,%d", &n, &m);


    int a[n + 1][m + 1];  // preference Array

    for (int i = 0; i <= n; i++)
    {
        for (int j = 0; j <= m; j++)
        {
            a[i][j] = 0;
        }
    }

    int N;
    cin >> N;
    int b[m + 1];
    for (int i = 1; i <= m; i++)
    {
        if (i < m)
            scanf("%d,", &b[i]);
        else
            scanf("%d", &b[i]);
    }
    int p;
    cin >> p;
    for (int k = 0; k < p; k++)
    {
        int s, c;
        scanf("%d,%d", &s, &c);

        a[s][c] = 1;
    }


    int rows = n + m + n;
    glp_add_rows(lp1, rows);
    
    //adding row constraints
    for (int i = 1; i <= n; i++)
    {
        glp_set_row_bnds(lp1, i, GLP_UP, 0.0, N);
    }
    for (int i = n + 1; i <= n + m; i++)
    {
        glp_set_row_bnds(lp1, i, GLP_UP, 0.0, b[i - n]);
    }
    for (int i = n + m + 1; i <= n + m + n; i++)
    {
        glp_set_row_bnds(lp1, i, GLP_FX, 0, 0);
    }

    int cols = n * m;
    glp_add_cols(lp1, cols);

    //adding column constraints
    for (int i = 1; i <= n * m; i++)
    {

        glp_set_col_bnds(lp1, i, GLP_DB, 0.0, 1.0);
        glp_set_col_kind(lp1, i, GLP_BV);

        glp_set_obj_coef(lp1, i, 1.0);
    }
    int count = 0;
    int tt = 1;
    int rn = 1;

    //A student can take atmost N students constraints
    for (int i = 1; i <= n; i++)
    {
        for (int j = 1; j <= m; j++)
        {
            if (a[i][j] == 1)
            {
                int val = (i - 1) * m + j;
                x[tt] = rn;
                y[tt] = val;
                ar[tt] = 1.0;
                tt++;
                count++;
            }
        }
        rn++;
    }

    //Maximumn b[j] students can take course j 
    for (int j = 1; j <= m; j++)
    {
        for (int i = 1; i <= n; i++)
        {
            if (a[i][j] == 1)
            {
                int val = (i - 1) * m + j;
                x[tt] = rn;
                y[tt] = val;
                ar[tt] = 1.0;
                tt++;
                count++;
            }
        }
        rn++;
    }

    //Students will never take courses other than their preferences
    for (int i = 1; i <= n; i++)
    {
        for (int j = 1; j <= m; j++)
        {
            if (a[i][j] != 1)
            {
                int val = (i - 1) * m + j;
                x[tt] = rn;
                y[tt] = val;
                ar[tt] = 1.0;
                tt++;

                count++;
            }
        }
        rn++;
    }

    glp_load_matrix(lp1, count, x, y, ar);
    glp_simplex(lp1, NULL);

    glp_iocp parm;
    parm.presolve = GLP_ON;
    glp_init_iocp(&parm);
    glp_intopt(lp1, &parm);

    int max_course = glp_mip_obj_val(lp1);
    cout << "Maximum Course Allocation: " << max_course << endl;
    int xsc[n + 1][m + 1];

    /*output format
    Student_id  --> [list of courses]
    */
   
    for (int i = 1; i <= n; i++)
    {
        cout << i << "--> ";
        bool flag = true;
        for (int j = 1; j <= m; j++)
        {
            int tt = (i - 1) * m + j;
            xsc[i][j] = glp_get_col_prim(lp1, tt);

            if (xsc[i][j] == 1)
            {
                if (!flag)
                {
                    cout << ",";
                }
                flag = false;
                cout << j;
            }
        }
        cout << endl;
    }

    glp_delete_prob(lp1);
    glp_free_env();
    return 0;
}