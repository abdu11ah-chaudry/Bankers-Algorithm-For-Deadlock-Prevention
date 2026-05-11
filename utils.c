#include <stdio.h>
#include "banker.h"
#include "utils.h"

void print_state(void)
{
    printf("\n===== SYSTEM STATE =====\n");

    printf("Available: ");
    for(int j=0;j<n_res;j++)
        printf("%4d", available[j]);

    printf("\n");
}

void print_menu(void)
{
    printf("\n========== MENU ==========\n");

    printf("1. Display state\n");
    printf("2. Run Safety Algorithm\n");
    printf("3. Resource Request\n");
    printf("4. Release Resources\n");
    printf("5. Save Snapshot\n");
    printf("6. Restore Snapshot\n");
    printf("7. Stress Test\n");
    printf("8. Performance Analysis\n");
    printf("0. Exit\n");

    printf("Choice: ");
}
