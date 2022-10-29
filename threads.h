#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <queue>
#include <iostream>
#include <cstdio>
#include <chrono>
#include <vector>

#define FORMAT "%5.3f ID=%2u %4s %-10s %5s\n"

void start_process(int thread_num, int id);