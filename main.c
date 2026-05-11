#include <stdio.h>

#include "banker.h"
#include "logger.h"
#include "utils.h"

int main()
{
    log_open();

    int choice;

    printf("Banker's Algorithm Simulator\n");

    printf("1. Manual Input\n");
    printf("2. Demo Data\n");

    scanf("%d", &choice);

    if(choice == 1)
        init_from_input();
    else
        init_demo();

    print_state();

    int running = 1;

    while(running)
    {
        print_menu();

        scanf("%d", &choice);

        switch(choice)
        {
            case 1:
                print_state();
                break;

            case 2:
            {
                int seq[MAX_PROCESSES];

                if(safety_algorithm(seq,1))
                {
                    printf("SAFE\n");
                }
                else
                {
                    printf("UNSAFE\n");
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
                break;

            default:
                printf("Invalid option\n");
        }
    }

    log_close();

    return 0;
}
