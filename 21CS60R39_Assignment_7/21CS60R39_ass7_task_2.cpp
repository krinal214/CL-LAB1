//Compile Command: g++ filename.cpp -pthread
//Execution Command: ./a.out datafile
//Enter the no of epoch and thread pool size in the prompt

#include <iostream>
#include <string>
#include <pthread.h>
#include <fstream>
#include <cmath>
#include <vector>
#include <ctime>
#define records 581012 // total no of training example
#define batch 1024     //mini batch size used in gradient descent
#define features 54    //total no of features in dataset
#define Nthreads 10    //maximum no of thread possible- will be overwritten by input

using namespace std;

pthread_mutex_t file_handle;
pthread_mutex_t permission[Nthreads];
pthread_cond_t permission_check[Nthreads];
pthread_mutex_t mutex_counter;
pthread_cond_t counter_threshold;
pthread_mutex_t learning_mutex;
pthread_mutex_t WB;

int permit[Nthreads] = {0};
double learning_rate = 0.1;
const char *file;
ifstream input_file;

double entire_data[54][records] = {0}; // used for accuracy calculation at last
double P[records];
double Q[records];
double D[records] = {0};
struct Node
{
    int id;
    Node *next;
};

Node *head1 = NULL;
Node *tail1 = NULL;

double W[features], B; // weight vector and bias

/*convert string int token*/
void tokenize(vector<string> &v, string s, string del)
{
    int start = 0;
    int end = s.find(del);

    while (end != -1)
    {
        v.push_back(s.substr(start, end - start));
        start = end + del.size();
        end = s.find(del, start);
    }
    if (s.substr(start, end - start).length() >= 3)
        v.push_back(s.substr(start, end - start));
}
double sigmoid(double d)
{
    double ans = 1 / (double)(1 + exp(-d));

    return ans;
}

/*read minibatch from shared file and calculate gradient descent*/
void *update(void *threadid)
{
    double w[features], b;
    double Y[batch];
    double Y1[batch];

    bool flag = true;
    vector<string> v;
    string s;
    while (true)
    {
        string *buffer = new string[batch];

        double X[features][batch] = {0};
        double p[batch] = {0};
        double gradient_w[batch] = {0};
        double gradient_b = 0;
        long thid = (long)threadid;
        double rate = 0;
        pthread_mutex_lock(&mutex_counter);
        Node *temp = new Node();
        temp->id = thid;
        temp->next = NULL;
        if (head1 == NULL)
        {
            head1 = temp;
            tail1 = head1;
        }
        else
        {
            tail1->next = temp;
            tail1 = temp;
        }
        pthread_cond_signal(&counter_threshold);
        pthread_mutex_unlock(&mutex_counter);

        pthread_mutex_lock(&(permission[thid]));

        while (permit[thid] == 0)
        {
            pthread_cond_wait(&(permission_check[thid]), &(permission[thid])); // wait for main thread to give permission
        }
        if (permit[thid] == -1)
            flag = false;
        else
            permit[thid] = 0;
        pthread_mutex_unlock(&(permission[thid]));

        if (!flag)
        {
            cout << "Exiting from Update Thread: " << (long)threadid << endl;
            break;
        }

        pthread_mutex_lock(&learning_mutex);
        rate = learning_rate;
        pthread_mutex_unlock(&learning_mutex);
        int cur = batch + 1;
        /*read the file by taking access of shared file handle*/
        pthread_mutex_lock(&file_handle);

        for (int i = 1; i <= batch; i++)
        {
            if (!getline(input_file, s))
            {
                cur = i;
                input_file.clear();
                input_file.seekg(0);
            }
            else
            {
                buffer[i - 1] = s;
            }
        }
        if (cur != batch + 1)
        {
            for (int j = cur; j <= batch; j++)
            {
                getline(input_file, s);
                buffer[j - 1] = s;
                input_file.clear();
                input_file.seekg(0);
            }
        }
        pthread_mutex_unlock(&file_handle);

        pthread_mutex_lock(&WB);
        for (int i = 0; i < features; i++)
        {
            w[i] = W[i];
        }
        b = B;
        pthread_mutex_unlock(&WB);

        for (int i = 0; i < batch; i++)
        {
            v.clear();
            tokenize(v, buffer[i], " ");
            if (v[0] == "1")
            {
                Y[i] = 1;
            }
            else
            {
                Y[i] = 0;
            }

            for (int j = 1; j < v.size(); j++)
            {
                int start = 0;
                string del = ":";
                int end = v[j].find(del);
                int index = stoi(v[j].substr(start, end - start));
                start = end + del.size();
                end = v[j].find(del, start);
                double val = stoi(v[j].substr(start, end - start));
                X[index - 1][i] = val;
            }
        }

        for (int k = 0; k < batch; k++)
        {
            for (int j = 0; j < features; j++)
            {
                p[k] += (X[j][k] * w[j]);
            }
            p[k] += b;
            Y1[k] = 1 / (double)(1 + exp(-p[k]));
            for (int r = 0; r < features; r++)
            {
                gradient_w[r] += ((Y1[k] - Y[k]) * X[r][k]);
            }
            gradient_b += (Y1[k] - Y[k]);
        }

        pthread_mutex_lock(&WB);
        for (int i = 0; i < features; i++)
        {
            W[i] = W[i] - rate * (gradient_w[i] / (double)batch);
        }
        B = B - rate * (gradient_b / (double)batch);
        pthread_mutex_unlock(&WB);
        delete[] buffer;
    }
    pthread_exit(NULL);
}
/*initalisation of mutex and condition variable*/
void init()
{
    pthread_mutex_init(&mutex_counter, NULL);
    pthread_mutex_init(&file_handle, NULL);
    pthread_mutex_init(&learning_mutex, NULL);
    pthread_mutex_init(&WB, NULL);
    pthread_cond_init(&counter_threshold, NULL);

    for (int i = 0; i < Nthreads; i++)
    {
        pthread_mutex_init(&permission[i], NULL);
        pthread_cond_init(&permission_check[i], NULL);
    }
    srand(121);
    for (int i = 0; i < features; i++)
    {
        double q = ((double)rand() / (RAND_MAX));
        W[i] = q; //random initialsation of weights
    }
    B = 0;

    input_file.open(file);
}

int main(int argc, char *argv[])
{
    int epoch = 1;
    file = argv[1];
    init();
    int total_iterations, update_threads;

    cout << "Enter Number of iteration: " << endl;

    cin >> total_iterations;
    cout << "Enter Number of Threads: " << endl;
    cin >> update_threads;
    pthread_t threads[update_threads], datathread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    double eta = 0.1;
    time_t t1 = time(0);
    /*create a thread as per user input*/
    for (int i = 0; i < update_threads; i++)
    {
        long g = i;
        pthread_create(&threads[i], &attr, update, (void *)g);
    }
    int total_batch = ceil(records / (double)batch);
    int count = 1;

    /*perform minibatch gradient descent in iterations*/
    while (epoch != (total_iterations + 1))
    {
        if (count == total_batch + 1)
        {
            cout << "Iteratiion: " << epoch << endl;

            pthread_mutex_lock(&learning_mutex);
            learning_rate = eta / (double)(sqrt(epoch + 1));
            pthread_mutex_unlock(&learning_mutex);
            count = 1;
            epoch++;
        }

        pthread_mutex_lock(&mutex_counter);
        while (head1 == NULL)
        {
            pthread_cond_wait(&counter_threshold, &mutex_counter);
        }
        Node *top = head1;
        int id = top->id;
        head1 = head1->next;
        free(top);

        pthread_mutex_lock(&permission[id]);
        permit[id] = 1;
        count++;
        pthread_cond_signal(&(permission_check[id]));
        pthread_mutex_unlock(&permission[id]);

        pthread_mutex_unlock(&mutex_counter);
    }
    for (int i = 0; i < update_threads; i++)
    {
        pthread_mutex_lock(&permission[i]);
        permit[i] = -1;
        pthread_cond_signal(&(permission_check[i]));
        pthread_mutex_unlock(&permission[i]);
    }

    //waits for thread to exit
    for (int i = 0; i < update_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    cout << "-----------------------------------" << endl;
    cout << "coefficents of W:" << endl;
    for (int i = 0; i < features; i++)
    {
        printf("W[%d]:%lf     ", i + 1, W[i]);
    }

    cout << endl;
    cout << endl;
    cout << "B: " << B << endl;
    input_file.clear();
    input_file.seekg(0);
    string s;
    vector<string> v;
    int i = 0;

    while (getline(input_file, s))
    {
        v.clear();
        tokenize(v, s, " ");
        if (v[0] == "1")
        {
            P[i] = 1;
        }
        else
        {
            P[i] = 0;
        }

        for (int j = 1; j < v.size(); j++)
        {
            int start = 0;
            string del = ":";
            int end = v[j].find(del);
            int index = stoi(v[j].substr(start, end - start));
            start = end + del.size();
            end = v[j].find(del, start);
            double val = stoi(v[j].substr(start, end - start));
            entire_data[index - 1][i] = val;
        }

        i++;
    }
    double loss = 0;
    int pos = 0;
    int tp = 0;
    int tq = 0;
    //read the file to calculate accuracy over entire dataset
    for (int k = 0; k < records; k++)
    {
        for (int j = 0; j < features; j++)
        {
            D[k] += (entire_data[j][k] * W[j]);
        }
        D[k] += B;
        Q[k] = 1 / (double)(1 + exp(-D[k]));
        if (P[k] == 0)
            tp++;
        else
            tq++;
        if (P[k] == 1 && Q[k] > 0.5)
            pos++;
        else if (P[k] == 0 && Q[k] <= 0.5)
            pos++;
        else
            loss += (P[k] * log(Q[k]) + (1 - P[k]) * log(1 - Q[k]));
    }
    time_t t2 = time(0);

    cout << "Total time taken(training + predection): " << (t2 - t1) << endl;
    double acc = ((pos / (double)records) * 100.00);
    cout << "Accuracy(#correct classification/(#total classification)): " << acc << "%" << endl;
    return 0;
}
