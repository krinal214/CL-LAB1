/*
No of floors:6 (from 0 to 5)
No of lifts:2 (L0 and L1)
No of person:9 (1 to 9)
*/

#include <iostream>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)
#define NFLOORS 5
#define NLIFTS 2

using namespace std;

struct sembuf pop, vop;
int shared_floor_info;
int shared_lift_info;
int shared_floor_lift_up;
int shared_floor_lift_down;

int get_floor(int seed)
{
    srand(seed);
    int num = (rand() % (NFLOORS + 1));
    return num;
}

typedef struct
{
    int waiting_to_go_up;
    int waiting_to_go_down;
    int up_arrow;
    int down_arrow;

} floor_info;

typedef struct
{
    int no;
    int position;
    int direction;
    int people_in_lift;
    int stops[NFLOORS + 1];
    int stopsem[NFLOORS + 1];

} lift_info;

int floor_mutex[NFLOORS + 1]; //floor level semaphore to achieve mutual exclusion
int waiting_mutex[NLIFTS];    //semaphore to wait for persons to enter into the lift after waking up persons
int lift_mutex[NLIFTS];       // lift semaphore to achive mutual exclusion
int floor_lift_mutex[2];      // sempare to update floor_lift data structure in mutual exclusion manner
int print_semaphore;          // semaphore to avoid multiple processes to print simultaneously

floor_info *level;  //floor info
int *floor_lift[2]; // stores which lift is at floor and direction is up or down
lift_info *lifts;   // lift info

void drop(lift_info *lifts, int lno)
{
    int floor_n = lifts[lno].position;

    P(lift_mutex[lno]);
    int persons = lifts[lno].stops[floor_n]; // check how many persons want to leave
    V(lift_mutex[lno]);

    if (persons == 0)
        return;

    for (int i = 1; i <= persons; i++)
    {
        V(lifts[lno].stopsem[floor_n]); //wake up person to leave the lift
    }
    P(lift_mutex[lno]);
    lifts[lno].people_in_lift -= persons;
    lifts[lno].stops[floor_n] -= persons;
    V(lift_mutex[lno]);

    P(print_semaphore);
    cout << "Lift L" << lno << " has dropped " << persons << " persons on floor " << floor_n << endl;
    cout.flush();
    V(print_semaphore);
}

void pickup(lift_info *lifts, int lno, floor_info *level, int *floor_lift[2])
{
    int direction = lifts[lno].direction;
    int f = lifts[lno].position;
    int k = 0;

    P(floor_mutex[f]);
    if (direction == 0)
    {
        k = level[f].waiting_to_go_down;
    }
    else
    {
        k = level[f].waiting_to_go_up;
    }
    V(floor_mutex[f]);

    if (direction == 0)
    {
        for (int i = 1; i <= k; i++)
        {
            V(level[f].down_arrow); //wakeup person who want to go down
        }
    }
    else
    {
        for (int i = 1; i <= k; i++)
        {
            V(level[f].up_arrow); //wakeup person who want to go up
        }
    }

    for (int i = 1; i <= k; i++)
    {
        P(waiting_mutex[lno]); //wait for the people to enter into lift
    }

    if (k == 0)
        return;

    P(floor_mutex[f]);
    if (direction == 0)
    {
        level[f].waiting_to_go_down -= k;
    }
    else
    {
        level[f].waiting_to_go_up -= k;
    }
    lifts[lno].people_in_lift += k;
    V(floor_mutex[f]);

    P(floor_lift_mutex[direction]);
    floor_lift[direction][f] = -1;
    V(floor_lift_mutex[direction]);

    P(print_semaphore);
    cout << k << " people entered into lift L" << lno << " on floor " << f << endl;
    cout.flush();
    V(print_semaphore);
}

void lift(int lno, int speed)
{
    /*
    lift requires to drop and pick up people from floor. Thus, it updates floor structure
    */
    floor_info *level;
    int *floor_lift[2];
    lift_info *lifts;
    lifts = (lift_info *)shmat(shared_lift_info, 0, 0);
    level = (floor_info *)shmat(shared_floor_info, 0, 0);
    floor_lift[0] = (int *)shmat(shared_floor_lift_down, 0, 0);
    floor_lift[1] = (int *)shmat(shared_floor_lift_up, 0, 0);

    while (true)
    {

        drop(lifts, lno); // drop all the people whose destination floor is current floor
        sleep(1);

        int people = 0;

        if (lifts[lno].direction == 1 || lifts[lno].people_in_lift == 0) // either lift is going up or no one in the lift
        {

            int x = -1;

            P(floor_lift_mutex[1]);
            x = floor_lift[1][lifts[lno].position];
            if (x == -1)
            {
                floor_lift[1][lifts[lno].position] = lno; //update its current floor
                x = lno;
            }
            V(floor_lift_mutex[1]);

            if (x == lno)
            {
                people = 0;
                P(floor_mutex[lifts[lno].position]);
                people = level[lifts[lno].position].waiting_to_go_up; // check how many person want to go up
                V(floor_mutex[lifts[lno].position]);

                if (people > 0)
                {
                    if (lifts[lno].people_in_lift == 0)
                    {
                        P(lift_mutex[lno]);
                        lifts[lno].direction = 1;
                        V(lift_mutex[lno]);
                    }
                    pickup(lifts, lno, level, floor_lift); // pick up if there is atleast one person who wants to go up
                }
                else
                {
                    P(floor_lift_mutex[1]);
                    floor_lift[1][lifts[lno].position] = -1;
                    V(floor_lift_mutex[1]);
                }
            }
        }
        if (lifts[lno].direction == 0 || lifts[lno].people_in_lift == 0)
        {

            int x = -1;
            P(floor_lift_mutex[0]);
            x = floor_lift[0][lifts[lno].position];
            if (x == -1)
            {
                floor_lift[0][lifts[lno].position] = lno; //update its current floor
                x = lno;
            }
            V(floor_lift_mutex[0]);
            if (x == lno)
            {
                people = 0;
                P(floor_mutex[lifts[lno].position]);
                people = level[lifts[lno].position].waiting_to_go_down; // check how many person want to go down
                V(floor_mutex[lifts[lno].position]);
                if (people > 0)
                {
                    if (lifts[lno].people_in_lift == 0)
                    {
                        P(lift_mutex[lno]);
                        lifts[lno].direction = 0;
                        V(lift_mutex[lno]);
                    }
                    pickup(lifts, lno, level, floor_lift); // pick up if there is atleast one person who wants to go down
                }
                else
                {
                    P(floor_lift_mutex[0]);
                    floor_lift[0][lifts[lno].position] = -1;
                    V(floor_lift_mutex[0]);
                }
            }
        }

        sleep(speed);

        if (lifts[lno].direction == 0)
        {
            lifts[lno].position = lifts[lno].position - 1;
            if (lifts[lno].position == 0)
            {
                P(lift_mutex[lno]);
                lifts[lno].direction = 1;
                V(lift_mutex[lno]);
            }
        }
        else
        {
            lifts[lno].position = lifts[lno].position + 1;
            if (lifts[lno].position == NFLOORS)
            {
                P(lift_mutex[lno]);
                lifts[lno].direction = 0;
                V(lift_mutex[lno]);
            }
        }
    }
}

int get_sleep(int seed)
{
    srand(seed);
    int num = (rand() % (7)) + 2;
    return num;
}
void person(int p)
{
    /*
    person is at floor and it decides to go up or down. Thus, it requires to update floor structure.
    After entering into lift person press destination floor button. Thus, it requires to update lift structure.
    */

    floor_info *level;
    int *floor_lift[2];
    lift_info *lifts;

    level = (floor_info *)shmat(shared_floor_info, 0, 0);
    lifts = (lift_info *)shmat(shared_lift_info, 0, 0);
    floor_lift[0] = (int *)shmat(shared_floor_lift_down, 0, 0);
    floor_lift[1] = (int *)shmat(shared_floor_lift_up, 0, 0);
    int count = 1;
    int start_floor = get_floor(161 * p + 31 * count); // initially person can be at any floor

    while (true)
    {
        sleep(get_sleep(p * p + 113 * start_floor));

        int dest_floor = get_floor(p * p + 41 * count + 311 * start_floor); // select a destination floor
        count++;
        count %= 10007;

        while (dest_floor == start_floor)
        {
            dest_floor = get_floor(p * p + 41 * count + 311 * start_floor);
            count++;
            count %= 10007;
        }
        count %= 10007;

        if (dest_floor > start_floor)
        {
            P(floor_mutex[start_floor]);
            level[start_floor].waiting_to_go_up++; //press up button
            V(floor_mutex[start_floor]);

            P(print_semaphore);
            cout << "Person_Id: " << p << " Start: " << start_floor << " Destination: " << dest_floor << endl;
            cout.flush();
            V(print_semaphore);

            P(level[start_floor].up_arrow); // waiting till lift wakes up

            int lift_n = 0;

            P(floor_lift_mutex[1]);
            lift_n = floor_lift[1][start_floor]; //check which lift is at start floor which is going up
            V(floor_lift_mutex[1]);

            V(waiting_mutex[lift_n]); // inform lift that person has entered into lift

            P(lift_mutex[lift_n]);
            lifts[lift_n].stops[dest_floor]++; // person press destination floor
            V(lift_mutex[lift_n]);

            P(lifts[lift_n].stopsem[dest_floor]); // person waits till destination floor arives
        }
        else
        {

            P(floor_mutex[start_floor]);
            level[start_floor].waiting_to_go_down++; //press down button
            V(floor_mutex[start_floor]);

            P(print_semaphore);
            cout << "Person_Id: " << p << " Start: " << start_floor << " Destination: " << dest_floor << endl;
            cout.flush();
            V(print_semaphore);

            P(level[start_floor].down_arrow); // waiting till lift wakes up

            int lift_n = 0;

            P(floor_lift_mutex[0]);
            lift_n = floor_lift[0][start_floor]; //check which lift is at start floor which is going down
            V(floor_lift_mutex[0]);

            V(waiting_mutex[lift_n]); // inform lift that person has entered into lift

            P(lift_mutex[lift_n]);
            lifts[lift_n].stops[dest_floor]++; // person press destination floor
            V(lift_mutex[lift_n]);

            P(lifts[lift_n].stopsem[dest_floor]); // person waits till destination floor arives
        }

        P(print_semaphore);
        cout << "Person " << p << " reached to destination:" << dest_floor << endl;
        cout.flush();
        V(print_semaphore);

        start_floor = dest_floor;
        count++;
    }
}

lift_info create_lift(int lno, int speed)
{
    lift_info lift;
    lift.no = lno;
    lift.direction = 1;
    lift.position = 0;
    lift.people_in_lift = 0;

    for (int i = 0; i <= NFLOORS; i++)
        lift.stops[i] = 0;
    return lift;
}
void init()
{
    /* Creating shared memory */
    shared_floor_info = shmget(IPC_PRIVATE, (NFLOORS + 1) * sizeof(floor_info), 0777 | IPC_CREAT);
    shared_lift_info = shmget(IPC_PRIVATE, (NLIFTS) * sizeof(lift_info), 0777 | IPC_CREAT);
    shared_floor_lift_up = shmget(IPC_PRIVATE, (NFLOORS + 1) * sizeof(int), 0777 | IPC_CREAT);
    shared_floor_lift_down = shmget(IPC_PRIVATE, (NFLOORS + 1) * sizeof(int), 0777 | IPC_CREAT);

    lifts = (lift_info *)shmat(shared_lift_info, 0, 0);
    level = (floor_info *)shmat(shared_floor_info, 0, 0);
    floor_lift[0] = (int *)shmat(shared_floor_lift_up, 0, 0);
    floor_lift[1] = (int *)shmat(shared_floor_lift_down, 0, 0);

    for (int i = 0; i < 2; i++)
    {
        lifts[i] = create_lift(i, i + 1);
    }
    for (int i = 0; i <= NFLOORS; i++)
    {
        level[i].waiting_to_go_down = 0;
        level[i].waiting_to_go_up = 0;
        floor_lift[0][i] = -1;
        floor_lift[1][i] = -1;
    }
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = SEM_UNDO;
    pop.sem_op = -1; //wait operation
    vop.sem_op = 1;  //signal operation

    for (int i = 0; i <= NFLOORS; i++)
    {
        level[i].up_arrow = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT);
        level[i].down_arrow = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT);
        floor_mutex[i] = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT);

        semctl(level[i].up_arrow, 0, SETVAL, 0);
        semctl(level[i].down_arrow, 0, SETVAL, 0);
        semctl(floor_mutex[i], 0, SETVAL, 1);
    }
    for (int i = 0; i < NLIFTS; i++)
    {
        waiting_mutex[i] = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT);
        lift_mutex[i] = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT);

        semctl(waiting_mutex[i], 0, SETVAL, 0);
        semctl(lift_mutex[i], 0, SETVAL, 1);

        for (int j = 0; j <= NFLOORS; j++)
        {
            lifts[i].stopsem[j] = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT);
            semctl(lifts[i].stopsem[j], 0, SETVAL, 0);
        }
    }
    floor_lift_mutex[0] = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT);
    floor_lift_mutex[1] = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT);

    print_semaphore = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT);

    semctl(print_semaphore, 0, SETVAL, 1);
    semctl(floor_lift_mutex[0], 0, SETVAL, 1);
    semctl(floor_lift_mutex[1], 0, SETVAL, 1);
}

void print_state(int floor_up[NFLOORS + 1], int floor_down[NFLOORS + 1], int lift_pos[NLIFTS], int direction[NLIFTS], int people_lift[NLIFTS])
{

    string u = "/\\";
    string d = "\\/";

    const char *dir[2] = {d.c_str(), u.c_str()};
    P(print_semaphore);
    cout << endl;
    printf("\t\t    |    L0     |    L1     |\n");
    cout << "---------------------------------------------" << endl;
    for (int i = NFLOORS; i >= 0; i--)
    {

        printf("Floor:%d UP:%d Down:%d |", i, floor_up[i], floor_down[i]);

        for (int j = 0; j < NLIFTS; j++)
        {
            if (lift_pos[j] == i)
            {
                printf("  %s P:%d   |", dir[direction[j]], people_lift[j]);
            }

            else
            {
                printf("           |");
            }
        }
        cout << endl;
        cout << "---------------------------------------------" << endl;
    }
    cout.flush();
    V(print_semaphore);
}

int main()
{

    init();
    if (fork() == 0)
    {

        int floor_up[NFLOORS + 1], floor_down[NFLOORS + 1];
        int people_lift[NLIFTS];
        int direction_lift[NLIFTS];
        int position_lift[NLIFTS];
        while (1)
        {

            for (int i = 0; i <= NFLOORS; i++)
            {

                P(floor_mutex[i]);
                floor_up[i] = level[i].waiting_to_go_up;
                floor_down[i] = level[i].waiting_to_go_down;
                V(floor_mutex[i]);
            }
            for (int i = 0; i < NLIFTS; i++)
            {
                P(lift_mutex[i]);
                people_lift[i] = lifts[i].people_in_lift;
                direction_lift[i] = lifts[i].direction;
                position_lift[i] = lifts[i].position;
                V(lift_mutex[i]);
            }
            print_state(floor_up, floor_down, position_lift, direction_lift, people_lift);
            sleep(1);
        }
    }
    else if (fork() == 0)
    {
        lift(0, 1);
    }
    else if (fork() == 0)
    {
        lift(1, 2);
    }
    else if (fork() == 0)
    {
        person(1);
    }
    else if (fork() == 0)
    {
        sleep(1);
        person(2);
    }
    else if (fork() == 0)
    {
        sleep(2);
        person(3);
    }
    else if (fork() == 0)
    {
        sleep(1);
        person(4);
    }
    else if (fork() == 0)
    {
        sleep(2);
        person(5);
    }
    else if (fork() == 0)
    {
        sleep(1);
        person(6);
    }
    else if (fork() == 0)
    {
        sleep(3);
        person(7);
    }
    else if (fork() == 0)
    {
        sleep(1);
        person(8);
    }
    else if (fork() == 0)
    {
        sleep(3);
        person(9);
    }
    int wpid;
    int status = 0;
    while ((wpid = wait(&status)) > 0)
        ;

    return 0;
}