/*
Even after applying optimization we will not be able to change the order of time complexity(O(MN)).
However, Because of optimization, fewer comparision will be required in certain cases.
*/
#include <iostream>
#include <math.h>
using namespace std;
typedef long double ld;

/*x,y are cordinates and d is a 0 based index*/
struct Point
{
  ld x, y;
  int d;
};
/*Euclidean disntace between two points*/
ld distance(Point p1, Point p2)
{
  ld dis = sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
  return dis;
}

int main()
{
  int m, n;
  cin >> m;

  Point base[m];
  for (int i = 0; i < m; i++)
  {
    scanf("%Lf,%Lf", &(base[i].x), &(base[i].y));
    base[i].d = i;
  }
  cin >> n;
  Point mobile[n];
  for (int i = 0; i < n; i++)
  {
    scanf("%Lf,%Lf", &(mobile[i].x), &(mobile[i].y));
    mobile[i].d = i;
  }

  int ans[n];
  for (int i = 0; i < n; i++)
  {
    ld minval;
    int minidx = -1;

    for (int j = 0; j < m; j++)
    {
      ld dis = distance(mobile[i], base[j]);
      if (minidx == -1)
      {
        minval = dis;
        minidx = j;
      }
      else if (dis < minval)
      {
        minval = dis;
        minidx = j;
      }
    }
    ans[i] = minidx + 1; // The nearest base station of a mobile before Update Queries
  }

  int m1, n1;
  cin >> m1;
  int changebase[m];
  Point newbase[m1];
  for (int i = 0; i < m; i++)
    changebase[i] = -1;

  for (int i = 0; i < m1; i++)
  {
    int loc;
    scanf("%d,%Lf,%Lf", &loc, &(newbase[i].x), &(newbase[i].y));
    newbase[i].d = loc - 1;
    changebase[loc - 1] = i;
  }

  cin >> n1;
  int changemob[n];
  Point newmob[n1];
  for (int i = 0; i < n; i++)
    changemob[i] = -1;

  for (int i = 0; i < n1; i++)
  {
    int loc;
    scanf("%d,%Lf,%Lf", &loc, &(newmob[i].x), &(newmob[i].y));
    newmob[i].d = loc - 1;
    changemob[loc - 1] = i;
  }

  for (int i = 0; i < n; i++)
  {

    if (changemob[i] == -1) // If phone location doesn't change
    {
      ld prevval = distance(mobile[i], base[ans[i] - 1]);
      int previdx = ans[i] - 1;
      ld newval = -1;
      int newidx = -1;
      for (int j = 0; j < m1; j++) /* compare with new base station location only*/
      {
        ld dis = distance(mobile[i], newbase[j]);
        if (newidx == -1)
        {
          newval = dis;
          newidx = newbase[j].d;
        }
        else if (dis < newval)
        {
          newval = dis;
          newidx = newbase[j].d;
        }
      }
      /*if minimum distance from new location of base station is <= previous minimum distance from all old base station
       then new minimum distance will be stored in ans array
      */
      if (newval >= 0 && newval <= prevval)
      {
        ans[i] = newidx + 1;
      }
      /*if minimum distance from new location of base station is > previous minimum distance from all old base station
       and location of previous nearest base station doesn't change 
       then previous ans doesn't change*/
      else if (changebase[previdx] == -1)
      {
      }
      /*if minimum distance from new location of base station is > previous minimum distance from all old base station
       and location of previous nearest base station changes
       */
      else
      {
        for (int r = 0; r < m; r++)
        {
          if (changebase[r] == -1)
          {
            ld dis = distance(mobile[i], base[r]);
            if (newidx == -1)
            {
              newval = dis;
              newidx = r;
            }
            else if (dis < newval)
            {
              newval = dis;
              newidx = r;
            }
          }
        }
        ans[i] = newidx + 1;
      }
    }

    /*mobile location changes*/
    else
    {

      ld newval = -1;
      int newidx = -1;
      for (int r = 0; r < m1; r++)
      {
        ld dis = distance(newmob[changemob[i]], newbase[r]);
        if (newidx == -1)
        {
          newval = dis;
          newidx = newbase[r].d;
        }
        else if (dis < newval)
        {
          newval = dis;
          newidx = newbase[r].d;
        }
      }

      for (int j = 0; j < m; j++)
      {
        if (changebase[j] == -1)
        {
          ld dis = distance(newmob[changemob[i]], base[j]);
          if (newidx == -1)
          {
            newval = dis;
            newidx = base[j].d;
          }
          else if (dis < newval)
          {
            newval = dis;
            newidx = base[j].d;
          }
        }
      }
      ans[i] = newidx + 1;
    }
  }

  for (int i = 0; i < n; i++)
  {
    printf("[%d] --> [%d]\n", (i + 1), (ans[i]));
  }

  return 0;
}
