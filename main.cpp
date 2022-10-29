#include <iostream>
#include "threads.h"

using namespace std;

int main(int argc, char **argv) {
    int thread_num = 0;
    int id;
    if (argc == 2) {
        thread_num = atoi(argv[1]);
        // cout << thread_num << " " << id << endl;
    } else if (argc == 3) {
        thread_num = atoi(argv[1]);
        id = atoi(argv[2]);
        // cout << thread_num << " " << id << endl;
    } else {
        cout << "Wrong number of arguments, use 2 or 1 arguments to run the program" << endl;
        exit(0);
    }
    start_process(thread_num, id);
    return 0;
}