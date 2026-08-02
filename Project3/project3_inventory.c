#include "project3_inventory.h"

/* =========================================================================
 * MEMORY MANAGEMENT FUNCTIONS
 * ========================================================================= */

/**
 * Initializes the dynamic inventory structure.
 * Allocates initial memory for the Book array.
 *
 * @param inv Pointer to the Inventory struct to initialize.
 * @return 1 on success, 0 on memory allocation failure.
 */
int init_inventory(Inventory *inv) {
    if (inv == NULL) return 0;

    inv->capacity = INITIAL_CAPACITY;
    inv->count = 0;
    inv->books = (Book *)malloc(inv->capacity * sizeof(Book));

    if (inv->books == NULL) {
        printf("[ERROR] Memory allocation failed during inventory initialization!\n");
        return 0;
    }

    printf("[DEBUG] Inventory initialized with capacity: %d\n", inv->capacity);
    return 1;
}

/**
 * Resizes the dynamic Book array when capacity is reached.
 * Doubles current capacity using realloc with safety checks.
 *
 * @param inv Pointer to the Inventory struct to resize.
 * @return 1 on success, 0 on realloc failure.
 */
int resize_inventory(Inventory *inv) {
    if (inv == NULL) return 0;

    int new_capacity = inv->capacity * 2;
    // Use temporary pointer to prevent memory leak if realloc fails
    Book *temp = (Book *)realloc(inv->books, new_capacity * sizeof(Book));

    if (temp == NULL) {
        printf("[ERROR] Memory reallocation failed! Inventory remains at capacity %d.\n", inv->capacity);
        return 0;
    }

    inv->books = temp;
    inv->capacity = new_capacity;
    printf("[DEBUG] Inventory expanded. New capacity: %d\n", inv->capacity);
    return 1;
}

/**
 * Frees all allocated memory in the inventory and resets counters.
 *
 * @param inv Pointer to the Inventory struct to cleanup.
 */
void free_inventory(Inventory *inv) {
    if (inv != NULL && inv->books != NULL) {
        free(inv->books);
        inv->books = NULL;
        inv->count = 0;
        inv->capacity = 0;
        printf("[DEBUG] Dynamic memory freed successfully.\n");
    }
}

/* =========================================================================
 * UTILITY FUNCTIONS
 * ========================================================================= */

/**
 * Clears remaining characters from stdin buffer (flush buffer).
 */
void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // Read and discard until newline or EOF
    }
}

/**
 * Removes trailing newline character from strings read via fgets.
 *
 * @param str Null-terminated string to modify.
 */
void remove_newline(char *str) {
    if (str == NULL) return;
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

/**
 * Searches for a book by its exact ID.
 *
 * @param inv Pointer to Inventory struct.
 * @param id  ID string to search for.
 * @return Array index (0 to count-1) if found, or -1 if not found.
 */
int find_book_index_by_id(const Inventory *inv, const char *id) {
    if (inv == NULL || id == NULL) return -1;

    for (int i = 0; i < inv->count; i++) {
        if (strcmp(inv->books[i].id, id) == 0) {
            return i;
        }
    }
    return -1;
}

/* =========================================================================
 * FILE I/O IMPLEMENTATIONS (PHASE 2)
 * ========================================================================= */

/**
 * Saves current inventory records to a binary file (.bin).
 *
 * @param inv      Pointer to Inventory struct.
 * @param filename Name/path of binary file.
 * @return 1 on success, 0 on failure.
 */
int save_to_bin(const Inventory *inv, const char *filename) {
    if (inv == NULL || filename == NULL) return 0;

    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("[ERROR] Could not open binary file '%s' for writing.\n", filename);
        return 0;
    }

    // Write record count first
    if (fwrite(&(inv->count), sizeof(int), 1, fp) != 1) {
        printf("[ERROR] Failed to write inventory record count to '%s'.\n", filename);
        fclose(fp);
        return 0;
    }

    // Write array of Book structs
    if (inv->count > 0) {
        size_t written = fwrite(inv->books, sizeof(Book), (size_t)inv->count, fp);
        if (written != (size_t)inv->count) {
            printf("[ERROR] Wrote %zu of %d records to '%s'. File may be incomplete.\n", written, inv->count, filename);
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    printf("[SUCCESS] Saved %d book record(s) to binary file '%s'.\n", inv->count, filename);
    return 1;
}

/**
 * Loads inventory records from a binary file (.bin).
 *
 * @param inv      Pointer to Inventory struct.
 * @param filename Name/path of binary file.
 * @return 1 if loaded successfully, 0 if file does not exist or failed to load.
 */
int load_from_bin(Inventory *inv, const char *filename) {
    if (inv == NULL || filename == NULL) return 0;

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        // File does not exist yet (normal on first launch)
        return 0;
    }

    int saved_count = 0;
    if (fread(&saved_count, sizeof(int), 1, fp) != 1) {
        printf("[WARNING] Binary file '%s' is empty or corrupted.\n", filename);
        fclose(fp);
        return 0;
    }

    if (saved_count < 0) {
        printf("[ERROR] Invalid record count (%d) in binary file '%s'.\n", saved_count, filename);
        fclose(fp);
        return 0;
    }

    // Ensure inventory dynamic array has sufficient capacity
    while (inv->capacity < saved_count) {
        if (!resize_inventory(inv)) {
            printf("[ERROR] Failed to expand memory while loading '%s'.\n", filename);
            fclose(fp);
            return 0;
        }
    }

    if (saved_count > 0) {
        size_t read_count = fread(inv->books, sizeof(Book), (size_t)saved_count, fp);
        if (read_count != (size_t)saved_count) {
            printf("[WARNING] Read %zu of %d expected records from '%s'.\n", read_count, saved_count, filename);
            inv->count = (int)read_count;
        } else {
            inv->count = saved_count;
        }
    } else {
        inv->count = 0;
    }

    fclose(fp);
    printf("[SUCCESS] Loaded %d book record(s) from binary file '%s'.\n", inv->count, filename);
    return 1;
}

/**
 * Imports book records from a pipe-delimited text file (e.g. ID|Title|Author|Category|Copies)
 * and merges them into the current inventory, preventing duplicate Book IDs.
 *
 * @param inv      Pointer to Inventory struct.
 * @param filename Name/path of text file to import.
 * @return Total number of new books imported.
 */
int import_from_txt(Inventory *inv, const char *filename) {
    if (inv == NULL || filename == NULL) return 0;

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("[ERROR] Could not open text seed file '%s' for reading.\n", filename);
        return 0;
    }

    char line[512];
    int imported_count = 0;
    int line_num = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        line_num++;
        remove_newline(line);

        // Skip empty lines
        if (strlen(line) == 0) continue;

        // Skip header line if present
        if (line_num == 1 && (strncmp(line, "Book ID", 7) == 0 || strncmp(line, "ID", 2) == 0)) {
            continue;
        }

        Book temp;
        memset(&temp, 0, sizeof(Book));

        // Parse pipe-delimited tokens: ID|Title|Author|Category|Copies
        // Using sscanf with character scans excluding '|'
        int matched = sscanf(line, "%19[^|]|%99[^|]|%99[^|]|%49[^|]|%d",
                             temp.id, temp.title, temp.author, temp.category, &temp.copies);

        if (matched != 5) {
            printf("[WARNING] Line %d format invalid. Skipped: '%s'\n", line_num, line);
            continue;
        }

        // Validate non-duplicate Book ID
        if (find_book_index_by_id(inv, temp.id) != -1) {
            printf("[INFO] Book ID '%s' already exists. Skipping duplicate entry on line %d.\n", temp.id, line_num);
            continue;
        }

        // Expand memory if array capacity reached
        if (inv->count >= inv->capacity) {
            if (!resize_inventory(inv)) {
                printf("[ERROR] Memory allocation failed while importing line %d. Import stopped.\n", line_num);
                break;
            }
        }

        // Insert record into inventory array
        inv->books[inv->count] = temp;
        inv->count++;
        imported_count++;
    }

    fclose(fp);

    printf("\n[IMPORT COMPLETE] Successfully imported %d new book record(s) from '%s'.\n", imported_count, filename);
    if (imported_count > 0) {
        // Automatically save updated memory to binary file
        save_to_bin(inv, "library_data.bin");
    }

    return imported_count;
}

/* =========================================================================
 * CRUD OPERATIONS IMPLEMENTATIONS (PHASE 3)
 * ========================================================================= */

/**
 * Displays all books in a formatted ASCII table layout.
 *
 * @param inv Pointer to Inventory struct.
 */
void display_books(const Inventory *inv) {
    if (inv == NULL || inv->count == 0) {
        printf("\n=========================================================================================\n");
        printf("                                 LIBRARY BOOK INVENTORY                                  \n");
        printf("=========================================================================================\n");
        printf(" No books currently registered in inventory.\n");
        printf("=========================================================================================\n");
        return;
    }

    printf("\n=========================================================================================\n");
    printf("                                 LIBRARY BOOK INVENTORY                                  \n");
    printf("=========================================================================================\n");
    printf(" %-3s | %-8s | %-28s | %-20s | %-12s | %-6s\n",
           "#", "Book ID", "Title", "Author", "Category", "Copies");
    printf("-----+----------+------------------------------+----------------------+--------------+--------\n");

    for (int i = 0; i < inv->count; i++) {
        const Book *b = &inv->books[i];
        
        // Truncate long strings for clean table alignment
        char title_disp[29], author_disp[21], cat_disp[13];
        snprintf(title_disp, sizeof(title_disp), "%s", b->title);
        snprintf(author_disp, sizeof(author_disp), "%s", b->author);
        snprintf(cat_disp, sizeof(cat_disp), "%s", b->category);

        printf(" %-3d | %-8s | %-28s | %-20s | %-12s | %-6d\n",
               i + 1, b->id, title_disp, author_disp, cat_disp, b->copies);
    }

    printf("=========================================================================================\n");
    printf(" Total Distinct Books: %d | Dynamic Memory Capacity: %d\n", inv->count, inv->capacity);
    printf("=========================================================================================\n");
}

/**
 * Adds a new book to the inventory.
 * Validates unique Book ID, non-empty fields, and non-negative copy count.
 * Expands dynamic array memory if capacity is reached.
 *
 * @param inv Pointer to Inventory struct.
 */
void add_book(Inventory *inv) {
    if (inv == NULL) return;

    printf("\n-------------------------------------------------------\n");
    printf("                    ADD NEW BOOK                       \n");
    printf("-------------------------------------------------------\n");

    Book new_book;
    memset(&new_book, 0, sizeof(Book));

    // 1. Read & Validate Book ID
    while (1) {
        printf("Enter Book ID (e.g. B011): ");
        if (fgets(new_book.id, sizeof(new_book.id), stdin) == NULL) return;
        remove_newline(new_book.id);

        if (strlen(new_book.id) == 0) {
            printf("[ERROR] Book ID cannot be empty. Please try again.\n");
            continue;
        }

        // Duplicate check
        if (find_book_index_by_id(inv, new_book.id) != -1) {
            printf("[ERROR] Book ID '%s' already exists in inventory! IDs must be unique.\n", new_book.id);
            continue;
        }

        break; // Unique and valid ID
    }

    // 2. Read Title
    while (1) {
        printf("Enter Title: ");
        if (fgets(new_book.title, sizeof(new_book.title), stdin) == NULL) return;
        remove_newline(new_book.title);

        if (strlen(new_book.title) == 0) {
            printf("[ERROR] Title cannot be empty. Please try again.\n");
            continue;
        }
        break;
    }

    // 3. Read Author
    while (1) {
        printf("Enter Author: ");
        if (fgets(new_book.author, sizeof(new_book.author), stdin) == NULL) return;
        remove_newline(new_book.author);

        if (strlen(new_book.author) == 0) {
            printf("[ERROR] Author cannot be empty. Please try again.\n");
            continue;
        }
        break;
    }

    // 4. Read Category
    while (1) {
        printf("Enter Category (e.g. Fiction, Science): ");
        if (fgets(new_book.category, sizeof(new_book.category), stdin) == NULL) return;
        remove_newline(new_book.category);

        if (strlen(new_book.category) == 0) {
            printf("[ERROR] Category cannot be empty. Please try again.\n");
            continue;
        }
        break;
    }

    // 5. Read Copies Available
    while (1) {
        printf("Enter Number of Available Copies: ");
        if (scanf("%d", &new_book.copies) != 1) {
            printf("[ERROR] Invalid number format! Please enter a valid integer.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        if (new_book.copies < 0) {
            printf("[ERROR] Copy count cannot be negative. Please try again.\n");
            continue;
        }
        break;
    }

    // Check dynamic array capacity & expand if full
    if (inv->count >= inv->capacity) {
        if (!resize_inventory(inv)) {
            printf("[ERROR] Could not allocate memory for new book!\n");
            return;
        }
    }

    // Insert record into memory
    inv->books[inv->count] = new_book;
    inv->count++;

    printf("\n[SUCCESS] Book '%s' (ID: %s) added successfully!\n", new_book.title, new_book.id);

    // Save to binary file
    save_to_bin(inv, "library_data.bin");
}

/**
 * Updates information for an existing book in inventory.
 *
 * @param inv Pointer to Inventory struct.
 */
void update_book(Inventory *inv) {
    if (inv == NULL || inv->count == 0) {
        printf("\n[INFO] Inventory is currently empty. Nothing to update.\n");
        return;
    }

    printf("\n-------------------------------------------------------\n");
    printf("                  UPDATE BOOK RECORD                   \n");
    printf("-------------------------------------------------------\n");
    printf("Enter Book ID to update: ");

    char search_id[MAX_ID_LEN];
    if (fgets(search_id, sizeof(search_id), stdin) == NULL) return;
    remove_newline(search_id);

    int idx = find_book_index_by_id(inv, search_id);
    if (idx == -1) {
        printf("[ERROR] Book with ID '%s' was not found in inventory.\n", search_id);
        return;
    }

    Book *b = &inv->books[idx];

    printf("\nExisting Record Found:\n");
    printf("  Book ID  : %s\n", b->id);
    printf("  Title    : %s\n", b->title);
    printf("  Author   : %s\n", b->author);
    printf("  Category : %s\n", b->category);
    printf("  Copies   : %d\n", b->copies);

    printf("\nWhat field would you like to update?\n");
    printf("  1. Title\n");
    printf("  2. Author\n");
    printf("  3. Category\n");
    printf("  4. Available Copies\n");
    printf("  5. Update All Fields\n");
    printf("  0. Cancel\n");
    printf("Select option (0-5): ");

    int choice = -1;
    if (scanf("%d", &choice) != 1) {
        printf("[ERROR] Invalid choice. Update cancelled.\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    if (choice == 0) {
        printf("Update cancelled.\n");
        return;
    }

    int modified = 0;

    if (choice == 1 || choice == 5) {
        printf("Enter New Title: ");
        char buf[MAX_TITLE_LEN];
        if (fgets(buf, sizeof(buf), stdin) != NULL) {
            remove_newline(buf);
            if (strlen(buf) > 0) {
                strcpy(b->title, buf);
                modified = 1;
            }
        }
    }

    if (choice == 2 || choice == 5) {
        printf("Enter New Author: ");
        char buf[MAX_AUTHOR_LEN];
        if (fgets(buf, sizeof(buf), stdin) != NULL) {
            remove_newline(buf);
            if (strlen(buf) > 0) {
                strcpy(b->author, buf);
                modified = 1;
            }
        }
    }

    if (choice == 3 || choice == 5) {
        printf("Enter New Category: ");
        char buf[MAX_CATEGORY_LEN];
        if (fgets(buf, sizeof(buf), stdin) != NULL) {
            remove_newline(buf);
            if (strlen(buf) > 0) {
                strcpy(b->category, buf);
                modified = 1;
            }
        }
    }

    if (choice == 4 || choice == 5) {
        printf("Enter New Number of Copies: ");
        int new_copies = -1;
        if (scanf("%d", &new_copies) == 1 && new_copies >= 0) {
            b->copies = new_copies;
            modified = 1;
        } else {
            printf("[WARNING] Invalid copy count input. Copies unchanged.\n");
        }
        clear_input_buffer();
    }

    if (modified) {
        printf("\n[SUCCESS] Book record ID '%s' updated successfully!\n", b->id);
        save_to_bin(inv, "library_data.bin");
    } else {
        printf("\nNo changes were made to book record ID '%s'.\n", b->id);
    }
}

/**
 * Deletes a book from the inventory by its Book ID.
 * Shifts remaining elements left in the dynamic array to preserve order.
 *
 * @param inv Pointer to Inventory struct.
 */
void delete_book(Inventory *inv) {
    if (inv == NULL || inv->count == 0) {
        printf("\n[INFO] Inventory is currently empty. Nothing to delete.\n");
        return;
    }

    printf("\n-------------------------------------------------------\n");
    printf("                  DELETE BOOK RECORD                   \n");
    printf("-------------------------------------------------------\n");
    printf("Enter Book ID to delete: ");

    char search_id[MAX_ID_LEN];
    if (fgets(search_id, sizeof(search_id), stdin) == NULL) return;
    remove_newline(search_id);

    int idx = find_book_index_by_id(inv, search_id);
    if (idx == -1) {
        printf("[ERROR] Book with ID '%s' was not found in inventory.\n", search_id);
        return;
    }

    Book *b = &inv->books[idx];
    printf("\nRecord to delete:\n");
    printf("  Book ID : %s\n", b->id);
    printf("  Title   : %s\n", b->title);
    printf("  Author  : %s\n", b->author);
    printf("  Copies  : %d\n", b->copies);

    printf("\nAre you sure you want to permanently delete this book? (y/n): ");
    char confirm[10];
    if (fgets(confirm, sizeof(confirm), stdin) != NULL) {
        remove_newline(confirm);
        if (confirm[0] != 'y' && confirm[0] != 'Y') {
            printf("Deletion cancelled. Book record preserved.\n");
            return;
        }
    }

    // Shift elements left to remove element at idx
    for (int i = idx; i < inv->count - 1; i++) {
        inv->books[i] = inv->books[i + 1];
    }

    inv->count--;
    printf("\n[SUCCESS] Book ID '%s' has been deleted from inventory.\n", search_id);

    // Save updated inventory to binary file
    save_to_bin(inv, "library_data.bin");
}

#include <ctype.h>

/**
 * Case-insensitive substring search helper.
 * Checks if 'needle' exists inside 'haystack', ignoring character case.
 *
 * @param haystack Full string to search within.
 * @param needle   Substring pattern to look for.
 * @return 1 if needle is found inside haystack, 0 otherwise.
 */
int str_case_contains(const char *haystack, const char *needle) {
    if (haystack == NULL || needle == NULL) return 0;
    if (strlen(needle) == 0) return 1;

    size_t h_len = strlen(haystack);
    size_t n_len = strlen(needle);

    if (n_len > h_len) return 0;

    for (size_t i = 0; i <= h_len - n_len; i++) {
        size_t j = 0;
        while (j < n_len && tolower((unsigned char)haystack[i + j]) == tolower((unsigned char)needle[j])) {
            j++;
        }
        if (j == n_len) return 1; // Match found!
    }

    return 0;
}

/* =========================================================================
 * SEARCH & SORT IMPLEMENTATIONS (PHASE 4)
 * ========================================================================= */

/**
 * Searches inventory for a book by its Book ID (case-insensitive).
 *
 * @param inv Pointer to Inventory struct.
 */
void search_by_id(const Inventory *inv) {
    if (inv == NULL || inv->count == 0) {
        printf("\n[INFO] Inventory is currently empty. Nothing to search.\n");
        return;
    }

    printf("\n-------------------------------------------------------\n");
    printf("                  SEARCH BOOK BY ID                    \n");
    printf("-------------------------------------------------------\n");
    printf("Enter Book ID to search: ");

    char search_id[MAX_ID_LEN];
    if (fgets(search_id, sizeof(search_id), stdin) == NULL) return;
    remove_newline(search_id);

    if (strlen(search_id) == 0) {
        printf("[ERROR] Search query cannot be empty.\n");
        return;
    }

    int matches_found = 0;
    for (int i = 0; i < inv->count; i++) {
        if (str_case_contains(inv->books[i].id, search_id)) {
            const Book *b = &inv->books[i];
            printf("\nMatch Found [%d]:\n", ++matches_found);
            printf("  Book ID  : %s\n", b->id);
            printf("  Title    : %s\n", b->title);
            printf("  Author   : %s\n", b->author);
            printf("  Category : %s\n", b->category);
            printf("  Copies   : %d\n", b->copies);
        }
    }

    if (matches_found == 0) {
        printf("\n[NO MATCHES] No book found matching ID '%s'.\n", search_id);
    } else {
        printf("\nFound %d matching record(s).\n", matches_found);
    }
}

/**
 * Searches inventory for books matching a Title query (case-insensitive partial match).
 *
 * @param inv Pointer to Inventory struct.
 */
void search_by_title(const Inventory *inv) {
    if (inv == NULL || inv->count == 0) {
        printf("\n[INFO] Inventory is currently empty. Nothing to search.\n");
        return;
    }

    printf("\n-------------------------------------------------------\n");
    printf("                SEARCH BOOK BY TITLE                   \n");
    printf("-------------------------------------------------------\n");
    printf("Enter Title keyword or substring: ");

    char search_term[MAX_TITLE_LEN];
    if (fgets(search_term, sizeof(search_term), stdin) == NULL) return;
    remove_newline(search_term);

    if (strlen(search_term) == 0) {
        printf("[ERROR] Search term cannot be empty.\n");
        return;
    }

    int matches[100];
    int match_count = 0;

    for (int i = 0; i < inv->count; i++) {
        if (str_case_contains(inv->books[i].title, search_term)) {
            matches[match_count++] = i;
        }
    }

    if (match_count == 0) {
        printf("\n[NO MATCHES] No books found containing '%s' in their title.\n", search_term);
        return;
    }

    printf("\n=========================================================================================\n");
    printf("                  SEARCH RESULTS FOR TITLE KEYWORD '%s'                  \n", search_term);
    printf("=========================================================================================\n");
    printf(" %-3s | %-8s | %-28s | %-20s | %-12s | %-6s\n",
           "#", "Book ID", "Title", "Author", "Category", "Copies");
    printf("-----+----------+------------------------------+----------------------+--------------+--------\n");

    for (int m = 0; m < match_count; m++) {
        int idx = matches[m];
        const Book *b = &inv->books[idx];
        
        char title_disp[29], author_disp[21], cat_disp[13];
        snprintf(title_disp, sizeof(title_disp), "%s", b->title);
        snprintf(author_disp, sizeof(author_disp), "%s", b->author);
        snprintf(cat_disp, sizeof(cat_disp), "%s", b->category);

        printf(" %-3d | %-8s | %-28s | %-20s | %-12s | %-6d\n",
               m + 1, b->id, title_disp, author_disp, cat_disp, b->copies);
    }

    printf("=========================================================================================\n");
    printf(" Found %d matching book(s).\n", match_count);
    printf("=========================================================================================\n");
}

/**
 * Sorts inventory records manually (Selection Sort algorithm).
 * Offers choice to sort by Book ID, Title, or Available Copies.
 *
 * @param inv Pointer to Inventory struct.
 */
void sort_inventory(Inventory *inv) {
    if (inv == NULL || inv->count <= 1) {
        printf("\n[INFO] Need at least 2 books in inventory to perform sorting.\n");
        return;
    }

    printf("\n-------------------------------------------------------\n");
    printf("                   SORT INVENTORY                      \n");
    printf("-------------------------------------------------------\n");
    printf("Select sorting criteria:\n");
    printf("  1. Book ID (Alphabetical A-Z)\n");
    printf("  2. Title (Alphabetical A-Z)\n");
    printf("  3. Number of Copies (Ascending: Low to High)\n");
    printf("  4. Number of Copies (Descending: High to Low)\n");
    printf("  0. Cancel\n");
    printf("Enter choice (0-4): ");

    int sort_choice = -1;
    if (scanf("%d", &sort_choice) != 1) {
        printf("[ERROR] Invalid choice format. Sorting cancelled.\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    if (sort_choice == 0) {
        printf("Sorting cancelled.\n");
        return;
    }

    // Selection Sort Algorithm Implementation
    for (int i = 0; i < inv->count - 1; i++) {
        int target_idx = i;

        for (int j = i + 1; j < inv->count; j++) {
            int should_swap = 0;

            switch (sort_choice) {
                case 1: // Book ID (Alphabetical)
                    if (strcmp(inv->books[j].id, inv->books[target_idx].id) < 0) {
                        should_swap = 1;
                    }
                    break;
                case 2: // Title (Alphabetical)
                    if (strcmp(inv->books[j].title, inv->books[target_idx].title) < 0) {
                        should_swap = 1;
                    }
                    break;
                case 3: // Copies (Ascending)
                    if (inv->books[j].copies < inv->books[target_idx].copies) {
                        should_swap = 1;
                    }
                    break;
                case 4: // Copies (Descending)
                    if (inv->books[j].copies > inv->books[target_idx].copies) {
                        should_swap = 1;
                    }
                    break;
                default:
                    printf("[ERROR] Invalid sort option selected.\n");
                    return;
            }

            if (should_swap) {
                target_idx = j;
            }
        }

        // Swap elements at i and target_idx
        if (target_idx != i) {
            Book temp = inv->books[i];
            inv->books[i] = inv->books[target_idx];
            inv->books[target_idx] = temp;
        }
    }

    printf("\n[SUCCESS] Inventory sorted successfully!\n");
    display_books(inv);

    // Save sorted order to binary file
    save_to_bin(inv, "library_data.bin");
}

/**
 * Generates and prints comprehensive inventory analytics reports including:
 * 1. Total distinct books & aggregate copy count.
 * 2. Book(s) with the highest number of copies (handles ties).
 * 3. Category breakdown tally (unique titles & copies per category).
 *
 * @param inv Pointer to Inventory struct.
 */
void generate_reports(const Inventory *inv) {
    if (inv == NULL || inv->count == 0) {
        printf("\n[INFO] Inventory is currently empty. No data available to generate reports.\n");
        return;
    }

    printf("\n=========================================================================================\n");
    printf("                             INVENTORY ANALYTICS REPORT                                  \n");
    printf("=========================================================================================\n");

    // 1. Calculate Summary Statistics
    int total_books = inv->count;
    int total_copies = 0;
    int max_copies = inv->books[0].copies;

    for (int i = 0; i < inv->count; i++) {
        total_copies += inv->books[i].copies;
        if (inv->books[i].copies > max_copies) {
            max_copies = inv->books[i].copies;
        }
    }

    printf(" 1. SUMMARY STATISTICS\n");
    printf("    ---------------------------------------------------\n");
    printf("    - Total Distinct Book Titles : %d\n", total_books);
    printf("    - Total Available Copies     : %d\n", total_copies);
    printf("    - Average Copies per Book    : %.2f\n", (double)total_copies / total_books);

    // 2. Report Book(s) with Highest Copy Count
    printf("\n 2. BOOK(S) WITH HIGHEST NUMBER OF COPIES (%d copies)\n", max_copies);
    printf("    ---------------------------------------------------\n");
    for (int i = 0; i < inv->count; i++) {
        if (inv->books[i].copies == max_copies) {
            printf("    * ID: %-8s | Title: %-30s | Category: %s\n",
                   inv->books[i].id, inv->books[i].title, inv->books[i].category);
        }
    }

    // 3. Category Breakdown & Tally
    printf("\n 3. CATEGORY BREAKDOWN & DISTRIBUTION\n");
    printf("    ---------------------------------------------------\n");
    
    char categories[100][MAX_CATEGORY_LEN];
    int cat_counts[100] = {0};
    int cat_copies[100] = {0};
    int num_categories = 0;

    for (int i = 0; i < inv->count; i++) {
        const char *cat = inv->books[i].category;
        int found_idx = -1;

        // Check if category already tracked
        for (int c = 0; c < num_categories; c++) {
            if (strcmp(categories[c], cat) == 0) {
                found_idx = c;
                break;
            }
        }

        if (found_idx != -1) {
            cat_counts[found_idx]++;
            cat_copies[found_idx] += inv->books[i].copies;
        } else {
            if (num_categories < 100) {
                strcpy(categories[num_categories], cat);
                cat_counts[num_categories] = 1;
                cat_copies[num_categories] = inv->books[i].copies;
                num_categories++;
            }
        }
    }

    printf("    %-20s | %-14s | %-12s\n", "Category Name", "Unique Titles", "Total Copies");
    printf("    ---------------------+----------------+-------------\n");
    for (int c = 0; c < num_categories; c++) {
        printf("    %-20s | %-14d | %-12d\n", categories[c], cat_counts[c], cat_copies[c]);
    }

    printf("=========================================================================================\n");
}

