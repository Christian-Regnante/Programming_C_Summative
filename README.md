# Programming in C - Summative Portfolio

Welcome to the **Programming in C Summative Portfolio**. This repository contains five modular, industry-aligned projects covering advanced low-level C concepts, embedded systems design, shell automation, data structures, function pointers, file system manipulation, and multi-threaded processing.

---

## Assessment Outcomes Coverage

This portfolio directly demonstrates mastery of the core C and systems programming outcomes:

| Outcome | Demonstrated In | Key Technical Concepts |
| :--- | :--- | :--- |
| **Manipulate strings using C language** | **Projects 3, 4, 5** | Tokenization (`strtok`), input sanitization, string formatting (`snprintf`), string sorting (`strcmp`), code parsing. |
| **Use function pointers properly in C language** | **Project 4** | Function pointer dispatch tables (`typedef double (*ConversionFunc)(double)`), modular algorithm mapping. |
| **Design circuits and deploy the right program** | **Project 1** | Arduino UNO/Nano microcontroller C++, Tinkercad circuit simulation, EasyEDA 2-layer PCB schematic & layout design. |
| **Implement multiple threads** | **Project 5** | POSIX Threads (`pthreads`), concurrent file processing, thread synchronization, non-blocking file analysis. |
| **Manipulate file system using structures** | **Projects 3, 5** | Binary struct serialization (`library_data.bin`), dynamic array reallocation (`realloc`), directory scanning, report generation. |
| **Write bash script that completes some tasks** | **Project 2** | Shell scripting (`backup_manager.sh`), environment checks, `tar`/`gzip` compression, automated backup rotation, logging. |

---

## Project Structure & Overview

```text
Programming_C_Summative/
├── Project1/     # Smart Parking Lot Monitoring System (Embedded C / Tinkercad / EasyEDA PCB)
├── Project2/     # Automated Backup Manager (Bash Shell Scripting)
├── Project3/     # Library Book Inventory Management System (Structs, Dynamic Memory, Binary I/O)
├── Project4/     # Multi-Unit Converter Toolkit (Function Pointer Dispatch Tables & History Tracking)
├── Project5/     # Multi-Threaded File & Log System Analyzer (POSIX pthreads, Directory Scanning)
└── README.md     # Project Documentation
```

---

### Project 1: Smart Parking Lot Monitoring System
* **Directory**: [`Project1/`](file:///g:/Programming%20in%20C/Programming_C_Summative/Project1)
* **Primary Files**: [`project1_parking.ino`](file:///g:/Programming%20in%20C/Programming_C_Summative/Project1/project1_parking.ino), [`project1_pseudocode`](file:///g:/Programming%20in%20C/Programming_C_Summative/Project1/project1_pseudocode)
* **Hardware & Tools**: Arduino UNO / Nano, Tinkercad Circuits, EasyEDA PCB Designer.

#### Key Features & Implementation:
* **Microcontroller Logic**: Implements an automated vehicle entry/exit parking gate system monitoring 4 parking slots.
* **Non-Blocking Debouncing**: Asynchronous input processing using `millis()` without blocking `delay()` calls.
* **Dynamic Memory & Structs**: Heap allocation of `ParkingSpace` structures (`malloc`) accessed via pointers (`->`).
* **Visual LED Feedback**: LEDs visually signal space availability (ON = Available, OFF = Occupied).
* **Serial Monitor Reporting**: Prints real-time vehicle entry/exit actions and live occupancy statistics.
* **PCB Schematic & Layout**: Complete 2-layer PCB designed in EasyEDA with component footprints, trace routing, and ground pour.

---

### Project 2: Automated Backup Manager
* **Directory**: [`Project2/`](file:///g:/Programming%20in%20C/Programming_C_Summative/Project2)
* **Primary File**: [`backup_manager.sh`](file:///g:/Programming%20in%20C/Programming_C_Summative/Project2/backup_manager.sh)
* **Language & Tools**: Bash Shell Scripting, Linux Utilities (`tar`, `gzip`, `find`, `du`).

#### Key Features & Implementation:
* **Automated Archiving**: Takes source directories, validates input paths, and generates compressed `.tar.gz` backups.
* **Backup Rotation**: Automatically scans target backup directories and purges old archives exceeding maximum retention limits.
* **Storage Space Auditing**: Monitors disk utilization and alerts users of storage thresholds before creating archives.
* **Detailed System Logging**: Records timestamps, file lists, archive sizes, and exit statuses into a persistent log file.

---

### Project 3: Library Book Inventory Management System
* **Directory**: [`Project3/`](file:///g:/Programming%20in%20C/Programming_C_Summative/Project3)
* **Primary File**: [`project3_main.c`](file:///g:/Programming%20in%20C/Programming_C_Summative/Project3/project3_main.c)
* **Language & Compiler**: GCC.

#### Key Features & Implementation:
* **Dynamic Array Management**: Struct inventory dynamically resizes using `malloc`, `realloc`, and `free`.
* **Full CRUD Operations**: Create, Read, Update, Delete, and Search book records by ID, Title, or Author.
* **Text Data Persistence**: Saves and loads records from a simple pipe-delimited text file for easier inspection.
* **Text Record Importer**: Parses and imports raw text records from formatted input files.
* **Sorting Algorithms**: Sorts book records by ID, Title, or Available Copies using custom sorting routines.

---

### Project 4: Multi-Unit Converter & History Toolkit
* **Directory**: [`Project4/`](file:///g:/Programming%20in%20C/Programming_C_Summative/Project4)
* **Primary Files**: [`project4_main.c`](file:///g:/Programming%20in%20C/Programming_C_Summative/Project4/project4_main.c), [`project4_conversions.c`](file:///g:/Programming%20in%20C/Programming_C_Summative/Project4/project4_conversions.c), [`project4_history.c`](file:///g:/Programming%20in%20C/Programming_C_Summative/Project4/project4_history.c)
* **Language & Compiler**: GCC.

#### Key Features & Implementation:
* **Function Pointer Dispatch Tables**: Maps unit conversion mathematical functions cleanly using:
  ```c
  typedef double (*ConversionFunc)(double input);
  ```
* **Bitwise Conversion Tools**: Includes binary, hexadecimal, and bitwise manipulation features alongside unit conversions (temperature, distance, mass, length).
* **Persistent History Tracking**: Logs every conversion query with timestamps and values into a binary file (`unit_history.bin`).
* **Modular Architecture**: Separate header and implementation files decoupling conversion math from CLI history management.

---

### Project 5: Multi-Threaded File & Log System Analyzer
* **Directory**: [`Project5/`](file:///g:/Programming%20in%20C/Programming_C_Summative/Project5)
* **Primary Files**: [`project5_main.c`](file:///g:/Programming%20in%20C/Programming_C_Summative/Project5/project5_main.c), [`project5_analyzer.c`](file:///g:/Programming%20in%20C/Programming_C_Summative/Project5/project5_analyzer.c), [`project5_analyzer.h`](file:///g:/Programming%20in%20C/Programming_C_Summative/Project5/project5_analyzer.h)
* **Language & Compiler**: GCC.

#### Key Features & Implementation:
* **Multi-Threaded Execution**: Uses POSIX Threads (`pthread_create`, `pthread_join`) to scan and process multiple text and C source files concurrently.
* **Comprehensive Metrics**: Counts lines, words, characters, C language keywords, and file size statistics per file.
* **Directory File System Scanner**: Automatically scans local workspace directories for processable text files.
* **Report Generation**: Outputs structured `.txt` analysis summaries for each processed file.
* **Docker/Git Style System Logger**: Maintains system activity logs (`system_activity.log`) with formatted timestamps.

---

## Building & Running the Projects

### Prerequisites
* **GCC(for Projects 3, 4, and 5)
* **Bash Shell / Linux / WSL / Git Bash** (for Project 2)
* **Arduino IDE or Tinkercad** (for Project 1)

### Compilation Commands

#### Project 3 (Library System):
```bash
gcc -Wall -Wextra Project3/project3_main.c -o Project3/library_system -lm
./Project3/library_system
```

#### Project 4 (Unit Toolkit):
```bash
gcc -Wall -Wextra Project4/project4_main.c Project4/project4_conversions.c Project4/project4_history.c -o Project4/unit_toolkit
./Project4/unit_toolkit
```

#### Project 5 (Multi-Threaded Analyzer):
```bash
gcc -Wall -Wextra Project5/project5_main.c Project5/project5_analyzer.c -o Project5/file_processor
./Project5/file_processor
```

#### Project 2 (Backup Manager):
```bash
chmod +x Project2/backup_manager.sh
./Project2/backup_manager.sh
```

