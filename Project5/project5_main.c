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
    printf(" 1. Scan & Analyze Text Files Concurrently\n");
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
        printf("\n[INFO] No processable text files were found in directory.\n");
        return;
    }

    printf("\n=======================================================\n");
    printf("         SCAN & ANALYZE TEXT FILES (PTHREADS)          \n");
    printf("=======================================================\n");
    printf(" 1. Analyze ALL %d Found File(s) Concurrently\n", num_found);
    for (int i = 0; i < num_found; i++) {
        printf(" %d. %s\n", i + 2, found_files[i]);
    }
    printf(" 0. Back to Main Menu\nSelect option (0-%d): ", num_found + 1);

    int choice = -1;
    if (scanf("%d", &choice) != 1 || choice == 0) { clear_input_buffer(); return; }
    clear_input_buffer();

    if (choice == 1) {
        FileTask *tasks = (FileTask *)malloc(num_found * sizeof(FileTask));
        memset(tasks, 0, num_found * sizeof(FileTask));
        for (int i = 0; i < num_found; i++) {
            strncpy(tasks[i].input_filename, found_files[i], 255);
        }
        run_multithreaded_analysis(tasks, num_found);
        free(tasks);
    } else if (choice >= 2 && choice <= num_found + 1) {
        int idx = choice - 2;
        FileTask task = {0};
        strncpy(task.input_filename, found_files[idx], 255);
        run_multithreaded_analysis(&task, 1);
    } else {
        printf("[ERROR] Option out of range.\n");
    }
}

void view_reports_menu_ui(void) {
    char found_files[50][256];
    int num_found = scan_directory_files(found_files, 50);

    if (num_found == 0) {
        printf("\n[INFO] No analysis reports available yet.\n");
        return;
    }

    printf("\n--- View Generated Analysis Reports ---\n");
    for (int i = 0; i < num_found; i++) {
        printf(" %d. %s_analysis.txt\n", i + 1, found_files[i]);
    }
    printf(" 0. Back to Main Menu\nSelect report to view (0-%d): ", num_found);

    int choice = -1;
    if (scanf("%d", &choice) != 1 || choice == 0) { clear_input_buffer(); return; }
    clear_input_buffer();

    if (choice >= 1 && choice <= num_found) {
        char report_name[300];
        snprintf(report_name, sizeof(report_name), "%s_analysis.txt", found_files[choice - 1]);
        view_report_file(report_name);
    }
}

int main(void) {
    printf("\nWelcome to the Multi-Threaded File Processing System!\n");
    log_activity("Application started.");

    int choice = -1;
    while (1) {
        display_main_menu();
        if (scanf("%d", &choice) != 1) {
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        if (choice == 0) {
            log_activity("Application terminated gracefully.");
            break;
        }

        switch (choice) {
            case 1: analyze_files_menu_ui(); break;
            case 2: view_reports_menu_ui(); break;
            case 3: view_activity_log(); break;
            default: printf("[ERROR] Choice out of range.\n"); break;
        }
    }

    printf("\nThank you for using the Multi-Threaded File Processor. Goodbye!\n");
    return EXIT_SUCCESS;
}
