#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include "project3_inventory.h"

// Binary storage file name
#define BIN_FILENAME "library_data.bin"

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

    int choice = -1;

    printf("\nWelcome to the Library Inventory Management System!\n");
    printf("System initialized with dynamic memory allocation.\n");

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
            printf("\nExiting application...\n");
            // Prompt to save before exit in later phase
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
            case 9:
                import_from_txt(&inventory, "book_record_format2.txt");
                break;
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
    printf("Thank you for using the Library Inventory Management System. Goodbye!\n");

    return EXIT_SUCCESS;
}
