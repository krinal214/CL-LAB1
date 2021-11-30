#include <iostream>
#include <math.h>
using namespace std;
typedef long double ld;

struct Point
{
  ld x, y;
};

/*Eculidean Distance between Point p1 and p2 */
ld distance(Point p1, Point p2)
{
  ld dis = sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
  return dis;
}

/*Utility function to find nearest base station for a mobile*/
int searchNeighbor(Point mob, Point base[], int m)
{
  ld minval;
  int minidx = -1;

  for (int j = 0; j < m; j++)
  {
    ld dis = distance(mob, base[j]);
    if (minidx == -1)
    {
      minval = dis;
      minidx = j + 1;
    }
    else if (dis < minval)
    {
      minval = dis;
      minidx = j + 1;
    }
  }
  return minidx;
}

int main()
{

  int m, n;
  cin >> m;
  Point base[m];

  for (int i = 0; i < m; i++)
  {
    scanf("%Lf,%Lf", &(base[i].x), &(base[i].y));
  }

  cin >> n;
  Point mobile[n];

  for (int i = 0; i < n; i++)
  {
    scanf("%Lf,%Lf", &(mobile[i].x), &(mobile[i].y));
  }

  int ans[n];
  for (int i = 0; i < n; i++)
  {

    ans[i] = searchNeighbor(mobile[i], base, m);
  }

  for (int i = 0; i < n; i++)
  {
    printf("[%d] --> [%d]\n", (i + 1), (ans[i]));
  }

  return 0;
}