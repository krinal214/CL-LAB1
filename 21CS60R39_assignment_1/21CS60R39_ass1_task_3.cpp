#include <iostream>
#include <math.h>
using namespace std;
typedef long double ld;

/*x,y are cordinates, d is 1 based index*/
struct Point
{
    ld x, y;
    int d;
};

struct Node
{
    Point p;
    Node *left;
    Node *right;
};

/*function to compare point p1 and p2 based on axis
axis=0 means X axis
axis=1 means Y axis
*/
bool cmp(Point p1, Point p2, int axis)
{
    if (axis == 0)
    {
        if (p1.x < p2.x)
            return true;
        else if (p1.x == p2.x && p1.y <= p2.y)
            return true;
        else
            return false;
    }
    else
    {
        if (p1.y < p2.y)
            return true;
        else if (p1.y == p2.y && p1.x <= p2.x)
            return true;
        else
            return false;
    }
}

/*Function to merge two sorted array*/
void merge(int start, int mid, int end, Point P[], Point aux[], int axis)
{
    int i = start;
    int j = mid + 1;
    int k = start;

    while (i <= mid && j <= end)
    {
        if (cmp(P[i], P[j], axis))
        {
            aux[k] = P[i];
            i++;
        }
        else
        {
            aux[k] = P[j];
            j++;
        }
        k++;
    }
    while (i <= mid)
    {
        aux[k] = P[i];
        i++;
        k++;
    }
    while (j <= end)
    {
        aux[k] = P[j];
        j++;
        k++;
    }
    for (int g = start; g <= end; g++)
        P[g] = aux[g];
}

/*Utility function to sort an array based on axis*/
void mergesort(int start, int end, Point P[], Point A[], int axis)
{
    if (start == end)
    {
        A[start] = P[start];
    }
    else if (start < end)
    {
        int mid = (start + end) / 2;
        mergesort(start, mid, P, A, axis);
        mergesort(mid + 1, end, P, A, axis);
        merge(start, mid, end, P, A, axis);
    }
}

/*Euclidean distance between two points*/
ld distance(Point p1, Point p2)
{
    ld dis = sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
    return dis;
}

/*Utility function to build an K-D Tree*/
Node *build(Point xx[], Point yy[], int start, int end, int axis, int level)
{
    if (start > end)
    {
        return NULL;
    }

    Node *node = NULL;
    int mid = (start + end) / 2;
    Point temp1[(mid - start)];
    Point temp2[(end - mid)];
    bool flag = false;
    int k1 = 0;
    int k2 = 0;
    if (axis == 0)
    {
        for (int j = start; j <= end; j++)
        {
            if (xx[mid].d == yy[j].d)
            {
                flag = true;
            }
            else if (k1 < (mid - start) && cmp(yy[j], xx[mid], axis))
                temp1[k1++] = yy[j];
            else
                temp2[k2++] = yy[j];
        }
        yy[mid] = xx[mid];
        k1 = 0;
        k2 = 0;
        for (int j = start; j < mid; j++)
        {
            yy[j] = temp1[k1++];
        }
        for (int j = mid + 1; j <= end; j++)
        {
            yy[j] = temp2[k2++];
        }
        node = new Node();
        node->p = xx[mid];
        node->left = build(xx, yy, start, mid - 1, 1 - axis, level + 1);
        node->right = build(xx, yy, mid + 1, end, 1 - axis, level + 1);
    }
    else
    {
        for (int j = start; j <= end; j++)
        {
            if (yy[mid].d == xx[j].d)
            {
                flag = true;
            }
            else if (k1 < (mid - start) && cmp(xx[j], yy[mid], axis))
                temp1[k1++] = xx[j];
            else
                temp2[k2++] = xx[j];
        }
        xx[mid] = yy[mid];
        k1 = 0;
        k2 = 0;
        for (int j = start; j < mid; j++)
        {
            xx[j] = temp1[k1++];
        }
        for (int j = mid + 1; j <= end; j++)
        {
            xx[j] = temp2[k2++];
        }
        node = new Node();
        node->p = yy[mid];
        node->left = build(xx, yy, start, mid - 1, 1 - axis, level + 1);
        node->right = build(xx, yy, mid + 1, end, 1 - axis, level + 1);
    }

    return node;
}
/*Search utility to find a nearest base station of a mobile using KD Tree*/
void searchNeighbor(Point mob, Node *root, int axis, ld &min, int &index)
{
    if (root == NULL)
        return;
    else
    {
        ld dis = distance(root->p, mob);

        if (min < 0)
        {
            min = dis;
            index = root->p.d;
        }
        else if (dis < min)
        {
            min = dis;
            index = root->p.d;
        }

        if (cmp(mob, root->p, axis))
        {
            searchNeighbor(mob, root->left, 1 - axis, min, index);
            if (axis == 0)
            {
                if (min < 0 || abs(root->p.x - mob.x) < min)
                {
                    searchNeighbor(mob, root->right, 1 - axis, min, index);
                }
            }
            else
            {
                if (min < 0 || abs(root->p.y - mob.y) < min)
                {
                    searchNeighbor(mob, root->right, 1 - axis, min, index);
                }
            }
        }
        else
        {
            searchNeighbor(mob, root->right, 1 - axis, min, index);
            if (axis == 0)
            {
                if (min < 0 || abs(root->p.x - mob.x) < min)
                {
                    searchNeighbor(mob, root->left, 1 - axis, min, index);
                }
            }
            else
            {
                if (min < 0 || abs(root->p.y - mob.y) < min)
                {
                    searchNeighbor(mob, root->left, 1 - axis, min, index);
                }
            }
        }
    }
}

int main()
{

    int m, n;
    cin >> m;

    Point base[m];
    Point xx[m], yy[m];
    for (int i = 0; i < m; i++)
    {
        scanf("%Lf,%Lf", &(base[i].x), &(base[i].y));
        base[i].d = (i + 1);
    }
    cin >> n;
    Point mobile[n];
    for (int i = 0; i < n; i++)
    {
        scanf("%Lf,%Lf", &(mobile[i].x), &(mobile[i].y));
    }

    mergesort(0, m - 1, base, xx, 0);
    mergesort(0, m - 1, base, yy, 1);

    Node *root = build(xx, yy, 0, m - 1, 0, 1);
    int ans[n];

    for (int i = 0; i < n; i++)
    {
        int index = 0;
        ld min = -1;

        searchNeighbor(mobile[i], root, 0, min, index);
        ans[i] = index;
    }

    for (int i = 0; i < n; i++)
    {
        printf("[%d] --> [%d]\n", (i + 1), (ans[i]));
    }
    return 0;
}