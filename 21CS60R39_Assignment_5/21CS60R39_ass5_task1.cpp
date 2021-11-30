#include <iostream>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <string>
#include <math.h>
using namespace std;
typedef long long ll;

//child process c1
void child1()
{

    for (ll i = 1; i <= 100000; i++)
    {
        for (ll j = 1; j <= 1000000; j++)
        {
            ll q = (i * j) % 19;
            q = q * 2;
            for (int k = 1; k < 100000; k++)
            {
                ll x=pow(2,k%10);
                q+=x;
            
            }
            for (int p = 1; p <= 10000; p++)
            {
                int *a = new int;
            }
        }
        sleep(5);
    }
}

//child process c2
void child2()
{

    for (ll i = 1; i <= 20; i++)
    {
        ll j = 1 + i;
        for (ll j = 1; j <= 10000000; j++)
        {
            ll k = (i * j) % 19;
            if (j % 2 == 0)
                ll *p = new ll;
        }

        sleep(2);
    }
}

//child process c3
void child3()
{
    for (ll i = 1; i <= 10000; i++)
    {
        ll j = 1 + i;
        for (ll j = 1; j <= 10000000; j++)
        {

            ll k = (i * j + i + j) % 19;
            for (int p = 1; p <= j; p++)
                int *ar = new int;
            for (ll k = 1; k <= 100000; k++)
            {
                double d = floor(k / 2.0);
            }
        }

        sleep(5);
    }
}

int main()
{
    int ids[3];
    bool visited[3] = {false};
    int alive = 3; // maintain the no of alive processess
    ids[0] = ids[1] = ids[2] = 0;
    int fd[2];
    pipe(fd);
    ids[0] = fork();
    if (ids[0] == 0)
    {
        child1(); //fork child c1
    }
    else
    {
        ids[1] = fork();
        if (ids[1] == 0)
        {
            child2(); // fork child c2
        }
        else
        {
            ids[2] = fork();

            if (ids[2] == 0)
            {
                child3(); // fork child c3
            }
            else
            {

                while (alive > 0)
                {
                    int m = fork(); // fork child m at certain interval to get the memory and cpu utilization of child processes
                    if (m == 0)
                    {
                        close(1);

                        dup(fd[1]);
                        close(fd[0]);
                        string s = to_string(ids[0]) + " " + to_string(ids[1]) + " " + to_string(ids[2]);
                        const char *id_list = s.c_str();

                        execlp("/bin/ps", "/bin/ps", "-p", id_list, "-o", "pid,%mem,\%cpu", NULL); // process status query
                    }
                    else
                    {

                        close(0);
                        dup(fd[0]);
                        int i = 0;

                        string col1, col2, col3;
                        fprintf(stdout, "Timestamp: %lu\n", (unsigned long)time(NULL));
                        cin >> col1 >> col2 >> col3;
                        cout << " " << col1 << "     " << col2 << "   " << col3 << endl;

                        for (int i = 1; i <= alive; i++)
                        {
                            int pid;
                            double mem, cpu;

                            cin >> pid >> mem >> cpu;

                            printf("%5d  %6.2lf  %6.2lf   \n", pid, mem, cpu);
                        }
                        cout << "------------------------------" << endl;
                        sleep(2);
                        int status;
                        for (int i = 0; i <= 2; i++)
                        {
                            if (visited[i])
                            {
                                continue;
                            }
                            else
                            {
                                /*A zombie process is a process whose execution is completed but it still has an entry in the process table.
                                 Zombie processes usually occur for child processes, as the parent process still needs to read its child’s exit status. 
                                 Once this is done using the wait system call, the zombie process is eliminated from the process table.*/

                                int cid = waitpid(ids[i], &status, WNOHANG);
                                if (cid != 0)
                                {
                                    cout << "**********Terminated child process: " << cid << "  *****************" << endl;
                                    visited[i] = true;
                                    alive--;
                                }
                            }
                        }
                    }
                }
                if (alive == 0)
                {
                    cout << "All child processess have been terminated" << endl;
                }
            }
        }
    }

    return 0;
}