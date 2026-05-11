#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "banker.h"
#include "logger.h"
#include "utils.h"

int n_procs = 0;
int n_res = 0;

int allocation[MAX_PROCESSES][MAX_RESOURCES];
int maximum[MAX_PROCESSES][MAX_RESOURCES];
int need[MAX_PROCESSES][MAX_RESOURCES];
int available[MAX_RESOURCES];

Snapshot snapshot;
