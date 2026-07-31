#!/bin/bash
# ==============================================================================
# Linux File Backup & Recovery Manager
# ------------------------------------------------------------------------------
# A clean, simple, and modular Bash script to automate timestamped file backups,
# file recovery, disk space monitoring, and activity logging on Linux (Arch Linux).
# ==============================================================================

# Exit on undeclared variables for safety
set -u

# --- Global Directory & File Configuration ---
CONFIG_DIR="${HOME}/.backup_manager"
BACKUP_DIR="${CONFIG_DIR}/archives"
LOG_FILE="${CONFIG_DIR}/backup_activity.log"
MANIFEST_FILE="${CONFIG_DIR}/backup_history.csv"

# ==============================================================================
# ENVIRONMENT & LOGGING FUNCTIONS
# ==============================================================================

# Ensures required directories and tracking files exist on system launch.
init_environment() {
    # Create configuration and backup archive directories if missing
    mkdir -p "$CONFIG_DIR"
    mkdir -p "$BACKUP_DIR"

    # Initialize activity log file if missing
    if [ ! -f "$LOG_FILE" ]; then
        touch "$LOG_FILE"
    fi

    # Initialize CSV backup history manifest with headers if missing
    if [ ! -f "$MANIFEST_FILE" ]; then
        echo "ID,Timestamp,SourceDir,ArchiveName,ArchiveSize,ArchivePath" > "$MANIFEST_FILE"
    fi

    log_activity "INFO" "Backup Manager system initialized successfully."
}

# Logs a timestamped event entry to the log file.
# Arguments: $1 = Event Type (e.g. INFO, SUCCESS, ERROR), $2 = Log Message
log_activity() {
    local event_type="$1"
    local message="$2"
    local timestamp
    timestamp=$(date +"%Y-%m-%d %H:%M:%S")

    echo "[$timestamp] [$event_type] $message" >> "$LOG_FILE"
}

# Displays available disk space on the backup destination partition.
check_disk_space() {
    echo ""
    echo "-------------------------------------------------------"
    echo "                AVAILABLE DISK SPACE                   "
    echo "-------------------------------------------------------"
    df -h "$BACKUP_DIR" | awk 'NR==1 || NR==2 {print " "$0}'
    echo "-------------------------------------------------------"
}

# Flushes stdin buffer / pauses until user presses ENTER.
pause_screen() {
    echo ""
    read -rp "Press [ENTER] to return to the menu..." temp
}

# ==============================================================================
# STUB FUNCTIONS (To be implemented in upcoming phases)
# ==============================================================================

# Prompts user for a source directory, validates disk space, and creates
# a timestamped compressed tarball backup (.tar.gz), updating manifest & logs.
create_backup() {
    echo ""
    echo "-------------------------------------------------------"
    echo "                 CREATE A NEW BACKUP                   "
    echo "-------------------------------------------------------"
    
    read -rp "Enter directory path to back up (e.g. /home/user/documents): " raw_source
    
    # Expand tilde ~ to HOME directory path if used
    eval source_dir="$raw_source"

    # Validate source directory existence
    if [ -z "$source_dir" ] || [ ! -d "$source_dir" ]; then
        echo ""
        echo "[ERROR] Source directory '$raw_source' does not exist or is invalid!"
        log_activity "ERROR" "Backup creation failed: Invalid source directory '$raw_source'."
        pause_screen
        return 1
    fi

    # Display storage information on backup destination partition
    check_disk_space

    echo ""
    echo "[INFO] Preparing to back up directory: $source_dir"
    read -rp "Proceed with backup creation? (y/n): " confirm
    if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
        echo "Backup creation cancelled."
        pause_screen
        return 0
    fi

    # Generate timestamp and filenames
    local raw_ts
    raw_ts=$(date +"%Y%m%d_%H%M%S")
    local display_ts
    display_ts=$(date +"%Y-%m-%d %H:%M:%S")

    local archive_name="backup_${raw_ts}.tar.gz"
    local archive_path="${BACKUP_DIR}/${archive_name}"

    echo ""
    echo "[INFO] Creating compressed archive '${archive_name}'..."

    # Execute tar compression (-czf) from source directory
    if tar -czf "$archive_path" -C "$source_dir" .; then
        # Retrieve compressed archive file size
        local archive_size
        archive_size=$(du -h "$archive_path" 2>/dev/null | awk '{print $1}')

        # Compute auto-increment ID from CSV manifest line count
        local next_id
        next_id=$(wc -l < "$MANIFEST_FILE" | tr -d ' ')

        # Append record entry to CSV manifest
        echo "${next_id},${display_ts},${source_dir},${archive_name},${archive_size},${archive_path}" >> "$MANIFEST_FILE"

        # Log success activity
        log_activity "BACKUP_SUCCESS" "Created backup '${archive_name}' for '${source_dir}' (Size: ${archive_size})."

        echo ""
        echo "======================================================="
        echo "               BACKUP CREATED SUCCESSFULLY             "
        echo "======================================================="
        echo " Backup ID    : $next_id"
        echo " Archive Name : $archive_name"
        echo " Source Path  : $source_dir"
        echo " Archive Size : $archive_size"
        echo " Storage Path : $archive_path"
        echo "======================================================="
    else
        echo ""
        echo "[ERROR] Tar command failed! Could not create backup archive."
        log_activity "ERROR" "Tar compression failed for source '$source_dir'."
    fi

    pause_screen
}

# Lists available backups from CSV manifest, prompts user to select a backup ID
# and destination directory, and extracts files using tar -xzf.
restore_backup() {
    echo ""
    echo "-------------------------------------------------------"
    echo "                 RESTORE A PREVIOUS BACKUP             "
    echo "-------------------------------------------------------"

    # Verify manifest exists and has recorded entries
    local record_count
    record_count=$(awk 'END {print NR-1}' "$MANIFEST_FILE" 2>/dev/null)

    if [ -z "$record_count" ] || [ "$record_count" -le 0 ]; then
        echo "[INFO] No backups found in history manifest."
        log_activity "INFO" "Restore attempt failed: Manifest is empty."
        pause_screen
        return 0
    fi

    echo "Available Backups for Recovery:"
    echo "-----------------------------------------------------------------------------------------"
    # Print formatted list from CSV manifest (skipping line 1 header)
    awk -F',' 'NR>1 {printf " [%s] Date: %s | Source: %s | File: %s (%s)\n", $1, $2, $3, $4, $5}' "$MANIFEST_FILE"
    echo "-----------------------------------------------------------------------------------------"

    read -rp "Enter Backup ID to restore (Press 0 to cancel): " target_id

    if [[ "$target_id" == "0" || -z "$target_id" ]]; then
        echo "Restore cancelled."
        pause_screen
        return 0
    fi

    # Retrieve matching CSV record line
    local record_line
    record_line=$(awk -F',' -v id="$target_id" '$1 == id {print $0}' "$MANIFEST_FILE")

    if [ -z "$record_line" ]; then
        echo ""
        echo "[ERROR] Backup ID '$target_id' was not found in manifest!"
        log_activity "ERROR" "Restore failed: Invalid Backup ID '$target_id'."
        pause_screen
        return 1
    fi

    # Parse CSV fields
    local id ts source_dir archive_name archive_size archive_path
    IFS=',' read -r id ts source_dir archive_name archive_size archive_path <<< "$record_line"

    # Verify physical archive file existence
    if [ ! -f "$archive_path" ]; then
        echo ""
        echo "[ERROR] Physical archive file '$archive_path' is missing from storage!"
        log_activity "ERROR" "Restore failed: Missing archive file '$archive_path'."
        pause_screen
        return 1
    fi

    echo ""
    echo "Selected Backup Details:"
    echo "  Archive Name : $archive_name"
    echo "  Original Path: $source_dir"
    echo "  Archive Size : $archive_size"
    echo ""
    read -rp "Enter destination directory to restore files into (Press ENTER for original '$source_dir'): " raw_restore
    
    local restore_dest="$source_dir"
    if [ -n "$raw_restore" ]; then
        eval restore_dest="$raw_restore"
    fi

    # Create target restore destination directory if it doesn't exist
    mkdir -p "$restore_dest"

    echo ""
    echo "[INFO] Restoring files into '$restore_dest'..."

    # Execute tar extraction (-xzf) into target restore destination
    if tar -xzf "$archive_path" -C "$restore_dest"; then
        log_activity "RESTORE_SUCCESS" "Restored archive '$archive_name' to '$restore_dest'."

        echo ""
        echo "======================================================="
        echo "              RESTORE COMPLETED SUCCESSFULLY           "
        echo "======================================================="
        echo " Restored Archive : $archive_name"
        echo " Destination Path : $restore_dest"
        echo "======================================================="
    else
        echo ""
        echo "[ERROR] Tar extraction failed during file restoration!"
        log_activity "ERROR" "Tar extraction failed for '$archive_path' into '$restore_dest'."
    fi

    pause_screen
}

# Displays formatted table of all created backups recorded in CSV manifest.
view_history() {
    echo ""
    echo "========================================================================================"
    echo "                                BACKUP HISTORY MANIFEST                                 "
    echo "========================================================================================"

    local record_count
    record_count=$(awk 'END {print NR-1}' "$MANIFEST_FILE" 2>/dev/null)

    if [ -z "$record_count" ] || [ "$record_count" -le 0 ]; then
        echo " No backup records found in history manifest."
        echo "========================================================================================"
        pause_screen
        return 0
    fi

    # Render formatted ASCII table using awk
    awk -F',' '
    BEGIN {
        printf " %-4s | %-19s | %-25s | %-28s | %-8s\n", "ID", "Timestamp", "Source Directory", "Archive Name", "Size";
        print "----------------------------------------------------------------------------------------";
    }
    NR>1 {
        # Truncate long source paths for tabular display
        src = $3;
        if (length(src) > 25) src = "..." substr(src, length(src)-21);
        printf " %-4s | %-19s | %-25s | %-28s | %-8s\n", $1, $2, src, $4, $5;
    }
    ' "$MANIFEST_FILE"

    echo "========================================================================================"
    echo " Total Backup Records: $record_count"
    echo "========================================================================================"
    
    pause_screen
}

# Deletes a selected backup archive file from storage and removes its record from CSV manifest.
delete_backup() {
    echo ""
    echo "-------------------------------------------------------"
    echo "                 DELETE AN EXISTING BACKUP             "
    echo "-------------------------------------------------------"

    local record_count
    record_count=$(awk 'END {print NR-1}' "$MANIFEST_FILE" 2>/dev/null)

    if [ -z "$record_count" ] || [ "$record_count" -le 0 ]; then
        echo "[INFO] No backups available to delete."
        pause_screen
        return 0
    fi

    echo "Recorded Backups:"
    awk -F',' 'NR>1 {printf " [%s] Date: %s | File: %s (%s)\n", $1, $2, $4, $5}' "$MANIFEST_FILE"
    echo "-------------------------------------------------------"

    read -rp "Enter Backup ID to delete (Press 0 to cancel): " target_id

    if [[ "$target_id" == "0" || -z "$target_id" ]]; then
        echo "Deletion cancelled."
        pause_screen
        return 0
    fi

    # Retrieve record line matching target ID
    local record_line
    record_line=$(awk -F',' -v id="$target_id" '$1 == id {print $0}' "$MANIFEST_FILE")

    if [ -z "$record_line" ]; then
        echo ""
        echo "[ERROR] Backup ID '$target_id' not found in manifest!"
        log_activity "ERROR" "Delete failed: Invalid Backup ID '$target_id'."
        pause_screen
        return 1
    fi

    # Parse CSV record fields
    local id ts source_dir archive_name archive_size archive_path
    IFS=',' read -r id ts source_dir archive_name archive_size archive_path <<< "$record_line"

    echo ""
    echo "Warning: You are about to permanently delete Backup ID #$target_id ('$archive_name')."
    read -rp "Are you sure? (y/n): " confirm
    if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
        echo "Deletion cancelled."
        pause_screen
        return 0
    fi

    # Remove physical archive file if it exists
    if [ -f "$archive_path" ]; then
        rm -f "$archive_path"
        echo "[INFO] Physical archive file '$archive_name' removed."
    else
        echo "[WARN] Archive file '$archive_name' was already missing from storage disk."
    fi

    # Filter out deleted record line from CSV manifest
    local temp_manifest="${CONFIG_DIR}/manifest.tmp"
    awk -F',' -v id="$target_id" '$1 != id {print $0}' "$MANIFEST_FILE" > "$temp_manifest"
    mv "$temp_manifest" "$MANIFEST_FILE"

    log_activity "DELETE_SUCCESS" "Deleted backup ID #$target_id ('$archive_name')."

    echo ""
    echo "[SUCCESS] Backup ID #$target_id deleted successfully."
    pause_screen
}

# Sub-menu allowing users to view recorded activity log entries or clear the log.
manage_logs() {
    echo ""
    echo "======================================================="
    echo "            VIEW OR CLEAR ACTIVITY LOGS                "
    echo "======================================================="
    echo " 1. View Activity Log (backup_activity.log)"
    echo " 2. Clear Activity Log"
    echo " 0. Return to Main Menu"
    echo "======================================================="
    read -rp "Enter choice (0-2): " log_choice

    case "$log_choice" in
        1)
            echo ""
            echo "======================================================="
            echo "                SYSTEM ACTIVITY LOGS                   "
            echo "======================================================="
            if [ -f "$LOG_FILE" ] && [ -s "$LOG_FILE" ]; then
                cat "$LOG_FILE"
                local line_cnt
                line_cnt=$(wc -l < "$LOG_FILE" | tr -d ' ')
                echo "======================================================="
                echo " Total Log Entries: $line_cnt"
            else
                echo " Activity log is currently empty."
            fi
            echo "======================================================="
            pause_screen
            ;;
        2)
            echo ""
            read -rp "Are you sure you want to CLEAR all activity logs? (y/n): " confirm_clear
            if [[ "$confirm_clear" == "y" || "$confirm_clear" == "Y" ]]; then
                > "$LOG_FILE"
                log_activity "INFO" "Activity log file cleared by user."
                echo "[SUCCESS] Activity log cleared successfully."
            else
                echo "Log clearing cancelled."
            fi
            pause_screen
            ;;
        0)
            return 0
            ;;
        *)
            echo "[ERROR] Invalid selection."
            pause_screen
            ;;
    esac
}

# ==============================================================================
# MENU INTERFACE & MAIN LOOP
# ==============================================================================

display_menu() {
    clear
    echo "======================================================="
    echo "     LINUX FILE BACKUP & RECOVERY MANAGER (ARCH)       "
    echo "======================================================="
    echo " 1. Create a New Backup"
    echo " 2. Restore a Previous Backup"
    echo " 3. View Backup History & Manifest"
    echo " 4. Delete an Existing Backup"
    echo " 5. View or Clear Activity Logs"
    echo " 6. Check Available Disk Space"
    echo " 0. Exit Application"
    echo "======================================================="
    echo -n "Enter your choice (0-6): "
}

main() {
    init_environment

    while true; do
        display_menu
        read -r choice

        case "$choice" in
            1)
                create_backup
                ;;
            2)
                restore_backup
                ;;
            3)
                view_history
                ;;
            4)
                delete_backup
                ;;
            5)
                manage_logs
                ;;
            6)
                check_disk_space
                pause_screen
                ;;
            0)
                echo ""
                echo "Exiting Backup Manager. Goodbye!"
                log_activity "INFO" "Backup Manager system shutdown."
                exit 0
                ;;
            *)
                echo ""
                echo "[ERROR] Invalid choice! Please enter a number between 0 and 6."
                pause_screen
                ;;
        esac
    done
}

# Launch script main function
main "$@"
