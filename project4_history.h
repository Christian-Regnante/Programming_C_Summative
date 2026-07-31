#ifndef PROJECT4_HISTORY_H
#define PROJECT4_HISTORY_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INITIAL_HISTORY_CAPACITY 10

/**
 * Record structure representing a single completed unit conversion.
 */
typedef struct {
    int id;
    char timestamp[25];       // e.g. "2026-07-31 02:01:48"
    char conversion_name[50]; // e.g. "Kilometers to Miles"
    char from_unit[10];       // e.g. "km"
    char to_unit[10];         // e.g. "mi"
    double input_value;
    double output_value;
} Record;

/**
 * Dynamic memory manager structure holding conversion history records.
 */
typedef struct {
    Record *records;
    int count;
    int capacity;
} HistoryManager;

/* =========================================================================
 * HISTORY MEMORY MANAGEMENT PROTOTYPES
 * ========================================================================= */
int init_history(HistoryManager *hm);
int resize_history(HistoryManager *hm);
void free_history(HistoryManager *hm);

/* =========================================================================
 * HISTORY LOGGING & DISPLAY PROTOTYPES
 * ========================================================================= */
int add_record(HistoryManager *hm, const char *name, const char *from_unit,
               const char *to_unit, double in_val, double out_val);
void view_history(const HistoryManager *hm);
void get_current_timestamp(char *buffer, size_t size);

/* =========================================================================
 * CALLBACK FUNCTION TYPEDEFS
 * ========================================================================= */

// Batch Processor Callback: Modifies or processes a single Record with custom param
typedef void (*RecordProcessor)(Record *rec, void *param);

// Filter Predicate Callback: Evaluates if a Record satisfies a condition criterion (returns 1 if true, 0 if false)
typedef int (*RecordPredicate)(const Record *rec, const void *criterion);

// Comparator Callback: Compares two Records for sorting (returns <0, 0, or >0)
typedef int (*RecordComparator)(const Record *a, const Record *b);

/* =========================================================================
 * CALLBACK PROCESSING ENGINE PROTOTYPES
 * ========================================================================= */
void process_history_batch(HistoryManager *hm, RecordProcessor processor, void *param);
void filter_history(const HistoryManager *hm, RecordPredicate predicate, const void *criterion);
void sort_history(HistoryManager *hm, RecordComparator comparator);

/* =========================================================================
 * CONCRETE CALLBACK FUNCTION PROTOTYPES
 * ========================================================================= */
// Processing Callbacks
void callback_round_precision(Record *rec, void *param);

// Filtering Predicate Callbacks
int predicate_filter_by_min_val(const Record *rec, const void *criterion);
int predicate_filter_by_type(const Record *rec, const void *criterion);

// Sorting Comparator Callbacks
int compare_by_type(const Record *a, const Record *b);
int compare_by_output_asc(const Record *a, const Record *b);
int compare_by_output_desc(const Record *a, const Record *b);

/* =========================================================================
 * SEARCH FUNCTION PROTOTYPES
 * ========================================================================= */
void search_history_by_type(const HistoryManager *hm, const char *search_term);
void search_history_by_value(const HistoryManager *hm, double min_val, double max_val);

/* =========================================================================
 * FILE I/O PROTOTYPES
 * ========================================================================= */
int save_history_bin(const HistoryManager *hm, const char *filename);
int load_history_bin(HistoryManager *hm, const char *filename);

#endif // PROJECT4_HISTORY_H
