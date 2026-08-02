#ifndef PROJECT4_HISTORY_H
#define PROJECT4_HISTORY_H

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INITIAL_HISTORY_CAPACITY 10

typedef struct {
    int id;
    char timestamp[25];
    char conversion_name[50];
    char from_unit[10];
    char to_unit[10];
    double input_value;
    double output_value;
} Record;

typedef struct {
    Record *records;
    int count;
    int capacity;
} HistoryManager;

// Function Pointer Callbacks
typedef void (*RecordProcessor)(Record *rec, void *param);
typedef int (*RecordPredicate)(const Record *rec, const void *criterion);
typedef int (*RecordComparator)(const Record *a, const Record *b);

// Memory Management
int init_history(HistoryManager *hm);
int resize_history(HistoryManager *hm);
void free_history(HistoryManager *hm);

// History Operations
int add_record(HistoryManager *hm, const char *name, const char *from_unit, const char *to_unit, double in_val, double out_val);
void view_history(const HistoryManager *hm);
void get_current_timestamp(char *buffer, size_t size);

// Callback Processing Engine
void process_history_batch(HistoryManager *hm, RecordProcessor processor, void *param);
void filter_history(const HistoryManager *hm, RecordPredicate predicate, const void *criterion);
void sort_history(HistoryManager *hm, RecordComparator comparator);

// Concrete Callbacks
void callback_round_precision(Record *rec, void *param);
int predicate_filter_by_min_val(const Record *rec, const void *criterion);
int predicate_filter_by_type(const Record *rec, const void *criterion);
int compare_by_type(const Record *a, const Record *b);
int compare_by_output_asc(const Record *a, const Record *b);
int compare_by_output_desc(const Record *a, const Record *b);

// Search & File I/O
void search_history_by_type(const HistoryManager *hm, const char *search_term);
void search_history_by_value(const HistoryManager *hm, double min_val, double max_val);
int save_history_bin(const HistoryManager *hm, const char *filename);
int load_history_bin(HistoryManager *hm, const char *filename);

#endif
