#ifndef PROJECT5_ANALYZER_H
#define PROJECT5_ANALYZER_H

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
  #include <windows.h>
  #include <process.h>
  typedef HANDLE pthread_t;
#else
  #include <pthread.h>
#endif

#define LOG_FILE "system_activity.log"

typedef struct {
    int thread_num;
    char input_filename[256];
    char output_filename[256];
    long line_count;
    long word_count;
    long char_count;
    int success;
    char error_msg[256];
} FileTask;

// System Logger & Analyzer Prototypes
void log_activity(const char *event_msg);
int analyze_file(FileTask *task);
int write_analysis_report(const FileTask *task);
void view_activity_log(void);
void view_report_file(const char *report_filename);

// Thread Operations
void* analyze_file_thread(void *arg);
int run_multithreaded_analysis(FileTask tasks[], int num_tasks);
int scan_directory_files(char file_list[][256], int max_files);

int create_worker_thread(pthread_t *thread, void *(*start_routine)(void *), void *arg);
int join_worker_thread(pthread_t thread);

#endif
