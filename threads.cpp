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
pthread_mutex_t mutexFile;
FILE* pFile;

void open_file(int id) {
    string file_name;
    if (id == 0) {
        file_name = "prodcon.log";
    } else {
        file_name = "prodcon." + to_string(id) + ".log";
    }
    const char* cfile_name = file_name.c_str();
    pFile = fopen(cfile_name, "w");
}

void log_to_file(const char* command, int num) {
    pthread_mutex_lock(&mutexFile);
    fprintf(pFile, "%5.3f ID=%3u %5s %-10s %3u\n", 0.000, 1, "Q= 4",command,num);
    pthread_mutex_unlock(&mutexFile);
}

void producer(int content) {
    // wait until there is more than 0 empty slots so can fill it up
    sem_wait(&empty);

    // locking to make sure no concurrent access of the buffer 
    pthread_mutex_lock(&mutexBuffer);
    
    struct arg_struct args = {content};

    // pass through n
    buffer.push(args);
    int in_line = buffer.size(); // waiting jobs 
    // cout << "push " << args.n << " and size is " << buffer.size() << endl;
    // string action = "push " + args.n;
    // cout << action << endl;
    // log_to_file(action);

    pthread_mutex_unlock(&mutexBuffer);
    // increment the items 
    sem_post(&full);
}

void closing(int thread_num) {
    // send -1 for each thread to end consumers from reading
    for (int i = 0; i < thread_num; i++) {

        sem_wait(&empty);
        pthread_mutex_lock(&mutexBuffer);

        struct arg_struct args = {-1};
        buffer.push(args);
        cout << "push " << args.n;

        pthread_mutex_unlock(&mutexBuffer);
        sem_post(&full);
    }
}

void get_command(int thread_num) {
    string line;
    while (getline(cin, line) && !line.empty()) {
        int num = line[1]-'0';
        if (line[0] == 'T') {
            producer(num);
        } else if (line[0] == 'S') {
            // cout << "sleep for " << num << endl;
            Sleep(num);
        }
    }
    closing(thread_num);
}

void* consumer(void* args) {
    while (true) {
        //wait until items is more than 0 so can take from buffer
        sem_wait(&full); 
        pthread_mutex_lock(&mutexBuffer);

        // pass through n
        struct arg_struct argv = buffer.front();
        const char* action = "read haha";
        // Trans(n);
        buffer.pop();
        if (argv.n == -1) {
            pthread_mutex_unlock(&mutexBuffer);
            break;
        }
        log_to_file(action, argv.n);
        cout << "read in " << argv.n << endl;
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
    open_file(id);

    for (int i = 0; i < thread_num; i++) {
        // thread_num amount of consumers
        if (pthread_create(&th[i], NULL, &consumer, NULL) != 0) {
            perror("Failed to create thread");
        }
    }

    get_command(thread_num);

    for (int i = 0; i < thread_num; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }

    fclose(pFile);
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutexBuffer);
    pthread_mutex_destroy(&mutexFile);
}

