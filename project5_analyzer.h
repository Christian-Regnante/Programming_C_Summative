#ifndef PROJECT5_ANALYZER_H
#define PROJECT5_ANALYZER_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// Cross-Platform POSIX / Windows Thread Support
#ifdef _WIN32
  #include <windows.h>
  #include <process.h>
  typedef HANDLE pthread_t;
#else
  #include <pthread.h>
#endif

#define LOG_FILE "system_activity.log"

/**
 * Task structure holding input filename, thread metadata, and analysis results.
 */
typedef struct {
    int thread_num;              // Thread index number (1-based)
    char input_filename[256];    // Source file path
    char output_filename[256];   // Analysis report destination path
    long line_count;             // Total lines counted
    long word_count;             // Total words counted
    long char_count;             // Total characters counted
    int success;                 // 1 on success, 0 on file open/read error
    char error_msg[256];         // Error message string if failed
} FileTask;

/* =========================================================================
 * SYSTEM LOGGER & ANALYZER PROTOTYPES
 * ========================================================================= */
void log_activity(const char *event_msg);
int analyze_file(FileTask *task);
int write_analysis_report(const FileTask *task);
void view_activity_log(void);
void view_report_file(const char *report_filename);

// Thread Entry Point & Multithreading Engine
void* analyze_file_thread(void *arg);
int run_multithreaded_analysis(FileTask tasks[], int num_tasks);
int scan_directory_files(char file_list[][256], int max_files);
int scan_report_files(char report_list[][256], int max_files);

// Thread Helper Abstractions
int create_worker_thread(pthread_t *thread, void *(*start_routine)(void *), void *arg);
int join_worker_thread(pthread_t thread);

#endif // PROJECT5_ANALYZER_H
