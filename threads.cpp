#include "threads.h"
#include "tands.h"

using namespace std;
using namespace std::chrono;

struct id_complete {
    int id;
    int complete;
};

sem_t empty;
sem_t full;
queue<int> buffer;
pthread_mutex_t mutexBuffer;
pthread_mutex_t mutexFile;
pthread_mutex_t mutexSize;
FILE* pFile;
high_resolution_clock::time_point start_time;
int work = 0;
int ask = 0;
int receive = 0;
int complete = 0;
int sleep = 0; 

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

double get_time() {
    high_resolution_clock::time_point current = high_resolution_clock().now();
    return (double) duration_cast<milliseconds> (current - start_time).count() / (double) 1000;
}

int get_size() {
    int qsize;
    pthread_mutex_lock(&mutexSize);
    qsize = (int) buffer.size();
    pthread_mutex_unlock(&mutexSize);
    return qsize;
}

void log_to_file(const char* command, int id, int arg_num) {
    double time_passed = get_time();

    char q[] = "    ";
    if (command == "Work" || command == "Receive") {
        sprintf(q,"Q= %s", to_string(buffer.size()).c_str());
    }

    char arg[] = "  ";
    if (arg_num != -1) {
        sprintf(arg, "%s", to_string(arg_num).c_str());
    } 

    // lock to write to files to prevent concurrent access 
    pthread_mutex_lock(&mutexFile);
    fprintf(pFile, "%5.3f ID=%3u %8s %-10s %3s\n", time_passed, id, q, command, arg);
    pthread_mutex_unlock(&mutexFile);
}

void producer(int content) {
    // wait until there is more than 0 empty slots so can fill it up
    sem_wait(&empty);

    // locking to make sure no concurrent access of the buffer 
    pthread_mutex_lock(&mutexBuffer);

    // pass through n
    buffer.push(content);
    log_to_file("Work", 0, content);
    work++; 

    pthread_mutex_unlock(&mutexBuffer);
    // increment the items 
    sem_post(&full);
}

void closing(int thread_num) {
    // send -1 for each thread to end consumers from reading
    for (int i = 0; i < thread_num; i++) {

        sem_wait(&empty);
        pthread_mutex_lock(&mutexBuffer);

        buffer.push(-1);

        pthread_mutex_unlock(&mutexBuffer);
        sem_post(&full);
    }
}

void log_summary(int thread_num, struct id_complete all_ids[]) {
    fprintf(pFile, "%s\n", "Summary:");
    fprintf(pFile, "%8s\t%3u\n", "Work", work);
    fprintf(pFile, "%7s\t\t%3u\n", "Ask", ask);
    fprintf(pFile, "%11s\t%3u\n", "Receive", receive);
    fprintf(pFile, "%12s\t%3u\n", "Complete", complete);
    fprintf(pFile, "%9s\t%3u\n", "Sleep", sleep);

    for (int i = 0; i < thread_num; i++) {
        fprintf(pFile, "%10s  %2u\t%3u\n", "Thread", i+1, all_ids[i].complete);
    }

    double avg = (double) complete / get_time();
    fprintf(pFile, "%15s\t%4.2f\n", "Transactions per second: ", avg);
}

void get_command(int thread_num) {
    string line;
    while (getline(cin, line)) {
        int num = line[1]-'0';
        if (line[0] == 'T') {
            producer(num);
        } else if (line[0] == 'S') {
            log_to_file("Sleep", 0, num);
            sleep++;
            Sleep(num);
        }
    }
    log_to_file("End", 0, -1);
    closing(thread_num);
}

void* consumer(void* args) {
    while (true) {
        struct id_complete* info = (struct id_complete *) args;
        log_to_file("Ask", info->id, -1);
        ask++; 

        //wait until items is more than 0 so can take from buffer
        sem_wait(&full); 
        pthread_mutex_lock(&mutexBuffer);

        // pass through n
        int n = buffer.front();
        buffer.pop();

        if (n == -1) {
            pthread_mutex_unlock(&mutexBuffer);
            break;
        }

        pthread_mutex_unlock(&mutexBuffer);

        // increment the empty variable since has taken one from buffer
        sem_post(&empty);

        log_to_file("Receive", info->id, n);
        receive++;
        Trans(n);
        log_to_file("Complete", info->id, n);
        complete++;
        info->complete += 1;
    }
    return nullptr;
}

void start_process(int thread_num, int id) {
    start_time = high_resolution_clock().now();
    pthread_t th[thread_num];
    sem_init(&empty, 0, thread_num*2);
    sem_init(&full, 0, 0);
    open_file(id);

    // saving ids to reference 
    struct id_complete all_ids[thread_num];
    for (int i = 0; i < thread_num; i++) {
        all_ids[i].id = i+1;
        all_ids[i].complete = 0;
    }

    for (int i = 0; i < thread_num; i++) {
        // thread_num amount of consumers
        if (pthread_create(&th[i], NULL, &consumer, &all_ids[i]) != 0) {
            perror("Failed to create thread");
        }
    }

    get_command(thread_num);

    for (int i = 0; i < thread_num; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }

    log_summary(thread_num, all_ids);

    fclose(pFile);
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutexBuffer);
    pthread_mutex_destroy(&mutexFile);
    pthread_mutex_destroy(&mutexSize);
}

