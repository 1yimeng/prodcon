#include "threads.h"
#include "tands.h"

using namespace std;

struct arg_struct {
    int n;
};

sem_t empty;
sem_t full;
queue<arg_struct*> buffer;
pthread_mutex_t mutexBuffer;

bool run_command() {
    if 
}
void* producer(void* args) {
    // string line;
    while (true) {
        // wait until there is more than 0 empty slots so can fill it up
        sem_wait(&empty);

        // locking to make sure no concurrent access of the buffer 
        pthread_mutex_lock(&mutexBuffer);

        // char num = line[1];  num-'0'
        struct arg_struct args = {2};

        int in_line = buffer.size(); // waiting jobs 
        cout << "push " << args.n << " and size is " << buffer.size() << endl;

        // pass through n
        buffer.push(&args);

        pthread_mutex_unlock(&mutexBuffer);
        // increment the items 
        sem_post(&full);
    }

}

void* consumer(void* args) {
    while (true) {
        // wait until items is more than 0 so can take from buffer
        sem_wait(&full); 
        pthread_mutex_lock(&mutexBuffer);

        // pass through n
        void* argv = buffer.front();
        struct arg_struct *args = (struct arg_struct *) argv;
        cout << "read in " << args->n << endl;
        // Trans(n);
        buffer.pop();

        pthread_mutex_unlock(&mutexBuffer);
        // increment the empty variable since has taken one from buffer
        sem_post(&empty);
    }
}



void start_process(int thread_num, int id) {
    pthread_t th[thread_num+1];
    struct arg_struct args;
    sem_init(&empty, 0, thread_num*2);
    sem_init(&full, 0, 0);

    for (int i = 0; i < thread_num+1; i++) {

        if (i == thread_num) {
            // 1 producer, (void *)&args
            if (pthread_create(&th[thread_num], NULL, &producer, NULL) != 0) {
                perror("Failed to create thread");
            }
        } else {
            // thread_num amount of consumers
            if (pthread_create(&th[i], NULL, &consumer, NULL) != 0) {
                perror("Failed to create thread");
            }
        }
    }

    for (int i = 0; i < thread_num+1; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }

    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutexBuffer);
}

