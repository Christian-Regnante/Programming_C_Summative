#include "project5_analyzer.h"

/* =========================================================================
 * SYSTEM LOGGING IMPLEMENTATION
 * ========================================================================= */

/**
 * Appends a timestamped log entry to system_activity.log.
 *
 * @param event_msg Text message describing the file processing activity.
 */
void log_activity(const char *event_msg) {
    if (event_msg == NULL) return;

    FILE *fp = fopen(LOG_FILE, "a");
    if (fp == NULL) return;

    time_t rawtime;
    struct tm *timeinfo;
    char time_str[30];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    if (timeinfo != NULL) {
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
    } else {
        strcpy(time_str, "UNKNOWN_TIME");
    }

    fprintf(fp, "[%s] %s\n", time_str, event_msg);
    fclose(fp);
}

/* =========================================================================
 * FILE ANALYSIS ENGINE
 * ========================================================================= */

/**
 * Reads an input file line by line and character by character to compute:
 * - Line count
 * - Word count
 * - Character count
 *
 * @param task Pointer to FileTask struct containing input/output paths.
 * @return 1 on successful analysis, 0 on failure.
 */
int analyze_file(FileTask *task) {
    if (task == NULL) return 0;

    char log_buf[512];
    snprintf(log_buf, sizeof(log_buf), "Thread #%d: Starting analysis on input file '%s'...",
             task->thread_num, task->input_filename);
    log_activity(log_buf);

    FILE *fp = fopen(task->input_filename, "r");
    if (fp == NULL) {
        task->success = 0;
        snprintf(task->error_msg, sizeof(task->error_msg), "Could not open file '%s'", task->input_filename);
        
        snprintf(log_buf, sizeof(log_buf), "Thread #%d: ERROR - %s", task->thread_num, task->error_msg);
        log_activity(log_buf);
        return 0;
    }

    task->line_count = 0;
    task->word_count = 0;
    task->char_count = 0;

    int ch;
    int in_word = 0;

    // Single-pass stream reading
    while ((ch = fgetc(fp)) != EOF) {
        task->char_count++;

        if (ch == '\n') {
            task->line_count++;
        }

        if (isspace(ch)) {
            in_word = 0; // Transitioned out of a word
        } else if (!in_word) {
            in_word = 1; // Transitioned into a new word
            task->word_count++;
        }
    }

    fclose(fp);
    task->success = 1;

    // Generate report output path if empty
    if (strlen(task->output_filename) == 0) {
        snprintf(task->output_filename, sizeof(task->output_filename), "%s_analysis.txt", task->input_filename);
    }

    // Write output analysis report file
    write_analysis_report(task);

    snprintf(log_buf, sizeof(log_buf),
             "Thread #%d: Completed analysis for '%s' (Lines: %ld, Words: %ld, Chars: %ld) -> Report: '%s'",
             task->thread_num, task->input_filename, task->line_count,
             task->word_count, task->char_count, task->output_filename);
    log_activity(log_buf);

    return 1;
}

/**
 * Writes calculated analysis results to a dedicated output text file.
 *
 * @param task Pointer to completed FileTask struct.
 * @return 1 on success, 0 on failure.
 */
int write_analysis_report(const FileTask *task) {
    if (task == NULL || strlen(task->output_filename) == 0) return 0;

    FILE *fp = fopen(task->output_filename, "w");
    if (fp == NULL) {
        char log_buf[512];
        snprintf(log_buf, sizeof(log_buf), "Thread #%d: ERROR - Could not create output report file '%s'",
                 task->thread_num, task->output_filename);
        log_activity(log_buf);
        return 0;
    }

    fprintf(fp, "=======================================================\n");
    fprintf(fp, "               FILE ANALYSIS REPORT                    \n");
    fprintf(fp, "=======================================================\n");
    fprintf(fp, " Input Filename  : %s\n", task->input_filename);
    fprintf(fp, " Thread Assigned : Thread #%d\n", task->thread_num);
    fprintf(fp, " Status          : %s\n", task->success ? "SUCCESS" : "FAILED");
    if (!task->success) {
        fprintf(fp, " Error Details   : %s\n", task->error_msg);
    } else {
        fprintf(fp, "-------------------------------------------------------\n");
        fprintf(fp, " Total Lines     : %ld\n", task->line_count);
        fprintf(fp, " Total Words     : %ld\n", task->word_count);
        fprintf(fp, " Total Chars     : %ld\n", task->char_count);
    }
    fprintf(fp, "=======================================================\n");

    fclose(fp);
    return 1;
}

/**
 * Displays the contents of system_activity.log to stdout (Docker/Git log style).
 */
void view_activity_log(void) {
    FILE *fp = fopen(LOG_FILE, "r");
    if (fp == NULL) {
        printf("\n=======================================================\n");
        printf("                SYSTEM ACTIVITY LOGS                   \n");
        printf("=======================================================\n");
        printf(" No activity logs recorded yet.\n");
        printf("=======================================================\n");
        return;
    }

    printf("\n=======================================================\n");
    printf("                SYSTEM ACTIVITY LOGS                   \n");
    printf("=======================================================\n");

    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        printf(" %s", line);
        count++;
    }

    fclose(fp);
    printf("=======================================================\n");
    printf(" Total Log Entries: %d\n", count);
    printf("=======================================================\n");
}

/* =========================================================================
 * POSIX THREAD ENGINE IMPLEMENTATIONS (PHASE 2)
 * ========================================================================= */

/**
 * Entry point executed independently by each worker thread for one file.
 *
 * @param arg Pointer to FileTask struct allocated for this thread.
 * @return NULL on completion.
 */
void* analyze_file_thread(void *arg) {
    if (arg == NULL) return NULL;

    FileTask *task = (FileTask *)arg;

    printf("[THREAD #%d] Started processing file '%s'...\n",
           task->thread_num, task->input_filename);

    // Perform actual single-pass file analysis
    analyze_file(task);

    if (task->success) {
        printf("[THREAD #%d] COMPLETED '%s' -> Lines: %ld, Words: %ld, Chars: %ld | Saved: '%s'\n",
               task->thread_num, task->input_filename, task->line_count,
               task->word_count, task->char_count, task->output_filename);
    } else {
        printf("[THREAD #%d] FAILED '%s': %s\n",
               task->thread_num, task->input_filename, task->error_msg);
    }

    return NULL;
}

/**
 * Spawns an independent POSIX worker thread for each task, running them concurrently,
 * and waits for all threads to join before returning.
 *
 * @param tasks     Array of FileTask structures.
 * @param num_tasks Number of tasks/files to process concurrently.
 * @return Total number of successfully analyzed files.
 */
int run_multithreaded_analysis(FileTask tasks[], int num_tasks) {
    if (tasks == NULL || num_tasks <= 0) return 0;

    pthread_t *threads = (pthread_t *)malloc((size_t)num_tasks * sizeof(pthread_t));
    if (threads == NULL) {
        printf("[ERROR] Failed to allocate thread handle array!\n");
        return 0;
    }

    char log_buf[256];
    snprintf(log_buf, sizeof(log_buf), "Launching %d concurrent worker threads for file analysis...", num_tasks);
    log_activity(log_buf);

    printf("\n=======================================================\n");
    printf("        CONCURRENT THREAD PROCESSING DISPATCHER        \n");
    printf("=======================================================\n");

    // 1. Spawn worker thread for each file task concurrently
    for (int i = 0; i < num_tasks; i++) {
        tasks[i].thread_num = i + 1;
        if (create_worker_thread(&threads[i], analyze_file_thread, &tasks[i]) != 0) {
            printf("[ERROR] Failed to spawn worker thread #%d for '%s'!\n", i + 1, tasks[i].input_filename);
        }
    }

    // 2. Synchronize: Wait for all worker threads to complete (pthread_join)
    int success_count = 0;
    for (int i = 0; i < num_tasks; i++) {
        join_worker_thread(threads[i]);
        if (tasks[i].success) {
            success_count++;
        }
    }

    free(threads);

    printf("=======================================================\n");
    printf(" All %d thread(s) completed. Successful: %d | Failed: %d\n",
           num_tasks, success_count, num_tasks - success_count);
    printf("=======================================================\n");

    snprintf(log_buf, sizeof(log_buf), "All %d concurrent worker threads completed (Success: %d, Failed: %d).",
             num_tasks, success_count, num_tasks - success_count);
    log_activity(log_buf);

    return success_count;
}

/* =========================================================================
 * DIRECTORY FILE SCANNER (PHASE 3)
 * ========================================================================= */

#ifndef _WIN32
  #include <dirent.h>
#endif

/**
 * Scans the current working directory for processable text and source files (.txt, .c, .h).
 * Skips log files (.log) and previously generated analysis report files (*_analysis.txt).
 *
 * @param file_list Output 2D array to store discovered filenames.
 * @param max_files Maximum capacity of file_list array.
 * @return Total number of processable files found.
 */
int scan_directory_files(char file_list[][256], int max_files) {
    if (file_list == NULL || max_files <= 0) return 0;

    int count = 0;

#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA("*.*", &findData);
    if (hFind == INVALID_HANDLE_VALUE) return 0;

    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            const char *name = findData.cFileName;
            size_t len = strlen(name);

            // Ignore activity logs and report files
            if (strstr(name, "_analysis.txt") != NULL || strstr(name, ".log") != NULL) {
                continue;
            }

            // Include .txt, .c, and .h files
            if ((len > 4 && strcmp(name + len - 4, ".txt") == 0) ||
                (len > 2 && strcmp(name + len - 2, ".c") == 0) ||
                (len > 2 && strcmp(name + len - 2, ".h") == 0)) {
                if (count < max_files) {
                    strncpy(file_list[count], name, 255);
                    file_list[count][255] = '\0';
                    count++;
                }
            }
        }
    } while (FindNextFileA(hFind, &findData) && count < max_files);

    FindClose(hFind);
#else
    DIR *dir = opendir(".");
    if (dir == NULL) return 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && count < max_files) {
        const char *name = entry->d_name;
        size_t len = strlen(name);

        if (strstr(name, "_analysis.txt") != NULL || strstr(name, ".log") != NULL) {
            continue;
        }

        if ((len > 4 && strcmp(name + len - 4, ".txt") == 0) ||
            (len > 2 && strcmp(name + len - 2, ".c") == 0) ||
            (len > 2 && strcmp(name + len - 2, ".h") == 0)) {
            strncpy(file_list[count], name, 255);
            file_list[count][255] = '\0';
            count++;
        }
    }
    closedir(dir);
#endif

    return count;
}

/**
 * Scans the current working directory for generated analysis report files (*_analysis.txt).
 *
 * @param report_list Output 2D array to store discovered report filenames.
 * @param max_files   Maximum capacity of report_list array.
 * @return Total number of analysis report files found.
 */
int scan_report_files(char report_list[][256], int max_files) {
    if (report_list == NULL || max_files <= 0) return 0;

    int count = 0;

#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA("*.*", &findData);
    if (hFind == INVALID_HANDLE_VALUE) return 0;

    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            const char *name = findData.cFileName;
            if (strstr(name, "_analysis.txt") != NULL) {
                if (count < max_files) {
                    strncpy(report_list[count], name, 255);
                    report_list[count][255] = '\0';
                    count++;
                }
            }
        }
    } while (FindNextFileA(hFind, &findData) && count < max_files);

    FindClose(hFind);
#else
    DIR *dir = opendir(".");
    if (dir == NULL) return 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && count < max_files) {
        const char *name = entry->d_name;
        if (strstr(name, "_analysis.txt") != NULL) {
            strncpy(report_list[count], name, 255);
            report_list[count][255] = '\0';
            count++;
        }
    }
    closedir(dir);
#endif

    return count;
}




/**
 * Creates a worker thread portably (Windows _beginthreadex / POSIX pthread_create).
 */
int create_worker_thread(pthread_t *thread, void *(*start_routine)(void *), void *arg) {
    if (thread == NULL || start_routine == NULL) return -1;

#ifdef _WIN32
    *thread = (HANDLE)_beginthreadex(NULL, 0, (unsigned (__stdcall *)(void *))start_routine, arg, 0, NULL);
    return (*thread != NULL) ? 0 : -1;
#else
    return pthread_create(thread, NULL, start_routine, arg);
#endif
}

/**
 * Joins a worker thread portably (Windows WaitForSingleObject / POSIX pthread_join).
 */
int join_worker_thread(pthread_t thread) {
#ifdef _WIN32
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    return 0;
#else
    return pthread_join(thread, NULL);
#endif
}


/**
 * Displays the content of a specific analysis report file to stdout.
 *
 * @param report_filename Path to report file.
 */
void view_report_file(const char *report_filename) {
    if (report_filename == NULL) return;

    FILE *fp = fopen(report_filename, "r");
    if (fp == NULL) {
        printf("[ERROR] Could not open analysis report file '%s'.\n", report_filename);
        return;
    }

    printf("\n");
    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        printf("%s", line);
    }
    fclose(fp);
}
