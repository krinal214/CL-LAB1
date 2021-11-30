#include <iostream>
using namespace std;
int INF = 1e8 + 7;

struct Node
{
  int w, index;
};
struct Lnode
{
  Node node;
  Lnode *prev;
};
bool cmp(Node a, Node b)
{
  if (a.w < b.w)
    return true;
  else if (a.w == b.w && a.index < b.index)
    return true;
  else
    return false;
}
void heapify(Node heap[], int heapidx[], int index, int &size)
{
  int left = 2 * index;
  int right = left + 1;
  int max = index;
  if (left <= size && heap[left].w < heap[max].w)
  {
    max = left;
  }
  if (right <= size && heap[right].w < heap[max].w)
  {
    max = right;
  }
  if (max != index)
  {
    Node temp = heap[index];
    heap[index] = heap[max];
    heap[max] = temp;
    heapidx[heap[index].index] = index;
    heapidx[heap[max].index] = max;
    heapify(heap, heapidx, max, size);
  }
}
Node extractmin(Node heap[], int heapidx[], int &size)
{
  Node ans = heap[1];
  if (size > 1)
  {
    Node temp = heap[size];
    heap[size] = heap[1];
    heap[1] = temp;
    heapidx[heap[1].index] = 1;
  }

  heapidx[heap[size].index] = -1;
  size--;
  if (size > 1)
  {
    heapify(heap, heapidx, 1, size);
  }
  return ans;
}

void decreasekey(Node heap[], int heapidx[], int idx, int newval)
{
  heap[idx].w = newval;

  int i = idx;
  while (i > 1 && (heap[i / 2].w > heap[i].w))
  {
    int tempindex = heap[i / 2].index;
    int tempval = heap[i / 2].w;
    Node temp = heap[i / 2];
    heap[i / 2] = heap[i];
    heap[i] = temp;
    heapidx[heap[i].index] = i;
    heapidx[heap[i / 2].index] = i / 2;
    i = i / 2;
  }
}
int main()
{

  int n, m, source, k;
  scanf("%d,%d,%d,%d", &n, &m, &source, &k);
  Lnode *graph[n + 1];  //to store the graph in adjacency list
  Node minheap[n + 1];  //heap array
  int heapindex[n + 1]; //inverse heap index. To determine the index of a node in heap. So update can be done in constant time.
  for (int i = 0; i <= n; i++)
  {
    graph[i] = NULL;
    heapindex[i] = -1;
  }
  for (int i = 0; i < m; i++)
  {
    int start, end, weight;
    scanf("%d,%d,%d", &start, &end, &weight);

    Lnode *node1 = new Lnode();
    node1->node.w = weight;
    node1->node.index = end;

    if (graph[start] == NULL)
    {
      graph[start] = node1;
      node1->prev = NULL;
    }
    else
    {
      node1->prev = graph[start];
      graph[start] = node1;
    }
    Lnode *node2 = new Lnode();
    node2->node.w = weight;
    node2->node.index = start;

    if (graph[end] == NULL)
    {
      graph[end] = node2;
      node2->prev = NULL;
    }
    else
    {
      node2->prev = graph[end];
      graph[end] = node2;
    }
  }
  int j = 1;
  minheap[j].w = 0;
  minheap[j].index = source;
  heapindex[source] = j;
  j++;
  for (int i = 1; i <= n; i++)
  {
    if (i == source)
      continue;
    minheap[j].w = INF;
    minheap[j].index = i;
    heapindex[i] = j;
    j++;
  }
  heapindex[source] = 0;
  int heapsize = n;

  int count = 0;

  while (heapsize > 0)
  {

    Node a = extractmin(minheap, heapindex, heapsize);
    if(a.w==INF)break;
    Lnode *s = graph[a.index];
    while (s != NULL)
    {
      int idx = heapindex[s->node.index];
      if (idx == -1)
      {
      }
      else if (a.w + s->node.w < minheap[idx].w)
      {

        decreasekey(minheap, heapindex, idx, a.w + s->node.w);
      }

      s = s->prev;
    }
    count++;
    if (count > 1)
    {
      cout << a.index << endl;
    }

    if (count == k + 1)
      break;
  }

  return 0;
}