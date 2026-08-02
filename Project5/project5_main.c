#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project5_analyzer.h"

void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void display_main_menu(void) {
    printf("\n=======================================================\n");
    printf("     MULTI-THREADED FILE PROCESSING SYSTEM (PTHREADS)  \n");
    printf("=======================================================\n");
    printf(" 1. Scan & Analyze Text Files (Sub-Menu)\n");
    printf(" 2. View Generated Analysis Reports\n");
    printf(" 3. View System Activity Logs (Docker/Git Style)\n");
    printf(" 0. Exit Application\n");
    printf("=======================================================\n");
    printf("Enter choice (0-3): ");
}

void analyze_files_menu_ui(void) {
    char found_files[50][256];
    int num_found = scan_directory_files(found_files, 50);

    if (num_found == 0) {
        printf("\n[INFO] No processable text files (.txt, .c, .h) were found in workspace directory.\n");
        return;
    }

    printf("\n=======================================================\n");
    printf("         SCAN & ANALYZE TEXT FILES (SUB-MENU)          \n");
    printf("=======================================================\n");
    printf(" 1. Analyze ALL %d Found File(s) Concurrently\n", num_found);
    for (int i = 0; i < num_found; i++) {
        printf(" %d. %s\n", i + 2, found_files[i]);
    }
    printf(" 0. Return to Main Menu\n");
    printf("=======================================================\n");
    printf("Select option (0-%d): ", num_found + 1);

    int choice = -1;
    if (scanf("%d", &choice) != 1) {
        printf("[ERROR] Invalid selection.\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    if (choice == 0) return;

    if (choice == 1) {
        FileTask tasks[50];
        memset(tasks, 0, sizeof(tasks));
        for (int i = 0; i < num_found; i++) {
            strncpy(tasks[i].input_filename, found_files[i], sizeof(tasks[i].input_filename) - 1);
            snprintf(tasks[i].output_filename, sizeof(tasks[i].output_filename), "%s_analysis.txt", found_files[i]);
        }

        run_multithreaded_analysis(tasks, num_found);
        printf("\n[SUCCESS] Analysis complete for all %d files! Select Option 2 from Main Menu to view reports.\n", num_found);
    } else if (choice >= 2 && choice <= num_found + 1) {
        int selected_idx = choice - 2;
        FileTask task;
        memset(&task, 0, sizeof(FileTask));
        strncpy(task.input_filename, found_files[selected_idx], sizeof(task.input_filename) - 1);
        snprintf(task.output_filename, sizeof(task.output_filename), "%s_analysis.txt", found_files[selected_idx]);

        run_multithreaded_analysis(&task, 1);
        printf("\n[SUCCESS] Analysis complete for '%s'! Select Option 2 from Main Menu to view report '%s'.\n",
               task.input_filename, task.output_filename);
    } else {
        printf("[ERROR] Selection out of range (0-%d).\n", num_found + 1);
    }
}

void view_reports_menu_ui(void) {
    char found_reports[50][256];
    int num_found = scan_report_files(found_reports, 50);

    if (num_found == 0) {
        printf("\n[INFO] No generated analysis reports (*_analysis.txt) found.\n");
        printf("Please run Option 1 from Main Menu to analyze files first!\n");
        return;
    }

    printf("\n=======================================================\n");
    printf("         VIEW GENERATED ANALYSIS REPORTS               \n");
    printf("=======================================================\n");
    printf(" 1. Display ALL %d Generated Analysis Report(s)\n", num_found);
    for (int i = 0; i < num_found; i++) {
        printf(" %d. %s\n", i + 2, found_reports[i]);
    }
    printf(" 0. Return to Main Menu\n");
    printf("=======================================================\n");
    printf("Select report to view (0-%d): ", num_found + 1);

    int choice = -1;
    if (scanf("%d", &choice) != 1) {
        printf("[ERROR] Invalid choice format.\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    if (choice == 0) return;

    if (choice == 1) {
        for (int i = 0; i < num_found; i++) {
            view_report_file(found_reports[i]);
        }
    } else if (choice >= 2 && choice <= num_found + 1) {
        int idx = choice - 2;
        view_report_file(found_reports[idx]);
    } else {
        printf("[ERROR] Selection out of range (0-%d).\n", num_found + 1);
    }
}

int main(void) {
    printf("\nWelcome to Project 5: Multi-threaded File Processing System!\n");
    log_activity("System launched.");

    int choice = -1;
    while (1) {
        display_main_menu();

        if (scanf("%d", &choice) != 1) {
            printf("\n[ERROR] Invalid input! Please enter a number between 0 and 3.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        if (choice == 0) {
            printf("\nExiting application. Goodbye!\n");
            log_activity("System shutdown successfully.");
            break;
        }

        switch (choice) {
            case 1:
                analyze_files_menu_ui();
                break;
            case 2:
                view_reports_menu_ui();
                break;
            case 3:
                view_activity_log();
                break;
            default:
                printf("\n[ERROR] Unrecognized option %d. Please try again.\n", choice);
                break;
        }
    }

    return EXIT_SUCCESS;
}
