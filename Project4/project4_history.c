#define _CRT_SECURE_NO_WARNINGS
#include "project4_history.h"
#include <ctype.h>
#include <math.h>

// --- MEMORY MANAGEMENT ---

int init_history(HistoryManager *hm) {
    if (!hm) return 0;
    hm->capacity = INITIAL_HISTORY_CAPACITY;
    hm->count = 0;
    hm->records = (Record *)malloc(hm->capacity * sizeof(Record));
    return hm->records != NULL;
}

int resize_history(HistoryManager *hm) {
    if (!hm) return 0;
    int new_cap = hm->capacity * 2;
    Record *temp = (Record *)realloc(hm->records, new_cap * sizeof(Record));
    if (!temp) return 0;
    hm->records = temp;
    hm->capacity = new_cap;
    return 1;
}

void free_history(HistoryManager *hm) {
    if (hm && hm->records) {
        free(hm->records);
        hm->records = NULL;
        hm->count = 0;
        hm->capacity = 0;
    }
}

// --- UTILITY & ADD RECORD ---

void get_current_timestamp(char *buffer, size_t size) {
    if (!buffer || size == 0) return;
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t) strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
    else strncpy(buffer, "N/A", size);
}

int add_record(HistoryManager *hm, const char *name, const char *from_unit, const char *to_unit, double in_val, double out_val) {
    if (!hm) return 0;
    if (hm->count >= hm->capacity && !resize_history(hm)) return 0;

    Record *r = &hm->records[hm->count];
    r->id = hm->count + 1;
    get_current_timestamp(r->timestamp, sizeof(r->timestamp));
    strncpy(r->conversion_name, name ? name : "Unknown", sizeof(r->conversion_name) - 1);
    strncpy(r->from_unit, from_unit ? from_unit : "", sizeof(r->from_unit) - 1);
    strncpy(r->to_unit, to_unit ? to_unit : "", sizeof(r->to_unit) - 1);
    r->input_value = in_val;
    r->output_value = out_val;
    hm->count++;
    return 1;
}

void view_history(const HistoryManager *hm) {
    if (!hm || hm->count == 0) {
        printf("\n[INFO] History is currently empty.\n");
        return;
    }
    printf("\n===================================================================================================\n");
    printf(" %-4s | %-19s | %-24s | %-12s | %-12s\n", "ID", "TIMESTAMP", "CONVERSION", "INPUT", "RESULT");
    printf("===================================================================================================\n");
    for (int i = 0; i < hm->count; i++) {
        const Record *r = &hm->records[i];
        printf(" %-4d | %-19s | %-24.24s | %8.2f %-3s | %8.2f %-3s\n",
               r->id, r->timestamp, r->conversion_name, r->input_value, r->from_unit, r->output_value, r->to_unit);
    }
    printf("===================================================================================================\n");
    printf(" Total Records: %d | Capacity: %d\n", hm->count, hm->capacity);
}

// --- CALLBACK ENGINE & CONCRETE CALLBACKS ---

void process_history_batch(HistoryManager *hm, RecordProcessor processor, void *param) {
    if (!hm || !processor) return;
    for (int i = 0; i < hm->count; i++) processor(&hm->records[i], param);
}

void filter_history(const HistoryManager *hm, RecordPredicate predicate, const void *criterion) {
    if (!hm || !predicate) return;
    int matches = 0;
    printf("\n--- FILTERED HISTORY RESULTS ---\n");
    for (int i = 0; i < hm->count; i++) {
        if (predicate(&hm->records[i], criterion)) {
            const Record *r = &hm->records[i];
            printf(" #%-3d [%s] %-22s: %.2f %s -> %.2f %s\n",
                   r->id, r->timestamp, r->conversion_name, r->input_value, r->from_unit, r->output_value, r->to_unit);
            matches++;
        }
    }
    if (matches == 0) printf("No matching records found.\n");
}

void sort_history(HistoryManager *hm, RecordComparator comparator) {
    if (!hm || !comparator || hm->count <= 1) return;
    for (int i = 0; i < hm->count - 1; i++) {
        for (int j = i + 1; j < hm->count; j++) {
            if (comparator(&hm->records[i], &hm->records[j]) > 0) {
                Record temp = hm->records[i];
                hm->records[i] = hm->records[j];
                hm->records[j] = temp;
            }
        }
    }
}

void callback_round_precision(Record *rec, void *param) {
    if (!rec) return;
    int decimals = param ? *(int *)param : 2;
    double factor = pow(10.0, decimals);
    rec->input_value = round(rec->input_value * factor) / factor;
    rec->output_value = round(rec->output_value * factor) / factor;
}

int predicate_filter_by_min_val(const Record *rec, const void *criterion) {
    if (!rec || !criterion) return 0;
    return rec->output_value >= *(const double *)criterion;
}

int predicate_filter_by_type(const Record *rec, const void *criterion) {
    if (!rec || !criterion) return 0;
    char r_lower[60] = {0}, c_lower[60] = {0};
    const char *crit = (const char *)criterion;
    for (int i = 0; rec->conversion_name[i] && i < 59; i++) r_lower[i] = (char)tolower((unsigned char)rec->conversion_name[i]);
    for (int i = 0; crit[i] && i < 59; i++) c_lower[i] = (char)tolower((unsigned char)crit[i]);
    return strstr(r_lower, c_lower) != NULL;
}

int compare_by_type(const Record *a, const Record *b) {
    return _stricmp(a->conversion_name, b->conversion_name);
}

int compare_by_output_asc(const Record *a, const Record *b) {
    if (a->output_value < b->output_value) return -1;
    return (a->output_value > b->output_value) ? 1 : 0;
}

int compare_by_output_desc(const Record *a, const Record *b) {
    if (a->output_value > b->output_value) return -1;
    return (a->output_value < b->output_value) ? 1 : 0;
}

// --- SEARCH & BINARY FILE I/O ---

void search_history_by_type(const HistoryManager *hm, const char *search_term) {
    filter_history(hm, predicate_filter_by_type, search_term);
}

void search_history_by_value(const HistoryManager *hm, double min_val, double max_val) {
    if (!hm || hm->count == 0) return;
    int matches = 0;
    printf("\n--- SEARCH RESULTS (Result between %.2f and %.2f) ---\n", min_val, max_val);
    for (int i = 0; i < hm->count; i++) {
        if (hm->records[i].output_value >= min_val && hm->records[i].output_value <= max_val) {
            const Record *r = &hm->records[i];
            printf(" #%-3d [%s] %-22s: %.2f %s -> %.2f %s\n",
                   r->id, r->timestamp, r->conversion_name, r->input_value, r->from_unit, r->output_value, r->to_unit);
            matches++;
        }
    }
    if (matches == 0) printf("No records found in range.\n");
}

int save_history_bin(const HistoryManager *hm, const char *filename) {
    if (!hm || !filename) return 0;
    FILE *fp = fopen(filename, "wb");
    if (!fp) return 0;
    fwrite(&hm->count, sizeof(int), 1, fp);
    if (hm->count > 0) fwrite(hm->records, sizeof(Record), hm->count, fp);
    fclose(fp);
    return 1;
}

int load_history_bin(HistoryManager *hm, const char *filename) {
    if (!hm || !filename) return 0;
    FILE *fp = fopen(filename, "rb");
    if (!fp) return 0;
    int loaded = 0;
    if (fread(&loaded, sizeof(int), 1, fp) != 1 || loaded <= 0) { fclose(fp); return 0; }
    hm->count = 0;
    for (int i = 0; i < loaded; i++) {
        if (hm->count >= hm->capacity) resize_history(hm);
        if (fread(&hm->records[hm->count], sizeof(Record), 1, fp) == 1) hm->count++;
    }
    fclose(fp);
    printf("[SUCCESS] Loaded %d conversion records from '%s'.\n", hm->count, filename);
    return 1;
}
