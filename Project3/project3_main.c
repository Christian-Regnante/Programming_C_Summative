#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project3_inventory.h"

// Default binary storage and text seed filenames
#define BIN_FILENAME "library_data.bin"
#define DEFAULT_TXT_SEED "book_record_format2.txt"

void display_menu(void) {
    printf("\n=======================================================\n");
    printf("     LIBRARY BOOK INVENTORY MANAGEMENT SYSTEM          \n");
    printf("=======================================================\n");
    printf(" 1. Display All Books\n");
    printf(" 2. Add New Book\n");
    printf(" 3. Update Book Information\n");
    printf(" 4. Delete a Book\n");
    printf(" 5. Search Book by ID\n");
    printf(" 6. Search Book by Title\n");
    printf(" 7. Sort Inventory (by ID / Title / Copies)\n");
    printf(" 8. Generate Inventory Analytics Report\n");
    printf(" 9. Import/Seed Records from Text File (.txt)\n");
    printf("10. Save Inventory to Binary File (.bin)\n");
    printf(" 0. Exit Application\n");
    printf("=======================================================\n");
    printf("Enter your choice (0-10): ");
}

int main(void) {
    Inventory inventory;
    
    // Initialize dynamic inventory
    if (!init_inventory(&inventory)) {
        printf("[FATAL] Could not initialize inventory. Exiting application.\n");
        return EXIT_FAILURE;
    }

    printf("\nWelcome to the Library Inventory Management System!\n");
    printf("System initialized with dynamic memory allocation.\n");

    // Phase 2: Attempt to load binary file on startup
    printf("\nChecking for existing binary storage ('%s')...\n", BIN_FILENAME);
    if (!load_from_bin(&inventory, BIN_FILENAME)) {
        printf("\nNo binary database found or database is empty.\n");
        printf("Would you like to import sample records from '%s'? (y/n): ", DEFAULT_TXT_SEED);
        
        char response[10];
        if (fgets(response, sizeof(response), stdin) != NULL) {
            remove_newline(response);
            if (response[0] == 'y' || response[0] == 'Y') {
                import_from_txt(&inventory, DEFAULT_TXT_SEED);
            } else {
                printf("Starting with a fresh empty library inventory.\n");
            }
        }
    }

    int choice = -1;

    // Main Interactive CLI Loop
    while (1) {
        display_menu();

        // Read choice and validate integer input
        if (scanf("%d", &choice) != 1) {
            printf("\n[ERROR] Invalid input! Please enter a number between 0 and 10.\n");
            clear_input_buffer(); // Clear invalid characters from buffer
            continue;
        }

        clear_input_buffer(); // Clear newline left by scanf

        if (choice == 0) {
            printf("\nSaving inventory state before exit...\n");
            save_to_bin(&inventory, BIN_FILENAME);
            break;
        }

        switch (choice) {
            case 1:
                display_books(&inventory);
                break;
            case 2:
                add_book(&inventory);
                break;
            case 3:
                update_book(&inventory);
                break;
            case 4:
                delete_book(&inventory);
                break;
            case 5:
                search_by_id(&inventory);
                break;
            case 6:
                search_by_title(&inventory);
                break;
            case 7:
                sort_inventory(&inventory);
                break;
            case 8:
                generate_reports(&inventory);
                break;
            case 9: {
                char txt_filename[256];
                printf("\nEnter text seed filename to import (Press ENTER for '%s'): ", DEFAULT_TXT_SEED);
                if (fgets(txt_filename, sizeof(txt_filename), stdin) != NULL) {
                    remove_newline(txt_filename);
                    if (strlen(txt_filename) == 0) {
                        strcpy(txt_filename, DEFAULT_TXT_SEED);
                    }
                    import_from_txt(&inventory, txt_filename);
                }
                break;
            }
            case 10:
                save_to_bin(&inventory, BIN_FILENAME);
                break;
            default:
                printf("\n[ERROR] Option %d is not recognized. Please choose 0 to 10.\n", choice);
                break;
        }
    }

    // Clean up dynamic memory before exiting
    free_inventory(&inventory);
    printf("\nThank you for using the Library Inventory Management System. Goodbye!\n");

    return EXIT_SUCCESS;
}
