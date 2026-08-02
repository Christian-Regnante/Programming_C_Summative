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

void perform_conversion_ui(HistoryManager *hm) {
    printf("\n--- Select Unit Conversion ---\n");
    int count = get_num_conversions();
    for (int i = 0; i < count; i++) {
        const ConversionOption *opt = get_conversion_option(i);
        printf(" %d. %s (%s -> %s)\n", i + 1, opt->name, opt->from_unit, opt->to_unit);
    }
    printf(" 0. Back to Main Menu\nSelect conversion (0-%d): ", count);

    int choice = -1;
    if (scanf("%d", &choice) != 1 || choice == 0) { clear_input_buffer(); return; }
    clear_input_buffer();

    if (choice < 1 || choice > count) {
        printf("[ERROR] Option out of range (1-%d).\n", count);
        return;
    }

    const ConversionOption *opt = get_conversion_option(choice - 1);
    if (!opt || !opt->convert) return;

    printf("Enter value in %s: ", opt->from_unit);
    double input_val = 0.0;
    if (scanf("%lf", &input_val) != 1) { clear_input_buffer(); return; }
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
    if (!hm || hm->count == 0) { printf("\n[INFO] History is empty.\n"); return; }

    printf("\n--- Search Records ---\n 1. By Type Keyword\n 2. By Result Value Range\n 0. Cancel\nChoice: ");
    int choice = 0;
    if (scanf("%d", &choice) != 1 || choice == 0) { clear_input_buffer(); return; }
    clear_input_buffer();

    if (choice == 1) {
        char keyword[100];
        printf("Enter conversion type keyword (e.g. Miles, Celsius): ");
        if (fgets(keyword, sizeof(keyword), stdin)) {
            size_t len = strlen(keyword);
            if (len > 0 && keyword[len - 1] == '\n') keyword[len - 1] = '\0';
            search_history_by_type(hm, keyword);
        }
    } else if (choice == 2) {
        double min_val = 0.0, max_val = 0.0;
        printf("Enter min value: ");
        if (scanf("%lf", &min_val) == 1) {
            printf("Enter max value: ");
            if (scanf("%lf", &max_val) == 1) search_history_by_value(hm, min_val, max_val);
        }
        clear_input_buffer();
    }
}

void sort_records_ui(HistoryManager *hm) {
    if (!hm || hm->count <= 1) { printf("\n[INFO] At least 2 records are needed to sort.\n"); return; }
    printf("\n--- Sort Records ---\n 1. By Conversion Name (A-Z)\n 2. By Result (Ascending)\n 3. By Result (Descending)\n 0. Cancel\nChoice: ");
    int choice = 0;
    if (scanf("%d", &choice) != 1 || choice == 0) { clear_input_buffer(); return; }
    clear_input_buffer();

    if (choice == 1) sort_history(hm, compare_by_type);
    else if (choice == 2) sort_history(hm, compare_by_output_asc);
    else if (choice == 3) sort_history(hm, compare_by_output_desc);
    
    printf("[SUCCESS] History sorted.\n");
    view_history(hm);
}

void callback_operations_ui(HistoryManager *hm) {
    if (!hm || hm->count == 0) { printf("\n[INFO] History is empty.\n"); return; }
    printf("\n--- Callback Engine Operations ---\n 1. Batch Round Precision (2 Decimals)\n 2. Filter Records with Result >= Threshold\n 0. Cancel\nChoice: ");
    int choice = 0;
    if (scanf("%d", &choice) != 1 || choice == 0) { clear_input_buffer(); return; }
    clear_input_buffer();

    if (choice == 1) {
        int decimals = 2;
        process_history_batch(hm, callback_round_precision, &decimals);
        printf("[SUCCESS] All values rounded to 2 decimal places.\n");
        view_history(hm);
    } else if (choice == 2) {
        double threshold = 0.0;
        printf("Enter minimum result value threshold: ");
        if (scanf("%lf", &threshold) == 1) filter_history(hm, predicate_filter_by_min_val, &threshold);
        clear_input_buffer();
    }
}

int main(void) {
    HistoryManager hm;
    if (!init_history(&hm)) return EXIT_FAILURE;

    printf("\nWelcome to the Unit Conversion Toolkit!\n");
    load_history_bin(&hm, HISTORY_BIN_FILE);

    int choice = -1;
    while (1) {
        display_main_menu();
        if (scanf("%d", &choice) != 1) {
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        if (choice == 0) {
            save_history_bin(&hm, HISTORY_BIN_FILE);
            break;
        }

        switch (choice) {
            case 1: perform_conversion_ui(&hm); break;
            case 2: view_history(&hm); break;
            case 3: search_records_ui(&hm); break;
            case 4: sort_records_ui(&hm); break;
            case 5: callback_operations_ui(&hm); break;
            case 6: save_history_bin(&hm, HISTORY_BIN_FILE); printf("[SUCCESS] History saved.\n"); break;
            case 7: load_history_bin(&hm, HISTORY_BIN_FILE); break;
            default: printf("[ERROR] Invalid choice.\n"); break;
        }
    }

    free_history(&hm);
    printf("\nThank you for using the Unit Conversion Toolkit. Goodbye!\n");
    return EXIT_SUCCESS;
}
