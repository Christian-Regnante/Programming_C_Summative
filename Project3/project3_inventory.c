#define _CRT_SECURE_NO_WARNINGS
#include "project3_inventory.h"
#include <ctype.h>

// --- UTILITY FUNCTIONS ---

void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void remove_newline(char *str) {
    if (!str) return;
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') str[len - 1] = '\0';
    if (len > 1 && str[len - 2] == '\r') str[len - 2] = '\0';
}

int find_book_index_by_id(const Inventory *inv, const char *id) {
    if (!inv || !id) return -1;
    for (int i = 0; i < inv->count; i++) {
        if (strcasecmp(inv->books[i].id, id) == 0) return i;
    }
    return -1;
}

int str_case_contains(const char *haystack, const char *needle) {
    if (!haystack || !needle) return 0;
    char h_lower[256] = {0}, n_lower[256] = {0};
    for (int i = 0; haystack[i] && i < 255; i++) h_lower[i] = (char)tolower((unsigned char)haystack[i]);
    for (int i = 0; needle[i] && i < 255; i++) n_lower[i] = (char)tolower((unsigned char)needle[i]);
    return strstr(h_lower, n_lower) != NULL;
}

// --- MEMORY MANAGEMENT ---

int init_inventory(Inventory *inv) {
    if (!inv) return 0;
    inv->capacity = INITIAL_CAPACITY;
    inv->count = 0;
    inv->books = (Book *)malloc(inv->capacity * sizeof(Book));
    return inv->books != NULL;
}

int resize_inventory(Inventory *inv) {
    if (!inv) return 0;
    int new_cap = inv->capacity * 2;
    Book *temp = (Book *)realloc(inv->books, new_cap * sizeof(Book));
    if (!temp) return 0;
    inv->books = temp;
    inv->capacity = new_cap;
    return 1;
}

void free_inventory(Inventory *inv) {
    if (inv && inv->books) {
        free(inv->books);
        inv->books = NULL;
        inv->count = 0;
        inv->capacity = 0;
    }
}

// --- FILE I/O ---

int save_to_bin(const Inventory *inv, const char *filename) {
    if (!inv || !filename) return 0;
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        printf("[ERROR] Failed to open binary file for writing.\n");
        return 0;
    }
    fwrite(&inv->count, sizeof(int), 1, fp);
    if (inv->count > 0) {
        fwrite(inv->books, sizeof(Book), inv->count, fp);
    }
    fclose(fp);
    printf("[SUCCESS] Saved %d records to '%s'.\n", inv->count, filename);
    return 1;
}

int load_from_bin(Inventory *inv, const char *filename) {
    if (!inv || !filename) return 0;
    FILE *fp = fopen(filename, "rb");
    if (!fp) return 0;

    int loaded_count = 0;
    if (fread(&loaded_count, sizeof(int), 1, fp) != 1 || loaded_count <= 0) {
        fclose(fp);
        return 0;
    }

    inv->count = 0;
    for (int i = 0; i < loaded_count; i++) {
        if (inv->count >= inv->capacity) resize_inventory(inv);
        if (fread(&inv->books[inv->count], sizeof(Book), 1, fp) == 1) {
            inv->count++;
        }
    }
    fclose(fp);
    printf("[SUCCESS] Loaded %d book records from '%s'.\n", inv->count, filename);
    return 1;
}

int import_from_txt(Inventory *inv, const char *filename) {
    if (!inv || !filename) return 0;
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("[ERROR] Text file '%s' not found.\n", filename);
        return 0;
    }

    char line[256];
    Book b = {0};
    int imported = 0, field_count = 0;

    while (fgets(line, sizeof(line), fp)) {
        remove_newline(line);
        if (strlen(line) == 0) continue;

        char *val = strchr(line, ':');
        if (val) {
            val++;
            while (*val == ' ' || *val == '\t') val++;
        } else {
            val = line;
        }

        if (strstr(line, "Book ID") || field_count == 0) {
            if (field_count >= 4 && strlen(b.id) > 0) {
                if (find_book_index_by_id(inv, b.id) == -1) {
                    if (inv->count >= inv->capacity) resize_inventory(inv);
                    inv->books[inv->count++] = b;
                    imported++;
                }
                memset(&b, 0, sizeof(Book));
            }
            strncpy(b.id, val, MAX_ID_LEN - 1);
            field_count = 1;
        } else if (strstr(line, "Title") || field_count == 1) {
            strncpy(b.title, val, MAX_TITLE_LEN - 1);
            field_count = 2;
        } else if (strstr(line, "Author") || field_count == 2) {
            strncpy(b.author, val, MAX_AUTHOR_LEN - 1);
            field_count = 3;
        } else if (strstr(line, "Category") || field_count == 3) {
            strncpy(b.category, val, MAX_CATEGORY_LEN - 1);
            field_count = 4;
        } else if (strstr(line, "copies") || strstr(line, "Copies") || field_count == 4) {
            b.copies = atoi(val);
            field_count = 5;
            if (find_book_index_by_id(inv, b.id) == -1) {
                if (inv->count >= inv->capacity) resize_inventory(inv);
                inv->books[inv->count++] = b;
                imported++;
            }
            memset(&b, 0, sizeof(Book));
            field_count = 0;
        }
    }

    if (field_count >= 4 && strlen(b.id) > 0 && find_book_index_by_id(inv, b.id) == -1) {
        if (inv->count >= inv->capacity) resize_inventory(inv);
        inv->books[inv->count++] = b;
        imported++;
    }

    fclose(fp);
    printf("[SUCCESS] Imported %d records from '%s'. Total: %d\n", imported, filename, inv->count);
    return 1;
}

// --- CRUD OPERATIONS ---

void display_books(const Inventory *inv) {
    if (!inv || inv->count == 0) {
        printf("\n[INFO] Inventory is currently empty.\n");
        return;
    }

    printf("\n=========================================================================================================\n");
    printf(" %-8s | %-30s | %-25s | %-15s | %-6s\n", "ID", "TITLE", "AUTHOR", "CATEGORY", "COPIES");
    printf("=========================================================================================================\n");
    for (int i = 0; i < inv->count; i++) {
        printf(" %-8s | %-30.30s | %-25.25s | %-15.15s | %-6d\n",
               inv->books[i].id, inv->books[i].title, inv->books[i].author,
               strlen(inv->books[i].category) > 0 ? inv->books[i].category : "General",
               inv->books[i].copies);
    }
    printf("=========================================================================================================\n");
    printf(" Total Records: %d | Capacity: %d\n", inv->count, inv->capacity);
}

void add_book(Inventory *inv) {
    if (!inv) return;
    Book b = {0};

    printf("\n--- ADD NEW BOOK RECORD ---\n");
    printf("Enter Book ID: ");
    if (fgets(b.id, sizeof(b.id), stdin)) remove_newline(b.id);
    if (strlen(b.id) == 0) { printf("[ERROR] ID cannot be empty.\n"); return; }
    if (find_book_index_by_id(inv, b.id) != -1) { printf("[ERROR] Book ID '%s' already exists.\n", b.id); return; }

    printf("Enter Title: ");
    if (fgets(b.title, sizeof(b.title), stdin)) remove_newline(b.title);

    printf("Enter Author: ");
    if (fgets(b.author, sizeof(b.author), stdin)) remove_newline(b.author);

    printf("Enter Category: ");
    if (fgets(b.category, sizeof(b.category), stdin)) remove_newline(b.category);

    printf("Enter Available Copies: ");
    if (scanf("%d", &b.copies) != 1 || b.copies < 0) {
        printf("[ERROR] Invalid number of copies.\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    if (inv->count >= inv->capacity) resize_inventory(inv);
    inv->books[inv->count++] = b;
    printf("[SUCCESS] Added '%s' (ID: %s) to inventory.\n", b.title, b.id);
}

void update_book(Inventory *inv) {
    if (!inv || inv->count == 0) { printf("\n[INFO] Inventory is empty.\n"); return; }

    char id[MAX_ID_LEN];
    printf("\nEnter Book ID to update: ");
    if (fgets(id, sizeof(id), stdin)) remove_newline(id);

    int idx = find_book_index_by_id(inv, id);
    if (idx == -1) { printf("[ERROR] Book ID '%s' not found.\n", id); return; }

    Book *b = &inv->books[idx];
    printf("Updating '%s' (Leave blank to keep current value):\n", b->title);

    char buf[MAX_TITLE_LEN];
    printf("New Title [%s]: ", b->title);
    if (fgets(buf, sizeof(buf), stdin) && strlen(buf) > 1) { remove_newline(buf); strcpy(b->title, buf); }

    printf("New Author [%s]: ", b->author);
    if (fgets(buf, sizeof(buf), stdin) && strlen(buf) > 1) { remove_newline(buf); strcpy(b->author, buf); }

    printf("New Category [%s]: ", b->category);
    if (fgets(buf, sizeof(buf), stdin) && strlen(buf) > 1) { remove_newline(buf); strcpy(b->category, buf); }

    printf("New Copies [%d]: ", b->copies);
    if (fgets(buf, sizeof(buf), stdin) && strlen(buf) > 1) {
        int new_copies = atoi(buf);
        if (new_copies >= 0) b->copies = new_copies;
    }
    printf("[SUCCESS] Updated book record ID '%s'.\n", id);
}

void delete_book(Inventory *inv) {
    if (!inv || inv->count == 0) { printf("\n[INFO] Inventory is empty.\n"); return; }

    char id[MAX_ID_LEN];
    printf("\nEnter Book ID to delete: ");
    if (fgets(id, sizeof(id), stdin)) remove_newline(id);

    int idx = find_book_index_by_id(inv, id);
    if (idx == -1) { printf("[ERROR] Book ID '%s' not found.\n", id); return; }

    char title_deleted[MAX_TITLE_LEN];
    strcpy(title_deleted, inv->books[idx].title);

    for (int i = idx; i < inv->count - 1; i++) {
        inv->books[i] = inv->books[i + 1];
    }
    inv->count--;
    printf("[SUCCESS] Deleted '%s' (ID: %s) from inventory.\n", title_deleted, id);
}

// --- SEARCH & SORT ---

void search_by_id(const Inventory *inv) {
    if (!inv || inv->count == 0) { printf("\n[INFO] Inventory is empty.\n"); return; }
    char id[MAX_ID_LEN];
    printf("\nEnter Book ID to search: ");
    if (fgets(id, sizeof(id), stdin)) remove_newline(id);

    int idx = find_book_index_by_id(inv, id);
    if (idx == -1) {
        printf("[RESULT] No book found with ID '%s'.\n", id);
    } else {
        printf("\n[MATCH FOUND]\nID: %s | Title: %s | Author: %s | Category: %s | Copies: %d\n",
               inv->books[idx].id, inv->books[idx].title, inv->books[idx].author,
               inv->books[idx].category, inv->books[idx].copies);
    }
}

void search_by_title(const Inventory *inv) {
    if (!inv || inv->count == 0) { printf("\n[INFO] Inventory is empty.\n"); return; }
    char keyword[MAX_TITLE_LEN];
    printf("\nEnter Title keyword to search: ");
    if (fgets(keyword, sizeof(keyword), stdin)) remove_newline(keyword);

    int matches = 0;
    printf("\n--- SEARCH RESULTS FOR '%s' ---\n", keyword);
    for (int i = 0; i < inv->count; i++) {
        if (str_case_contains(inv->books[i].title, keyword)) {
            printf(" ID: %-6s | Title: %-25s | Author: %-20s | Copies: %d\n",
                   inv->books[i].id, inv->books[i].title, inv->books[i].author, inv->books[i].copies);
            matches++;
        }
    }
    if (matches == 0) printf("No matching books found.\n");
}

void sort_inventory(Inventory *inv) {
    if (!inv || inv->count <= 1) { printf("\n[INFO] Not enough books to sort.\n"); return; }

    printf("\nSort Inventory By:\n 1. Book ID\n 2. Title\n 3. Available Copies\nEnter choice (1-3): ");
    int choice = 0;
    if (scanf("%d", &choice) != 1) { clear_input_buffer(); return; }
    clear_input_buffer();

    for (int i = 0; i < inv->count - 1; i++) {
        for (int j = i + 1; j < inv->count; j++) {
            int swap = 0;
            if (choice == 1 && strcasecmp(inv->books[i].id, inv->books[j].id) > 0) swap = 1;
            else if (choice == 2 && strcasecmp(inv->books[i].title, inv->books[j].title) > 0) swap = 1;
            else if (choice == 3 && inv->books[i].copies < inv->books[j].copies) swap = 1;

            if (swap) {
                Book temp = inv->books[i];
                inv->books[i] = inv->books[j];
                inv->books[j] = temp;
            }
        }
    }
    printf("[SUCCESS] Inventory sorted successfully.\n");
    display_books(inv);
}

// --- ANALYTICS ---

void generate_reports(const Inventory *inv) {
    if (!inv || inv->count == 0) { printf("\n[INFO] Inventory is empty.\n"); return; }

    int total_copies = 0;
    int max_copies = -1, min_copies = 999999;
    int max_idx = 0, min_idx = 0;

    for (int i = 0; i < inv->count; i++) {
        total_copies += inv->books[i].copies;
        if (inv->books[i].copies > max_copies) { max_copies = inv->books[i].copies; max_idx = i; }
        if (inv->books[i].copies < min_copies) { min_copies = inv->books[i].copies; min_idx = i; }
    }

    printf("\n=======================================================\n");
    printf("             INVENTORY ANALYTICS REPORT                \n");
    printf("=======================================================\n");
    printf(" Total Unique Titles  : %d\n", inv->count);
    printf(" Total Available Copies: %d\n", total_copies);
    printf(" Average Copies/Title : %.2f\n", (double)total_copies / inv->count);
    printf(" Most Stocked Book    : '%s' (%d copies)\n", inv->books[max_idx].title, max_copies);
    printf(" Lowest Stocked Book  : '%s' (%d copies)\n", inv->books[min_idx].title, min_copies);
    printf("=======================================================\n");
}
