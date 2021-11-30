#include <iostream>
#include <glpk.h>
using namespace std;

int n, m, o;
int N;
int x[5000001];
int y[5000001];
double ar[5000001];

//LP1 Procedure
int lp1(double **xsc, double **ysc)
{
    glp_prob *lp1;
    lp1 = glp_create_prob();
    glp_set_prob_name(lp1, "lp1");
    glp_set_obj_dir(lp1, GLP_MIN);

    int rows = 1 * m + (m * (m - 1) * o) / 2;
    int cols = o * m + (m * (m - 1) * o / 2);

    //adding row constraints
    glp_add_rows(lp1, rows);
    int total = 0;
    for (int i = 1; i <= m; i++)
    {
        glp_set_row_bnds(lp1, i, GLP_FX, 1, 1);
    }

    total = m;

    for (int i = total + 1; i <= total + (m * (m - 1) * o) / 2; i++)
    {
        glp_set_row_bnds(lp1, i, GLP_UP, 0, 1);
    }

    //column constraints
    glp_add_cols(lp1, cols);
    for (int i = 1; i <= cols; i++)

    {
        if (i <= m * o)
        {

            glp_set_col_bnds(lp1, i, GLP_DB, 0, 1);
            glp_set_col_kind(lp1, i, GLP_BV);
        }
        else
        {
            glp_set_col_bnds(lp1, i, GLP_DB, 0, 1);
        }
        glp_set_obj_coef(lp1, i, 0);
    }

    int px = 1;
    int index = 1;
    int row_num = 1;

    //for each course there is exactly one slot
    for (int i = 1; i <= m; i++)
    {
        for (int j = 1; j <= o; j++)
        {
            x[index] = row_num;
            y[index] = px;
            ar[index] = 1;
            index++;
            px++;
        }
        row_num++;
    }

    int shift = m * o;
    int q = 1;
    //course i and j are clashing through slot k
    for (int i = 1; i <= m - 1; i++)
    {
        for (int j = i + 1; j <= m; j++)
        {

            for (int k = 1; k <= o; k++)
            {
                x[index] = row_num;
                y[index] = shift + q;
                ar[index] = -1;
                index++;

                int g = (i - 1) * o;
                x[index] = row_num;
                y[index] = g + k;
                ar[index] = 1;
                index++;

                g = (j - 1) * o;
                x[index] = row_num;
                y[index] = g + k;
                ar[index] = 1;
                index++;
                row_num++;

                q++;
            }
        }
    }

    int curbase = m * o + 1;
    //setting coefficient of objective function
    for (int j = 1; j <= m - 1; j++)
    {
        for (int k = j + 1; k <= m; k++)
        {
            int pair = 0;
            for (int i = 1; i <= n; i++)
            {
                if (xsc[i][j] == 1 && xsc[i][k] == 1)
                {
                    pair++;
                }
            }
            for (int l = 1; l <= o; l++)
            {
                glp_set_obj_coef(lp1, curbase, pair);
                curbase++;
            }
        }
    }

    glp_load_matrix(lp1, index - 1, x, y, ar);
    glp_simplex(lp1, NULL);

    glp_iocp parm;
    parm.presolve = GLP_ON;
    glp_init_iocp(&parm);
    glp_intopt(lp1, &parm);
    int courseclash = glp_mip_obj_val(lp1);

    //ysc[i][j]=1 means jth course is assign to slot i
    for (int j = 1; j <= m; j++)
    {

        for (int i = 1; i <= o; i++)
        {
            int val = (j - 1) * o + i;
            ysc[i][j] = glp_mip_col_val(lp1, val);
        }
    }

    glp_delete_prob(lp1);
    glp_free_env();
    return courseclash;
}
void lp2(int **a, int *b, double **xsc, double **ysc, double lambda)
{

    glp_prob *lpp;
    lpp = glp_create_prob();
    glp_set_prob_name(lpp, "lp2");
    glp_set_obj_dir(lpp, GLP_MIN);

    int row1 = 2 * n + m + (m * (m - 1) * n / 2);
    int col1 = n * m + (m * (m - 1) * n / 2);

    //row constraints
    glp_add_rows(lpp, row1);
    int sum = 0;

    for (int i = 1; i <= n; i++)
    {
        glp_set_row_bnds(lpp, i, GLP_UP, 0.0, N);
    }
    for (int i = n + 1; i <= n + m; i++)
    {
        glp_set_row_bnds(lpp, i, GLP_UP, 0.0, b[i - n]);
    }
    for (int i = n + m + 1; i <= n + m + n; i++)
    {
        glp_set_row_bnds(lpp, i, GLP_FX, 0, 0);
    }
    sum = 2 * n + m;

    for (int i = sum + 1; i <= sum + (m * (m - 1) * n) / 2; i++)
    {
        glp_set_row_bnds(lpp, i, GLP_UP, 0, 1);
    }
    //column constraints
    glp_add_cols(lpp, col1);
    for (int i = 1; i <= col1; i++)

    {
        if (i <= m * n)
        {

            glp_set_col_bnds(lpp, i, GLP_DB, 0, 1);
            glp_set_col_kind(lpp, i, GLP_BV);
        }
        else
        {
            glp_set_col_bnds(lpp, i, GLP_DB, 0, 1);
        }
        if (i <= n * m)
        {
            glp_set_obj_coef(lpp, i, -1);
        }
    }

    int count = 1;
    int rn = 1;

    //A student can take at most N courses
    for (int i = 1; i <= n; i++)
    {
        for (int j = 1; j <= m; j++)
        {
            if (a[i][j] == 1)
            {
                int val = (j - 1) * n + i;
                x[count] = rn;
                y[count] = val;
                ar[count] = 1.0;
                count++;
            }
        }
        rn++;
    }

    //At most b[j] students can take course j
    for (int j = 1; j <= m; j++)
    {
        for (int i = 1; i <= n; i++)
        {
            if (a[i][j] == 1)
            {
                int val = (j - 1) * n + i;
                x[count] = rn;
                y[count] = val;
                ar[count] = 1.0;
                count++;
            }
        }
        rn++;
    }

    //Student can be assinged courses from their preference only
    for (int i = 1; i <= n; i++)
    {
        for (int j = 1; j <= m; j++)
        {
            if (a[i][j] != 1)
            {
                int val = (j - 1) * n + i;
                x[count] = rn;
                y[count] = val;
                ar[count] = 1.0;
                count++;
            }
        }
        rn++;
    }

    int change = m * n;
    int qx = 1;
    //Student k has taken both courses i and j
    for (int i = 1; i <= m - 1; i++)
    {
        for (int j = i + 1; j <= m; j++)
        {

            for (int k = 1; k <= n; k++)
            {
                x[count] = rn;
                y[count] = change + qx;
                ar[count] = -1;
                count++;

                int g = (i - 1) * n;
                x[count] = rn;
                y[count] = g + k;
                ar[count] = 1;
                count++;

                g = (j - 1) * n;
                x[count] = rn;
                y[count] = g + k;
                ar[count] = 1;
                count++;

                rn++;

                qx++;
            }
        }
    }

    int cur_base = m * n + 1;

    //setting coefficient for objective function
    for (int j = 1; j <= m - 1; j++)
    {
        for (int k = j + 1; k <= m; k++)
        {
            int pair = 0;
            for (int l = 1; l <= o; l++)
            {
                if (ysc[l][j] == 1 && ysc[l][k] == 1)
                {
                    pair++;
                }
            }
            for (int i = 1; i <= n; i++)
            {
                glp_set_obj_coef(lpp, cur_base, pair * lambda);
                cur_base++;
            }
        }
    }

    glp_load_matrix(lpp, count - 1, x, y, ar);
    glp_simplex(lpp, NULL);

    glp_iocp parm;
    parm.presolve = GLP_ON;
    glp_init_iocp(&parm);
    glp_intopt(lpp, &parm);

    //xsc[i][j]=1 means student i has allocated course j
    for (int i = 1; i <= n; i++)
    {
        for (int j = 1; j <= m; j++)
        {
            int val = (j - 1) * n + i;
            xsc[i][j] = glp_mip_col_val(lpp, val);
        }
    }

    glp_delete_prob(lpp);
    glp_free_env();
}
void print_student_course_allocation(double **xsc)
{
    int total_c = 0;
    cout << "Student-Course allocation" << endl;
    for (int i = 1; i <= n; i++)
    {
        cout << i << " --> ";
        bool flag = true;
        for (int j = 1; j <= m; j++)
        {
            if (xsc[i][j] == 1)
            {
                if (!flag)
                {
                    cout << ",";
                }
                flag = false;
                cout << j;
                total_c++;
            }
        }
        cout << endl;
    }
    cout << "total_courses: " << total_c << endl;
}

void print_course_slot_allocation(double **ysc)
{
    cout << "Course-Slot allocation" << endl;
    for (int j = 1; j <= m; j++)
    {
        cout << j << " --> ";
        bool flag = true;
        for (int i = 1; i <= o; i++)
        {

            if (ysc[i][j] == 1)
            {
                if (!flag)
                {
                    cout << ",";
                }
                flag = false;
                cout << i;
            }
        }
        cout << endl;
    }
}

int main()
{

    scanf("%d,%d,%d", &n, &m, &o);

    int **a = (int **)malloc((n + 1) * sizeof(int *));
    double **xsc = (double **)malloc((n + 1) * sizeof(double *));
    double **ysc = (double **)malloc((n + 1) * sizeof(double *));

    for (int i = 0; i <= n; i++)
    {
        a[i] = (int *)malloc((m + 1) * sizeof(int));
        xsc[i] = (double *)malloc((m + 1) * sizeof(double));
        ysc[i] = (double *)malloc((m + 1) * sizeof(double));

        for (int j = 0; j <= m; j++)
        {
            a[i][j] = 0;
            xsc[i][j] = 0;
            ysc[i][j] = 0;
        }
    }

    cin >> N;
    int b[m + 1];

    for (int i = 1; i <= m; i++)
    {
        if (i < m)
            scanf("%d,", &b[i]);
        else
            scanf("%d", &b[i]);
    }
    int prefer;
    cin >> prefer;

    for (int k = 0; k < prefer; k++)
    {
        int s, c;
        scanf("%d,%d", &s, &c);
        a[s][c] = 1;
    }
    double lambda = 0;
    int i = 0;

    //Iterative procedure for finding allocation with zero conflicts
    while (true)
    {

        lp2(a, b, xsc, ysc, lambda);
        int clash = lp1(xsc, ysc);

        if (clash == 0)
        {

            cout << "Lambda value ( For Zero clashes): " << lambda << endl;
            print_student_course_allocation(xsc);
            cout << endl;
            print_course_slot_allocation(ysc);

            break;
        }
        lambda += 0.5;  // gradually increasing lambda
        i++;
    }
    for (int i = 0; i <= n; i++)
    {
        free(a[i]);
        free(xsc[i]);
        free(ysc[i]);
    }
    free(a);
    free(xsc);
    free(ysc);

    return 0;
}
