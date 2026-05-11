#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define MAX_PROCESSES 20
#define MAX_RESOURCES 10
#define LOG_FILE      "banker_log.txt"


static int n_procs = 0;          /* number of processes */
static int n_res   = 0;          /* number of resource types */

static int allocation[MAX_PROCESSES][MAX_RESOURCES];
static int maximum  [MAX_PROCESSES][MAX_RESOURCES];
static int need     [MAX_PROCESSES][MAX_RESOURCES];
static int available[MAX_RESOURCES];

/* Snapshot (rollback) */
typedef struct {
    int valid;
    int allocation[MAX_PROCESSES][MAX_RESOURCES];
    int maximum   [MAX_PROCESSES][MAX_RESOURCES];
    int need      [MAX_PROCESSES][MAX_RESOURCES];
    int available [MAX_RESOURCES];
} Snapshot;

static Snapshot snapshot;  

/* Logging */
static FILE *log_fp = NULL;

static void log_open(void) {
    log_fp = fopen(LOG_FILE, "a");
    if (!log_fp) {
        fprintf(stderr, "[WARN] Cannot open log file %s\n", LOG_FILE);
    }
}

static void log_close(void) {
    if (log_fp) { fclose(log_fp); log_fp = NULL; }
}

static void log_write(const char *msg) {
    if (!log_fp) return;
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    fprintf(log_fp, "[%s] %s\n", buf, msg);
    fflush(log_fp);
}


static void compute_need(void) {
    for (int i = 0; i < n_procs; i++)
        for (int j = 0; j < n_res; j++)
            need[i][j] = maximum[i][j] - allocation[i][j];
}

static void print_state(void) {
    printf("\n========== SYSTEM STATE ==========\n");

    /* --- Available --- */
    printf("Available:  ");
    for (int j = 0; j < n_res; j++) printf("%4d", available[j]);
    printf("\n\n");

    /* --- Header row --- */
    printf("%-6s", "PID");
    printf("  Allocation");
    for (int j = 1; j < n_res; j++) printf("    ");
    printf("  Maximum");
    for (int j = 1; j < n_res; j++) printf("    ");
    printf("  Need\n");

    for (int i = 0; i < n_procs; i++) {
        printf("P%-5d", i);
        for (int j = 0; j < n_res; j++) printf("%4d", allocation[i][j]);
        printf("  |");
        for (int j = 0; j < n_res; j++) printf("%4d", maximum[i][j]);
        printf("  |");
        for (int j = 0; j < n_res; j++) printf("%4d", need[i][j]);
        printf("\n");
    }
    printf("==================================\n");
}

static int safety_algorithm(int safe_seq[], int print_steps) {
    int work  [MAX_RESOURCES];
    int finish[MAX_PROCESSES];

    memcpy(work, available, n_res * sizeof(int));
    memset(finish, 0, n_procs * sizeof(int));

    int count = 0;

    while (count < n_procs) {
        int found = 0;
        for (int i = 0; i < n_procs; i++) {
            if (finish[i]) continue;

            /* Check if Need[i] <= Work */
            int ok = 1;
            for (int j = 0; j < n_res; j++) {
                if (need[i][j] > work[j]) { ok = 0; break; }
            }

            if (ok) {
                /* Simulate process i finishing */
                for (int j = 0; j < n_res; j++)
                    work[j] += allocation[i][j];
                finish[i] = 1;
                safe_seq[count++] = i;
                found = 1;

                if (print_steps) {
                    printf("  Step %d: P%d can finish. Work -> ", count, i);
                    for (int j = 0; j < n_res; j++) printf("%d ", work[j]);
                    printf("\n");
                }
            }
        }
        if (!found) break;   /* no progress → unsafe */
    }

    return (count == n_procs);
}

/*  Resource-Request Algorithm */
static void resource_request(void) {
    int pid;
    int request[MAX_RESOURCES];

    printf("Enter Process ID (0 to %d): ", n_procs - 1);
    if (scanf("%d", &pid) != 1 || pid < 0 || pid >= n_procs) {
        printf("[ERROR] Invalid process ID.\n");
        return;
    }

    printf("Enter request for %d resource(s): ", n_res);
    for (int j = 0; j < n_res; j++) {
        if (scanf("%d", &request[j]) != 1) { printf("[ERROR] Bad input.\n"); return; }
    }

    /* Step 1: Request <= Need? */
    for (int j = 0; j < n_res; j++) {
        if (request[j] > need[pid][j]) {
            printf("[DENIED] Request exceeds declared maximum need.\n");
            log_write("REQUEST DENIED: exceeds need.");
            return;
        }
    }

    /* Step 2: Request <= Available? */
    for (int j = 0; j < n_res; j++) {
        if (request[j] > available[j]) {
            printf("[WAIT] Resources not available. Process P%d must wait.\n", pid);
            log_write("REQUEST WAIT: insufficient available resources.");
            return;
        }
    }

    /* Step 3: Tentative allocation */
    for (int j = 0; j < n_res; j++) {
        available  [j]       -= request[j];
        allocation [pid][j]  += request[j];
        need       [pid][j]  -= request[j];
    }

    /* Step 4: Safety check */
    int safe_seq[MAX_PROCESSES];
    int safe = safety_algorithm(safe_seq, 0);

    if (safe) {
        printf("[GRANTED] Request by P%d is safe.\n", pid);
        printf("Safe sequence: ");
        for (int k = 0; k < n_procs; k++) printf("P%d ", safe_seq[k]);
        printf("\n");
        log_write("REQUEST GRANTED: system remains safe.");
        print_state();
    } else {
        /* Roll back tentative allocation */
        for (int j = 0; j < n_res; j++) {
            available  [j]       += request[j];
            allocation [pid][j]  -= request[j];
            need       [pid][j]  += request[j];
        }
        printf("[DENIED] Granting this request would lead to an UNSAFE state.\n");
        printf("Request rolled back. System state unchanged.\n");
        log_write("REQUEST DENIED: would cause unsafe state (rolled back).");
    }
}

/* Release resources */
static void release_resources(void) {
    int pid;
    int release[MAX_RESOURCES];

    printf("Enter Process ID (0 to %d): ", n_procs - 1);
    if (scanf("%d", &pid) != 1 || pid < 0 || pid >= n_procs) {
        printf("[ERROR] Invalid process ID.\n");
        return;
    }

    printf("Enter resources to release (%d values): ", n_res);
    for (int j = 0; j < n_res; j++) {
        if (scanf("%d", &release[j]) != 1) { printf("[ERROR] Bad input.\n"); return; }
        if (release[j] > allocation[pid][j]) {
            printf("[ERROR] Cannot release more than allocated.\n");
            return;
        }
    }

    for (int j = 0; j < n_res; j++) {
        available  [j]       += release[j];
        allocation [pid][j]  -= release[j];
        need       [pid][j]  += release[j];
    }

    printf("[OK] Resources released by P%d.\n", pid);
    log_write("Resources released.");
    print_state();
}

/* Save / restore snapshot (rollback) */
static void save_snapshot(void) {
    snapshot.valid = 1;
    memcpy(snapshot.allocation, allocation, sizeof(allocation));
    memcpy(snapshot.maximum,    maximum,    sizeof(maximum));
    memcpy(snapshot.need,       need,       sizeof(need));
    memcpy(snapshot.available,  available,  sizeof(available));
    printf("[SNAPSHOT] Current state saved.\n");
    log_write("SNAPSHOT saved.");
}

static void restore_snapshot(void) {
    if (!snapshot.valid) {
        printf("[ERROR] No snapshot available.\n");
        return;
    }
    memcpy(allocation, snapshot.allocation, sizeof(allocation));
    memcpy(maximum,    snapshot.maximum,    sizeof(maximum));
    memcpy(need,       snapshot.need,       sizeof(need));
    memcpy(available,  snapshot.available,  sizeof(available));
    printf("[ROLLBACK] State restored from snapshot.\n");
    log_write("ROLLBACK: state restored from snapshot.");
    print_state();
}

/* Randomized stress-test */
static void random_stress_test(void) {
    int rounds;
    printf("Enter number of random requests to simulate: ");
    if (scanf("%d", &rounds) != 1 || rounds <= 0) {
        printf("[ERROR] Invalid input.\n");
        return;
    }

    srand((unsigned)time(NULL));

    int granted = 0, denied = 0, waited = 0;

    for (int r = 0; r < rounds; r++) {
        int pid = rand() % n_procs;
        int request[MAX_RESOURCES];
        int valid = 1;

        for (int j = 0; j < n_res; j++) {
            int max_req = need[pid][j];
            request[j] = (max_req > 0) ? (rand() % (max_req + 1)) : 0;
            if (request[j] > available[j]) { valid = 0; }
        }

        if (!valid) { waited++; continue; }

        /* Tentative */
        for (int j = 0; j < n_res; j++) {
            available [j]       -= request[j];
            allocation[pid][j]  += request[j];
            need      [pid][j]  -= request[j];
        }

        int safe_seq[MAX_PROCESSES];
        if (safety_algorithm(safe_seq, 0)) {
            granted++;
        } else {
            /* Roll back */
            for (int j = 0; j < n_res; j++) {
                available [j]       += request[j];
                allocation[pid][j]  -= request[j];
                need      [pid][j]  += request[j];
            }
            denied++;
        }
    }

    printf("\n--- Stress-Test Results (%d rounds) ---\n", rounds);
    printf("  Granted : %d\n", granted);
    printf("  Denied  : %d\n", denied);
    printf("  Waited  : %d\n", waited);
    log_write("Randomized stress test completed.");
}

/* Performance analysis */
static void performance_analysis(void) {
    int iterations;
    printf("Enter number of Safety Algorithm iterations to benchmark: ");
    if (scanf("%d", &iterations) != 1 || iterations <= 0) {
        printf("[ERROR] Invalid input.\n");
        return;
    }

    int safe_seq[MAX_PROCESSES];
    clock_t start = clock();

    for (int i = 0; i < iterations; i++)
        safety_algorithm(safe_seq, 0);

    clock_t end = clock();
    double elapsed_ms = 1000.0 * (end - start) / CLOCKS_PER_SEC;

    printf("\n--- Performance Analysis ---\n");
    printf("  Processes      : %d\n", n_procs);
    printf("  Resource types : %d\n", n_res);
    printf("  Iterations     : %d\n", iterations);
    printf("  Total time     : %.3f ms\n", elapsed_ms);
    printf("  Avg per call   : %.6f ms\n", elapsed_ms / iterations);
    log_write("Performance analysis completed.");
}

static void init_from_input(void) {
    printf("Enter number of processes (max %d): ", MAX_PROCESSES);
    scanf("%d", &n_procs);
    if (n_procs < 1 || n_procs > MAX_PROCESSES) {
        printf("[ERROR] Out of range.\n"); exit(1);
    }

    printf("Enter number of resource types (max %d): ", MAX_RESOURCES);
    scanf("%d", &n_res);
    if (n_res < 1 || n_res > MAX_RESOURCES) {
        printf("[ERROR] Out of range.\n"); exit(1);
    }

    printf("\nEnter Available resources (%d values): ", n_res);
    for (int j = 0; j < n_res; j++) scanf("%d", &available[j]);

    printf("\nEnter Allocation matrix (%d x %d):\n", n_procs, n_res);
    for (int i = 0; i < n_procs; i++) {
        printf("  P%d: ", i);
        for (int j = 0; j < n_res; j++) scanf("%d", &allocation[i][j]);
    }

    printf("\nEnter Maximum matrix (%d x %d):\n", n_procs, n_res);
    for (int i = 0; i < n_procs; i++) {
        printf("  P%d: ", i);
        for (int j = 0; j < n_res; j++) scanf("%d", &maximum[i][j]);
    }

    compute_need();
    snapshot.valid = 0;
    log_write("System initialized via interactive input.");
}

static void init_demo(void) {
    /*  Classic textbook example (Silberschatz OS Book)
     *  3 resource types (A, B, C), 5 processes (P0-P4)
     *  Total: A=10  B=5  C=7
     */
    n_procs = 5; n_res = 3;

    int alloc[5][3] = {{0,1,0},{2,0,0},{3,0,2},{2,1,1},{0,0,2}};
    int maxim[5][3] = {{7,5,3},{3,2,2},{9,0,2},{2,2,2},{4,3,3}};
    int avail[3]    =  {3,3,2};

    for (int i = 0; i < n_procs; i++)
        for (int j = 0; j < n_res; j++) {
            allocation[i][j] = alloc[i][j];
            maximum[i][j]    = maxim[i][j];
        }
    for (int j = 0; j < n_res; j++) available[j] = avail[j];

    compute_need();
    snapshot.valid = 0;
    log_write("System initialized with built-in demo.");
    printf("[DEMO] Loaded classic textbook example (5 processes, 3 resource types).\n");
}

/* Main menu */
static void print_menu(void) {
    printf("\n========== BANKER'S ALGORITHM SIMULATOR ==========\n");
    printf("  1. Display current system state\n");
    printf("  2. Run Safety Algorithm\n");
    printf("  3. Submit resource request\n");
    printf("  4. Release resources\n");
    printf("  5. Save snapshot\n");
    printf("  6. Restore snapshot (rollback)\n");
    printf("  7. Randomized stress test\n");
    printf("  8. Performance analysis\n");
    printf("  0. Exit\n");
    printf("Choice: ");
}

int main(void) {
    log_open();
    printf("============================================\n");
    printf("  Banker's Algorithm Simulator\n");
    printf("  OS Project — Group 24k-0513/0725/0936\n");
    printf("============================================\n\n");

    printf("Initialize system:\n");
    printf("  1. Enter data manually\n");
    printf("  2. Load built-in demo\n");
    printf("Choice: ");
    int choice;
    if (scanf("%d", &choice) != 1) choice = 2;

    if (choice == 1) init_from_input();
    else             init_demo();

    print_state();

    /* Check initial safety */
    int safe_seq[MAX_PROCESSES];
    if (safety_algorithm(safe_seq, 0)) {
        printf("\n[SAFE] Initial system state is SAFE.\n");
        printf("Safe sequence: ");
        for (int k = 0; k < n_procs; k++) printf("P%d ", safe_seq[k]);
        printf("\n");
        log_write("Initial state: SAFE.");
    } else {
        printf("\n[UNSAFE] Initial system state is UNSAFE!\n");
        log_write("Initial state: UNSAFE.");
    }

    /* Main interactive loop */
    int running = 1;
    while (running) {
        print_menu();
        if (scanf("%d", &choice) != 1) { choice = -1; }

        switch (choice) {
            case 1:
                print_state();
                break;

            case 2: {
                int ss[MAX_PROCESSES];
                printf("\n-- Safety Algorithm (step-by-step) --\n");
                if (safety_algorithm(ss, 1)) {
                    printf("[SAFE] Safe sequence: ");
                    for (int k = 0; k < n_procs; k++) printf("P%d ", ss[k]);
                    printf("\n");
                    log_write("Safety check: SAFE.");
                } else {
                    printf("[UNSAFE] No safe sequence exists!\n");
                    log_write("Safety check: UNSAFE.");
                }
                break;
            }

            case 3:
                resource_request();
                break;

            case 4:
                release_resources();
                break;

            case 5:
                save_snapshot();
                break;

            case 6:
                restore_snapshot();
                break;

            case 7:
                random_stress_test();
                break;

            case 8:
                performance_analysis();
                break;

            case 0:
                running = 0;
                printf("Goodbye! Log saved to %s\n", LOG_FILE);
                break;

            default:
                printf("[ERROR] Invalid option.\n");
        }
    }

    log_close();
    return 0;
}
