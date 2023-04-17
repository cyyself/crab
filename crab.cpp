#include <cstdio>
#include <random>
#include <sys/mman.h>
#include <sched.h>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <chrono>
#include <thread>
#include <unistd.h>

const int nr_max_thread = 32;
const int iter_size = 1e4;
const size_t map_size = 1024l*1024l*1024l;

long long statistic[nr_max_thread];

void pin_one_cpu(int cpu) {
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(cpu, &cpu_set);
    assert(sched_setaffinity(0, sizeof(cpu_set), &cpu_set) == 0);
}

void work_thread(int current_thread) {
    pin_one_cpu(current_thread);
    std::mt19937 rng;
    volatile char *mem = (char *)mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
    if (mem == MAP_FAILED) {
        printf("map failed!\n");
        exit(1);
    }
    while (true) {
        auto ts_start = std::chrono::steady_clock::now();
        for (int i=0;i<iter_size;i++) {
            mem[rng()%map_size];
        }
        auto ts_end = std::chrono::steady_clock::now();
        statistic[current_thread] += iter_size;
        std::chrono::nanoseconds time = ts_end - ts_start;
        double iops = (double)iter_size / ((double)time.count()/1000.0/1000.0/1000.0);
    }
}

int main() {
    std::thread **t = new std::thread*[nr_max_thread];

    for (int i=0;i<nr_max_thread;i++) {
        t[i] = new std::thread(work_thread, i);
    }
    long long last_step = 0;
    while (true) {
        sleep(1);
        long long cur_step = 0;
        for (int i=0;i<nr_max_thread;i++) cur_step += statistic[i];
        long long delta_step = cur_step - last_step;
        last_step = cur_step;
        printf("iops=%lld\n", delta_step);
    }
    return 0;
}