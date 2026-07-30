#ifndef PROJECT3_INVENTORY_H
#define PROJECT3_INVENTORY_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initial capacity for dynamic memory array
#define INITIAL_CAPACITY 10

// Maximum string lengths
#define MAX_ID_LEN 20
#define MAX_TITLE_LEN 100
#define MAX_AUTHOR_LEN 100
#define MAX_CATEGORY_LEN 50

/**
 * Structure representing a single Book record.
 * Uses fixed-size character arrays for safe binary serialization.
 */
typedef struct {
    char id[MAX_ID_LEN];
    char title[MAX_TITLE_LEN];
    char author[MAX_AUTHOR_LEN];
    char category[MAX_CATEGORY_LEN];
    int copies;
} Book;

/**
 * Structure representing the dynamic Inventory state.
 * Manages dynamically allocated Book array, current count, and total capacity.
 */
typedef struct {
    Book *books;
    int count;
    int capacity;
} Inventory;

/* =========================================================================
 * FUNCTION PROTOTYPES
 * ========================================================================= */

// --- Memory Management ---
int init_inventory(Inventory *inv);
int resize_inventory(Inventory *inv);
void free_inventory(Inventory *inv);

// --- File I/O (Implemented in Phase 2) ---
int save_to_bin(const Inventory *inv, const char *filename);
int load_from_bin(Inventory *inv, const char *filename);
int import_from_txt(Inventory *inv, const char *filename);

// --- CRUD Operations (Implemented in Phase 3) ---
void add_book(Inventory *inv);
void display_books(const Inventory *inv);
void update_book(Inventory *inv);
void delete_book(Inventory *inv);

// --- Search & Sort Operations (Implemented in Phase 4) ---
void search_by_id(const Inventory *inv);
void search_by_title(const Inventory *inv);
void sort_inventory(Inventory *inv);

// --- Analytics & Reports (Implemented in Phase 5) ---
void generate_reports(const Inventory *inv);

// --- Utility Functions ---
void clear_input_buffer(void);
void remove_newline(char *str);
int find_book_index_by_id(const Inventory *inv, const char *id);
int str_case_contains(const char *haystack, const char *needle);

#endif // PROJECT3_INVENTORY_H
