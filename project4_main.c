#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project4_conversions.h"
#include "project4_history.h"

#define HISTORY_BIN_FILE "unit_history.bin"

void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void display_main_menu(void) {
    printf("\n=======================================================\n");
    printf("      SMART CALCULATOR & UNIT CONVERSION TOOLKIT       \n");
    printf("=======================================================\n");
    printf(" 1. Perform Unit Conversion (Function Pointers)\n");
    printf(" 2. View Conversion History\n");
    printf(" 3. Search Conversion Records\n");
    printf(" 4. Sort Conversion Records (Comparator Callbacks)\n");
    printf(" 5. Apply Callback Operations (Batch & Filtering)\n");
    printf(" 6. Save History to Binary File (.bin)\n");
    printf(" 7. Load History from Binary File (.bin)\n");
    printf(" 0. Exit Application\n");
    printf("=======================================================\n");
    printf("Enter choice (0-7): ");
}

void display_conversion_menu(void) {
    printf("\n--- Select Unit Conversion ---\n");
    int count = get_num_conversions();
    for (int i = 0; i < count; i++) {
        const ConversionOption *opt = get_conversion_option(i);
        printf(" %d. %s (%s -> %s)\n", i + 1, opt->name, opt->from_unit, opt->to_unit);
    }
    printf(" 0. Back to Main Menu\n");
    printf("Select conversion (0-%d): ", count);
}

void perform_conversion_ui(HistoryManager *hm) {
    display_conversion_menu();

    int choice = -1;
    if (scanf("%d", &choice) != 1) {
        printf("[ERROR] Invalid choice format!\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    if (choice == 0) return;

    int total_options = get_num_conversions();
    if (choice < 1 || choice > total_options) {
        printf("[ERROR] Option out of range (1-%d).\n", total_options);
        return;
    }

    const ConversionOption *opt = get_conversion_option(choice - 1);
    if (opt == NULL || opt->convert == NULL) {
        printf("[ERROR] Invalid conversion option selected.\n");
        return;
    }

    printf("\nEnter value in %s: ", opt->from_unit);
    double input_val = 0.0;
    if (scanf("%lf", &input_val) != 1) {
        printf("[ERROR] Invalid numeric value!\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    // DYNAMIC FUNCTION POINTER INVOCATION
    double result = opt->convert(input_val);

    printf("\n=======================================================\n");
    printf(" CONVERSION RESULT (via Function Pointer)\n");
    printf("=======================================================\n");
    printf(" Conversion : %s\n", opt->name);
    printf(" Input      : %.4f %s\n", input_val, opt->from_unit);
    printf(" Result     : %.4f %s\n", result, opt->to_unit);
    printf("=======================================================\n");

    if (add_record(hm, opt->name, opt->from_unit, opt->to_unit, input_val, result)) {
        printf("[INFO] Conversion recorded in history (Record #%d).\n", hm->count);
        save_history_bin(hm, HISTORY_BIN_FILE);
    }
}

void search_records_ui(const HistoryManager *hm) {
    if (hm == NULL || hm->count == 0) {
        printf("\n[INFO] History is currently empty. Nothing to search.\n");
        return;
    }

    printf("\n--- Search Conversion Records ---\n");
    printf(" 1. Search by Conversion Type Keyword\n");
    printf(" 2. Search by Converted Result Value Range\n");
    printf(" 0. Cancel\n");
    printf("Select option (0-2): ");

    int choice = -1;
    if (scanf("%d", &choice) != 1) {
        printf("[ERROR] Invalid choice.\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    if (choice == 0) return;

    if (choice == 1) {
        printf("Enter conversion type keyword (e.g. Miles, Celsius): ");
        char keyword[100];
        if (fgets(keyword, sizeof(keyword), stdin) != NULL) {
            size_t len = strlen(keyword);
            if (len > 0 && keyword[len - 1] == '\n') keyword[len - 1] = '\0';
            if (strlen(keyword) > 0) {
                search_history_by_type(hm, keyword);
            }
        }
    } else if (choice == 2) {
        printf("Enter minimum converted value: ");
        double min_val = 0.0, max_val = 0.0;
        if (scanf("%lf", &min_val) == 1) {
            printf("Enter maximum converted value: ");
            if (scanf("%lf", &max_val) == 1) {
                search_history_by_value(hm, min_val, max_val);
            } else {
                printf("[ERROR] Invalid maximum value.\n");
            }
        } else {
            printf("[ERROR] Invalid minimum value.\n");
        }
        clear_input_buffer();
    } else {
        printf("[ERROR] Invalid option selected.\n");
    }
}

void sort_records_ui(HistoryManager *hm) {
    if (hm == NULL || hm->count <= 1) {
        printf("\n[INFO] At least 2 conversion records are needed to sort.\n");
        return;
    }

    printf("\n--- Sort Conversion History (Comparator Callbacks) ---\n");
    printf(" 1. Sort by Conversion Type (Alphabetical A-Z)\n");
    printf(" 2. Sort by Converted Result (Ascending: Low to High)\n");
    printf(" 3. Sort by Converted Result (Descending: High to Low)\n");
    printf(" 0. Cancel\n");
    printf("Select option (0-3): ");

    int choice = -1;
    if (scanf("%d", &choice) != 1) {
        printf("[ERROR] Invalid input.\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    RecordComparator comp = NULL;
    switch (choice) {
        case 1: comp = compare_by_type; break;
        case 2: comp = compare_by_output_asc; break;
        case 3: comp = compare_by_output_desc; break;
        case 0: return;
        default:
            printf("[ERROR] Invalid selection.\n");
            return;
    }

    sort_history(hm, comp);
}

void apply_callbacks_ui(HistoryManager *hm) {
    if (hm == NULL || hm->count == 0) {
        printf("\n[INFO] History is currently empty. No records to process.\n");
        return;
    }

    printf("\n--- Apply Callback Operations ---\n");
    printf(" 1. Batch Round Converted Results to N Decimal Places (Processing Callback)\n");
    printf(" 2. Filter History by Minimum Converted Value (Predicate Callback)\n");
    printf(" 3. Filter History by Conversion Type Keyword (Predicate Callback)\n");
    printf(" 0. Cancel\n");
    printf("Select option (0-3): ");

    int choice = -1;
    if (scanf("%d", &choice) != 1) {
        printf("[ERROR] Invalid choice format.\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    if (choice == 0) return;

    if (choice == 1) {
        printf("Enter desired decimal precision places (0-6): ");
        int decimals = 2;
        if (scanf("%d", &decimals) == 1 && decimals >= 0 && decimals <= 6) {
            process_history_batch(hm, callback_round_precision, &decimals);
            printf("\n[SUCCESS] Applied batch precision rounding callback (%d decimals) to all records!\n", decimals);
            view_history(hm);
            save_history_bin(hm, HISTORY_BIN_FILE);
        } else {
            printf("[ERROR] Invalid decimal places input.\n");
        }
        clear_input_buffer();
    } else if (choice == 2) {
        printf("Enter minimum converted value threshold: ");
        double min_val = 0.0;
        if (scanf("%lf", &min_val) == 1) {
            filter_history(hm, predicate_filter_by_min_val, &min_val);
        } else {
            printf("[ERROR] Invalid threshold value.\n");
        }
        clear_input_buffer();
    } else if (choice == 3) {
        printf("Enter keyword to filter (e.g. Miles, Celsius): ");
        char keyword[50];
        if (fgets(keyword, sizeof(keyword), stdin) != NULL) {
            size_t len = strlen(keyword);
            if (len > 0 && keyword[len - 1] == '\n') keyword[len - 1] = '\0';
            if (strlen(keyword) > 0) {
                filter_history(hm, predicate_filter_by_type, keyword);
            }
        }
    } else {
        printf("[ERROR] Unrecognized callback option.\n");
    }
}

int main(void) {
    HistoryManager history;
    if (!init_history(&history)) {
        printf("[FATAL] Could not initialize history manager. Exiting.\n");
        return EXIT_FAILURE;
    }

    printf("\nWelcome to Project 4: Smart Calculator & Unit Conversion Toolkit!\n");
    printf("System initialized with dynamic memory allocation.\n");

    load_history_bin(&history, HISTORY_BIN_FILE);

    int choice = -1;
    while (1) {
        display_main_menu();

        if (scanf("%d", &choice) != 1) {
            printf("\n[ERROR] Invalid input! Please enter a number between 0 and 7.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        if (choice == 0) {
            printf("\nSaving history state before exit...\n");
            save_history_bin(&history, HISTORY_BIN_FILE);
            break;
        }

        switch (choice) {
            case 1:
                perform_conversion_ui(&history);
                break;
            case 2:
                view_history(&history);
                break;
            case 3:
                search_records_ui(&history);
                break;
            case 4:
                sort_records_ui(&history);
                break;
            case 5:
                apply_callbacks_ui(&history);
                break;
            case 6:
                save_history_bin(&history, HISTORY_BIN_FILE);
                break;
            case 7:
                load_history_bin(&history, HISTORY_BIN_FILE);
                break;
            default:
                printf("\n[ERROR] Unrecognized option %d. Please try again.\n", choice);
                break;
        }
    }

    free_history(&history);
    printf("Thank you for using the Smart Calculator & Unit Conversion Toolkit. Goodbye!\n");

    return EXIT_SUCCESS;
}
