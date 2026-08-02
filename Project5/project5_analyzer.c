#define _CRT_SECURE_NO_WARNINGS
#include "project5_analyzer.h"

// --- SYSTEM LOGGER ---

void log_activity(const char *event_msg) {
    if (!event_msg) return;
    FILE *fp = fopen(LOG_FILE, "a");
    if (!fp) return;

    time_t rawtime = time(NULL);
    struct tm *info = localtime(&rawtime);
    char time_str[30];
    if (info) strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", info);
    else strcpy(time_str, "N/A");

    fprintf(fp, "[%s] %s\n", time_str, event_msg);
    fclose(fp);
}

// --- FILE ANALYSIS ENGINE ---

int analyze_file(FileTask *task) {
    if (!task) return 0;

    char log_buf[512];
    snprintf(log_buf, sizeof(log_buf), "Thread #%d: Starting analysis on '%s'...", task->thread_num, task->input_filename);
    log_activity(log_buf);

    FILE *fp = fopen(task->input_filename, "r");
    if (!fp) {
        task->success = 0;
        snprintf(task->error_msg, sizeof(task->error_msg), "Could not open file '%s'", task->input_filename);
        snprintf(log_buf, sizeof(log_buf), "Thread #%d: ERROR - %s", task->thread_num, task->error_msg);
        log_activity(log_buf);
        return 0;
    }

    task->line_count = 0;
    task->word_count = 0;
    task->char_count = 0;
    int ch, in_word = 0;

    while ((ch = fgetc(fp)) != EOF) {
        task->char_count++;
        if (ch == '\n') task->line_count++;
        if (isspace(ch)) {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            task->word_count++;
        }
    }
    fclose(fp);
    task->success = 1;

    if (strlen(task->output_filename) == 0) {
        snprintf(task->output_filename, sizeof(task->output_filename), "%s_analysis.txt", task->input_filename);
    }
    write_analysis_report(task);

    snprintf(log_buf, sizeof(log_buf), "Thread #%d: Completed '%s' (Lines: %ld, Words: %ld, Chars: %ld)",
             task->thread_num, task->input_filename, task->line_count, task->word_count, task->char_count);
    log_activity(log_buf);
    return 1;
}

int write_analysis_report(const FileTask *task) {
    if (!task || strlen(task->output_filename) == 0) return 0;

    FILE *fp = fopen(task->output_filename, "w");
    if (!fp) return 0;

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

void view_activity_log(void) {
    FILE *fp = fopen(LOG_FILE, "r");
    if (!fp) {
        printf("\n[INFO] Activity log '%s' is empty or not created yet.\n", LOG_FILE);
        return;
    }

    printf("\n=======================================================\n");
    printf("            SYSTEM ACTIVITY LOG (DOCKER STYLE)         \n");
    printf("=======================================================\n");
    char line[512];
    while (fgets(line, sizeof(line), fp)) printf("%s", line);
    printf("=======================================================\n");
    fclose(fp);
}

void view_report_file(const char *report_filename) {
    if (!report_filename) return;
    FILE *fp = fopen(report_filename, "r");
    if (!fp) {
        printf("[ERROR] Could not open report file '%s'.\n", report_filename);
        return;
    }
    printf("\n");
    char line[512];
    while (fgets(line, sizeof(line), fp)) printf("%s", line);
    fclose(fp);
}

// --- THREAD ENGINE & DIRECTORY SCANNER ---

void* analyze_file_thread(void *arg) {
    FileTask *task = (FileTask *)arg;
    if (task) analyze_file(task);
    return NULL;
}

#ifdef _WIN32
typedef struct { void *(*func)(void *); void *arg; } ThreadWrapperArgs;
static unsigned __stdcall win_thread_proc(void *p) {
    ThreadWrapperArgs *w = (ThreadWrapperArgs *)p;
    w->func(w->arg);
    free(w);
    return 0;
}
int create_worker_thread(pthread_t *thread, void *(*start_routine)(void *), void *arg) {
    ThreadWrapperArgs *w = (ThreadWrapperArgs *)malloc(sizeof(ThreadWrapperArgs));
    w->func = start_routine;
    w->arg = arg;
    HANDLE h = (HANDLE)_beginthreadex(NULL, 0, win_thread_proc, w, 0, NULL);
    if (h == NULL) { free(w); return -1; }
    *thread = h;
    return 0;
}
int join_worker_thread(pthread_t thread) {
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    return 0;
}
#else
int create_worker_thread(pthread_t *thread, void *(*start_routine)(void *), void *arg) {
    return pthread_create(thread, NULL, start_routine, arg);
}
int join_worker_thread(pthread_t thread) {
    return pthread_join(thread, NULL);
}
#endif

int run_multithreaded_analysis(FileTask tasks[], int num_tasks) {
    if (!tasks || num_tasks <= 0) return 0;
    pthread_t *threads = (pthread_t *)malloc(num_tasks * sizeof(pthread_t));
    if (!threads) return 0;

    printf("\n[INFO] Launching %d worker thread(s) for concurrent file analysis...\n", num_tasks);
    for (int i = 0; i < num_tasks; i++) {
        tasks[i].thread_num = i + 1;
        create_worker_thread(&threads[i], analyze_file_thread, &tasks[i]);
    }

    for (int i = 0; i < num_tasks; i++) {
        join_worker_thread(threads[i]);
    }
    free(threads);
    printf("[SUCCESS] All %d file analysis thread(s) completed.\n", num_tasks);
    return 1;
}

int scan_directory_files(char file_list[][256], int max_files) {
    int count = 0;
    const char *default_files[] = {
        "sample1.txt", "sample2.txt", "sample3.c", "sample4.txt",
        "book_record_format1.txt", "book_record_format2.txt", "book_record_format3.txt"
    };
    int total_defaults = sizeof(default_files) / sizeof(default_files[0]);

    for (int i = 0; i < total_defaults && count < max_files; i++) {
        FILE *fp = fopen(default_files[i], "r");
        if (fp) {
            fclose(fp);
            strncpy(file_list[count++], default_files[i], 255);
        }
    }
    return count;
}
