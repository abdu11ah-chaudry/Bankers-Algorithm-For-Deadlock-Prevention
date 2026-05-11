#ifndef BANKER_H
#define BANKER_H

#define MAX_PROCESSES 20
#define MAX_RESOURCES 10

typedef struct {
    int valid;

    int allocation[MAX_PROCESSES][MAX_RESOURCES];
    int maximum[MAX_PROCESSES][MAX_RESOURCES];
    int need[MAX_PROCESSES][MAX_RESOURCES];
    int available[MAX_RESOURCES];

} Snapshot;

extern int n_procs;
extern int n_res;

extern int allocation[MAX_PROCESSES][MAX_RESOURCES];
extern int maximum[MAX_PROCESSES][MAX_RESOURCES];
extern int need[MAX_PROCESSES][MAX_RESOURCES];
extern int available[MAX_RESOURCES];

extern Snapshot snapshot;

/* Core functions */
void compute_need();
int safety_algorithm(int safe_seq[], int print_steps);

void resource_request();
void release_resources();

void save_snapshot();
void restore_snapshot();

void random_stress_test();
void performance_analysis();

void init_from_input();
void init_demo();

#endif
