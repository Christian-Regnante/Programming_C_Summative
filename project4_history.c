#include "project4_history.h"
#include <math.h>

/* =========================================================================
 * MEMORY MANAGEMENT FUNCTIONS
 * ========================================================================= */

/**
 * Initializes the dynamic history manager.
 *
 * @param hm Pointer to HistoryManager struct.
 * @return 1 on success, 0 on memory allocation failure.
 */
int init_history(HistoryManager *hm) {
    if (hm == NULL) return 0;

    hm->capacity = INITIAL_HISTORY_CAPACITY;
    hm->count = 0;
    hm->records = (Record *)malloc(hm->capacity * sizeof(Record));

    if (hm->records == NULL) {
        printf("[ERROR] Memory allocation failed during history initialization!\n");
        return 0;
    }

    return 1;
}

/**
 * Resizes the dynamic history array when capacity limit is reached.
 * Doubles current capacity using realloc with a temporary pointer.
 *
 * @param hm Pointer to HistoryManager struct.
 * @return 1 on success, 0 on realloc failure.
 */
int resize_history(HistoryManager *hm) {
    if (hm == NULL) return 0;

    int new_capacity = hm->capacity * 2;
    Record *temp = (Record *)realloc(hm->records, new_capacity * sizeof(Record));

    if (temp == NULL) {
        printf("[ERROR] Memory reallocation failed! History capacity remains %d.\n", hm->capacity);
        return 0;
    }

    hm->records = temp;
    hm->capacity = new_capacity;
    printf("[DEBUG] History memory expanded to capacity: %d\n", hm->capacity);
    return 1;
}

/**
 * Frees dynamic history memory array.
 *
 * @param hm Pointer to HistoryManager struct.
 */
void free_history(HistoryManager *hm) {
    if (hm != NULL && hm->records != NULL) {
        free(hm->records);
        hm->records = NULL;
        hm->count = 0;
        hm->capacity = 0;
        printf("[DEBUG] History dynamic memory freed successfully.\n");
    }
}

/* =========================================================================
 * UTILITY & LOGGING FUNCTIONS
 * ========================================================================= */

/**
 * Generates current system date-time timestamp string.
 *
 * @param buffer Output buffer to receive formatted timestamp string.
 * @param size   Buffer capacity.
 */
void get_current_timestamp(char *buffer, size_t size) {
    if (buffer == NULL || size == 0) return;

    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    if (timeinfo != NULL) {
        strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeinfo);
    } else {
        snprintf(buffer, size, "UNKNOWN");
    }
}

/**
 * Adds a new conversion record to dynamic history.
 * Expands memory array if full.
 *
 * @param hm        Pointer to HistoryManager struct.
 * @param name      Name of conversion (e.g. "Kilometers to Miles").
 * @param from_unit Source unit label.
 * @param to_unit   Target unit label.
 * @param in_val    Input numeric value.
 * @param out_val   Converted numeric result.
 * @return 1 on success, 0 on failure.
 */
int add_record(HistoryManager *hm, const char *name, const char *from_unit,
               const char *to_unit, double in_val, double out_val) {
    if (hm == NULL || name == NULL || from_unit == NULL || to_unit == NULL) return 0;

    // Check capacity & expand if full
    if (hm->count >= hm->capacity) {
        if (!resize_history(hm)) {
            printf("[ERROR] Cannot log conversion history record due to memory limits!\n");
            return 0;
        }
    }

    Record *r = &hm->records[hm->count];
    memset(r, 0, sizeof(Record));

    r->id = hm->count + 1;
    get_current_timestamp(r->timestamp, sizeof(r->timestamp));
    strncpy(r->conversion_name, name, sizeof(r->conversion_name) - 1);
    strncpy(r->from_unit, from_unit, sizeof(r->from_unit) - 1);
    strncpy(r->to_unit, to_unit, sizeof(r->to_unit) - 1);
    r->input_value = in_val;
    r->output_value = out_val;

    hm->count++;
    return 1;
}

/**
 * Displays all recorded conversion records in a formatted ASCII table layout.
 *
 * @param hm Pointer to HistoryManager struct.
 */
void view_history(const HistoryManager *hm) {
    if (hm == NULL || hm->count == 0) {
        printf("\n=========================================================================================\n");
        printf("                                CONVERSION HISTORY LOG                                   \n");
        printf("=========================================================================================\n");
        printf(" No conversion records recorded in history yet.\n");
        printf("=========================================================================================\n");
        return;
    }

    printf("\n=========================================================================================\n");
    printf("                                CONVERSION HISTORY LOG                                   \n");
    printf("=========================================================================================\n");
    printf(" %-3s | %-19s | %-24s | %-16s | %-16s\n",
           "#", "Timestamp", "Conversion Type", "Input Value", "Converted Result");
    printf("-----+---------------------+--------------------------+------------------+-------------------\n");

    for (int i = 0; i < hm->count; i++) {
        const Record *r = &hm->records[i];
        
        char in_str[30], out_str[30];
        snprintf(in_str, sizeof(in_str), "%.4f %s", r->input_value, r->from_unit);
        snprintf(out_str, sizeof(out_str), "%.4f %s", r->output_value, r->to_unit);

        printf(" %-3d | %-19s | %-24s | %-16s | %-16s\n",
               r->id, r->timestamp, r->conversion_name, in_str, out_str);
    }

    printf("=========================================================================================\n");
    printf(" Total Recorded Conversions: %d | Dynamic Memory Capacity: %d\n", hm->count, hm->capacity);
    printf("=========================================================================================\n");
}

/* =========================================================================
 * FILE I/O IMPLEMENTATIONS (PHASE 3)
 * ========================================================================= */

/**
 * Saves conversion history to a binary file (.bin).
 *
 * @param hm       Pointer to HistoryManager struct.
 * @param filename Name of binary file.
 * @return 1 on success, 0 on failure.
 */
int save_history_bin(const HistoryManager *hm, const char *filename) {
    if (hm == NULL || filename == NULL) return 0;

    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("[ERROR] Could not open file '%s' for binary saving.\n", filename);
        return 0;
    }

    // Write record count first
    if (fwrite(&(hm->count), sizeof(int), 1, fp) != 1) {
        printf("[ERROR] Failed to write history count to binary file '%s'.\n", filename);
        fclose(fp);
        return 0;
    }

    // Write array of Record structs
    if (hm->count > 0) {
        size_t written = fwrite(hm->records, sizeof(Record), (size_t)hm->count, fp);
        if (written != (size_t)hm->count) {
            printf("[ERROR] Wrote %zu of %d expected records to '%s'.\n", written, hm->count, filename);
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    printf("[SUCCESS] Saved %d conversion record(s) to binary file '%s'.\n", hm->count, filename);
    return 1;
}

/**
 * Loads conversion history from a binary file (.bin).
 *
 * @param hm       Pointer to HistoryManager struct.
 * @param filename Name of binary file.
 * @return 1 if loaded successfully, 0 if file does not exist or failed to load.
 */
int load_history_bin(HistoryManager *hm, const char *filename) {
    if (hm == NULL || filename == NULL) return 0;

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        // File does not exist yet (normal on first launch)
        return 0;
    }

    int saved_count = 0;
    if (fread(&saved_count, sizeof(int), 1, fp) != 1) {
        printf("[WARNING] Binary history file '%s' is empty or invalid.\n", filename);
        fclose(fp);
        return 0;
    }

    if (saved_count < 0) {
        printf("[ERROR] Invalid record count (%d) in '%s'.\n", saved_count, filename);
        fclose(fp);
        return 0;
    }

    // Expand dynamic array capacity if needed
    while (hm->capacity < saved_count) {
        if (!resize_history(hm)) {
            printf("[ERROR] Failed to expand memory while loading '%s'.\n", filename);
            fclose(fp);
            return 0;
        }
    }

    if (saved_count > 0) {
        size_t read_count = fread(hm->records, sizeof(Record), (size_t)saved_count, fp);
        if (read_count != (size_t)saved_count) {
            printf("[WARNING] Read %zu of %d expected records from '%s'.\n", read_count, saved_count, filename);
            hm->count = (int)read_count;
        } else {
            hm->count = saved_count;
        }
    }

    fclose(fp);
    printf("[SUCCESS] Loaded %d conversion record(s) from binary file '%s'.\n", hm->count, filename);
    return 1;
}

/* =========================================================================
 * CALLBACK ENGINE IMPLEMENTATIONS (PHASE 4)
 * ========================================================================= */

/**
 * Applies a processing callback function to every record in history.
 *
 * @param hm        Pointer to HistoryManager struct.
 * @param processor Pointer to RecordProcessor callback function.
 * @param param     Optional parameter passed to callback.
 */
void process_history_batch(HistoryManager *hm, RecordProcessor processor, void *param) {
    if (hm == NULL || processor == NULL || hm->count == 0) return;

    for (int i = 0; i < hm->count; i++) {
        processor(&hm->records[i], param);
    }
}

/**
 * Filters conversion records using a predicate callback function and displays matching records.
 *
 * @param hm        Pointer to HistoryManager struct.
 * @param predicate Pointer to RecordPredicate callback function.
 * @param criterion Filter criteria passed to predicate.
 */
void filter_history(const HistoryManager *hm, RecordPredicate predicate, const void *criterion) {
    if (hm == NULL || predicate == NULL || hm->count == 0) {
        printf("\n[INFO] History log is empty. Nothing to filter.\n");
        return;
    }

    printf("\n=========================================================================================\n");
    printf("                                FILTERED CONVERSION LOG                                  \n");
    printf("=========================================================================================\n");
    printf(" %-3s | %-19s | %-24s | %-16s | %-16s\n",
           "#", "Timestamp", "Conversion Type", "Input Value", "Converted Result");
    printf("-----+---------------------+--------------------------+------------------+-------------------\n");

    int match_count = 0;
    for (int i = 0; i < hm->count; i++) {
        const Record *r = &hm->records[i];
        if (predicate(r, criterion)) {
            match_count++;
            char in_str[30], out_str[30];
            snprintf(in_str, sizeof(in_str), "%.4f %s", r->input_value, r->from_unit);
            snprintf(out_str, sizeof(out_str), "%.4f %s", r->output_value, r->to_unit);

            printf(" %-3d | %-19s | %-24s | %-16s | %-16s\n",
                   r->id, r->timestamp, r->conversion_name, in_str, out_str);
        }
    }

    printf("=========================================================================================\n");
    printf(" Filter Matches Found: %d of %d total records.\n", match_count, hm->count);
    printf("=========================================================================================\n");
}

/**
 * Sorts conversion records in dynamic memory using a comparator callback function (Selection Sort).
 *
 * @param hm         Pointer to HistoryManager struct.
 * @param comparator Pointer to RecordComparator callback function.
 */
void sort_history(HistoryManager *hm, RecordComparator comparator) {
    if (hm == NULL || comparator == NULL || hm->count <= 1) {
        if (hm != NULL && hm->count <= 1) {
            printf("\n[INFO] Need at least 2 records to perform sorting.\n");
        }
        return;
    }

    // In-place Selection Sort using Comparator Callback
    for (int i = 0; i < hm->count - 1; i++) {
        int target_idx = i;
        for (int j = i + 1; j < hm->count; j++) {
            if (comparator(&hm->records[j], &hm->records[target_idx]) < 0) {
                target_idx = j;
            }
        }

        if (target_idx != i) {
            Record temp = hm->records[i];
            hm->records[i] = hm->records[target_idx];
            hm->records[target_idx] = temp;
        }
    }

    printf("\n[SUCCESS] Conversion history sorted using callback comparator!\n");
    view_history(hm);
    save_history_bin(hm, "unit_history.bin");
}

/* =========================================================================
 * CONCRETE CALLBACK FUNCTIONS
 * ========================================================================= */

/**
 * Processing Callback: Rounds input and output values to N decimal places.
 * param: Pointer to int specifying desired decimal places (e.g. 2, 4).
 */
void callback_round_precision(Record *rec, void *param) {
    if (rec == NULL || param == NULL) return;

    int decimals = *(int *)param;
    if (decimals < 0) decimals = 0;

    double factor = pow(10.0, decimals);
    rec->input_value = round(rec->input_value * factor) / factor;
    rec->output_value = round(rec->output_value * factor) / factor;
}

/**
 * Predicate Callback: Checks if converted output_value >= minimum threshold.
 * criterion: Pointer to double threshold value.
 */
int predicate_filter_by_min_val(const Record *rec, const void *criterion) {
    if (rec == NULL || criterion == NULL) return 0;
    double min_val = *(const double *)criterion;
    return rec->output_value >= min_val;
}

/**
 * Predicate Callback: Checks if conversion_name contains substring.
 * criterion: Pointer to null-terminated substring string.
 */
int predicate_filter_by_type(const Record *rec, const void *criterion) {
    if (rec == NULL || criterion == NULL) return 0;
    const char *keyword = (const char *)criterion;
    return (strstr(rec->conversion_name, keyword) != NULL);
}

/**
 * Comparator Callback: Compares records by conversion_name (Alphabetical A-Z).
 */
int compare_by_type(const Record *a, const Record *b) {
    if (a == NULL || b == NULL) return 0;
    return strcmp(a->conversion_name, b->conversion_name);
}

/**
 * Comparator Callback: Compares records by output_value (Ascending: Low to High).
 */
int compare_by_output_asc(const Record *a, const Record *b) {
    if (a == NULL || b == NULL) return 0;
    if (a->output_value < b->output_value) return -1;
    if (a->output_value > b->output_value) return 1;
    return 0;
}

/**
 * Comparator Callback: Compares records by output_value (Descending: High to Low).
 */
int compare_by_output_desc(const Record *a, const Record *b) {
    if (a == NULL || b == NULL) return 0;
    if (a->output_value > b->output_value) return -1;
    if (a->output_value < b->output_value) return 1;
    return 0;
}

/* =========================================================================
 * SEARCH IMPLEMENTATIONS (PHASE 5)
 * ========================================================================= */

#include <ctype.h>

static int str_case_contains_local(const char *haystack, const char *needle) {
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
        if (j == n_len) return 1;
    }
    return 0;
}

/**
 * Searches conversion history by conversion type keyword (case-insensitive).
 *
 * @param hm          Pointer to HistoryManager struct.
 * @param search_term Conversion name keyword to search.
 */
void search_history_by_type(const HistoryManager *hm, const char *search_term) {
    if (hm == NULL || search_term == NULL || hm->count == 0) {
        printf("\n[INFO] History log is empty. Nothing to search.\n");
        return;
    }

    printf("\n=========================================================================================\n");
    printf("              SEARCH RESULTS FOR CONVERSION TYPE: '%s'                   \n", search_term);
    printf("=========================================================================================\n");
    printf(" %-3s | %-19s | %-24s | %-16s | %-16s\n",
           "#", "Timestamp", "Conversion Type", "Input Value", "Converted Result");
    printf("-----+---------------------+--------------------------+------------------+-------------------\n");

    int matches = 0;
    for (int i = 0; i < hm->count; i++) {
        const Record *r = &hm->records[i];
        if (str_case_contains_local(r->conversion_name, search_term)) {
            matches++;
            char in_str[30], out_str[30];
            snprintf(in_str, sizeof(in_str), "%.4f %s", r->input_value, r->from_unit);
            snprintf(out_str, sizeof(out_str), "%.4f %s", r->output_value, r->to_unit);

            printf(" %-3d | %-19s | %-24s | %-16s | %-16s\n",
                   r->id, r->timestamp, r->conversion_name, in_str, out_str);
        }
    }

    printf("=========================================================================================\n");
    printf(" Matches Found: %d of %d total records.\n", matches, hm->count);
    printf("=========================================================================================\n");
}

/**
 * Searches conversion history by converted result value within a range [min_val, max_val].
 *
 * @param hm      Pointer to HistoryManager struct.
 * @param min_val Minimum converted output value.
 * @param max_val Maximum converted output value.
 */
void search_history_by_value(const HistoryManager *hm, double min_val, double max_val) {
    if (hm == NULL || hm->count == 0) {
        printf("\n[INFO] History log is empty. Nothing to search.\n");
        return;
    }

    if (min_val > max_val) {
        // Swap bounds if inverted
        double temp = min_val;
        min_val = max_val;
        max_val = temp;
    }

    printf("\n=========================================================================================\n");
    printf("         SEARCH RESULTS FOR CONVERTED VALUE RANGE [%.2f to %.2f]         \n", min_val, max_val);
    printf("=========================================================================================\n");
    printf(" %-3s | %-19s | %-24s | %-16s | %-16s\n",
           "#", "Timestamp", "Conversion Type", "Input Value", "Converted Result");
    printf("-----+---------------------+--------------------------+------------------+-------------------\n");

    int matches = 0;
    for (int i = 0; i < hm->count; i++) {
        const Record *r = &hm->records[i];
        if (r->output_value >= min_val && r->output_value <= max_val) {
            matches++;
            char in_str[30], out_str[30];
            snprintf(in_str, sizeof(in_str), "%.4f %s", r->input_value, r->from_unit);
            snprintf(out_str, sizeof(out_str), "%.4f %s", r->output_value, r->to_unit);

            printf(" %-3d | %-19s | %-24s | %-16s | %-16s\n",
                   r->id, r->timestamp, r->conversion_name, in_str, out_str);
        }
    }

    printf("=========================================================================================\n");
    printf(" Matches Found: %d of %d total records.\n", matches, hm->count);
    printf("=========================================================================================\n");
}



