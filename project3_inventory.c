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

/* =========================================================================
 * FEATURE STUBS (Implemented in later phases)
 * ========================================================================= */

int save_to_bin(const Inventory *inv, const char *filename) {
    (void)inv; (void)filename;
    printf("\n[STUB] Feature 'Save to Binary File' will be implemented in Phase 2.\n");
    return 1;
}

int load_from_bin(Inventory *inv, const char *filename) {
    (void)inv; (void)filename;
    printf("\n[STUB] Feature 'Load from Binary File' will be implemented in Phase 2.\n");
    return 1;
}

int import_from_txt(Inventory *inv, const char *filename) {
    (void)inv; (void)filename;
    printf("\n[STUB] Feature 'Import from Text File' will be implemented in Phase 2.\n");
    return 1;
}

void add_book(Inventory *inv) {
    (void)inv;
    printf("\n[STUB] Feature 'Add Book' will be implemented in Phase 3.\n");
}

void display_books(const Inventory *inv) {
    if (inv == NULL || inv->count == 0) {
        printf("\n=======================================================\n");
        printf("                LIBRARY BOOK INVENTORY                 \n");
        printf("=======================================================\n");
        printf("No books currently in inventory.\n");
        printf("=======================================================\n");
    } else {
        printf("\n[STUB] Full table display will be implemented in Phase 3.\n");
        printf("Currently %d book(s) in memory.\n", inv->count);
    }
}

void update_book(Inventory *inv) {
    (void)inv;
    printf("\n[STUB] Feature 'Update Book' will be implemented in Phase 3.\n");
}

void delete_book(Inventory *inv) {
    (void)inv;
    printf("\n[STUB] Feature 'Delete Book' will be implemented in Phase 3.\n");
}

void search_by_id(const Inventory *inv) {
    (void)inv;
    printf("\n[STUB] Feature 'Search by Book ID' will be implemented in Phase 4.\n");
}

void search_by_title(const Inventory *inv) {
    (void)inv;
    printf("\n[STUB] Feature 'Search by Title' will be implemented in Phase 4.\n");
}

void sort_inventory(Inventory *inv) {
    (void)inv;
    printf("\n[STUB] Feature 'Sort Inventory' will be implemented in Phase 4.\n");
}

void generate_reports(const Inventory *inv) {
    (void)inv;
    printf("\n[STUB] Feature 'Inventory Reports' will be implemented in Phase 5.\n");
}
