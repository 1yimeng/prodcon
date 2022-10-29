#include "threads.h"
#include "tands.h"

using namespace std;

struct arg_struct {
    int n;
};

sem_t empty;
sem_t full;
queue<arg_struct> buffer;
pthread_mutex_t mutexBuffer;
int get_command(int thread_num) {
    string line;
    while (getline(cin, line) && !line.empty()) {
        int num = line[1]-'0';
        producer(num);
        // if (line[0] == 'T') {
        //     producer();
        // } else if (line[0] == 'S') {
        //     int num = line[1]-'0';
        //     cout << "sleep for " << num << endl;
        //     Sleep(num);
        continue;
    }

}


void producer(int thread_num) {
    string line;
    while (getline(cin, line) && !line.empty()) {
        // wait until there is more than 0 empty slots so can fill it up
        sem_wait(&empty);

        // locking to make sure no concurrent access of the buffer 
        pthread_mutex_lock(&mutexBuffer);
        
        int num = line[1]-'0';
        struct arg_struct args = {num-'0'};

        // pass through n
        buffer.push(args);
        int in_line = buffer.size(); // waiting jobs 
        cout << "push " << args.n << " and size is " << buffer.size() << endl;

        pthread_mutex_unlock(&mutexBuffer);
        // increment the items 
        sem_post(&full);

        cout << line << endl;
    }

    for (int i = 0; i < thread_num; i++) {
        // wait until there is more than 0 empty slots so can fill it up
        sem_wait(&empty);
        pthread_mutex_lock(&mutexBuffer);

        struct arg_struct args = {-1};
        buffer.push(args);
        cout << "push " << args.n;

        pthread_mutex_unlock(&mutexBuffer);
        sem_post(&full);
    }

}

void* consumer(void* args) {
    while (true) {
        //wait until items is more than 0 so can take from buffer
        sem_wait(&full); 
        pthread_mutex_lock(&mutexBuffer);

        // pass through n
        struct arg_struct argv = buffer.front();
        cout << "read in " << argv.n << endl;
        // Trans(n);
        buffer.pop();
        if (argv.n == -1) {
            pthread_mutex_unlock(&mutexBuffer);
            break;
        }
        pthread_mutex_unlock(&mutexBuffer);
        // increment the empty variable since has taken one from buffer
        sem_post(&empty);
    }
    return nullptr;
}

void start_process(int thread_num, int id) {
    pthread_t th[thread_num];
    struct arg_struct args;
    sem_init(&empty, 0, thread_num*2);
    sem_init(&full, 0, 0);

    for (int i = 0; i < thread_num; i++) {
        // thread_num amount of consumers
        if (pthread_create(&th[i], NULL, &consumer, NULL) != 0) {
            perror("Failed to create thread");
        }
    }

    producer(thread_num);

    for (int i = 0; i < thread_num; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }

    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutexBuffer);
}

