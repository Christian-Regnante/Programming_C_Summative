#!/bin/bash

# =====================================================================
# PROJECT 2: LINUX FILE BACKUP AND RECOVERY MANAGER
# =====================================================================

DEFAULT_SOURCE="./sample_data"
BACKUP_DIR="./backups"
RESTORE_DIR="./restored_files"
LOG_FILE="backup_activity.log"

# Ensure initial directories exist
mkdir -p "$BACKUP_DIR"
mkdir -p "$RESTORE_DIR"

# Function to log activity with timestamp
log_event() {
    local message="$1"
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $message" | tee -a "$LOG_FILE"
}

# Function to display available disk space before backup
show_disk_space() {
    echo ""
    echo "--- Available Disk Space ---"
    df -h . | awk 'NR==1 || NR==2 {print $0}'
}

# 1. CREATE BACKUP
create_backup() {
    show_disk_space

    echo ""
    read -p "Enter source directory to back up [Default: $DEFAULT_SOURCE]: " src_input
    local source_dir="${src_input:-$DEFAULT_SOURCE}"

    # Check if source directory exists
    if [ ! -d "$source_dir" ]; then
        if [ "$source_dir" = "$DEFAULT_SOURCE" ]; then
            mkdir -p "$DEFAULT_SOURCE"
            echo "Sample document data" > "$DEFAULT_SOURCE/file1.txt"
            echo "Sample code data" > "$DEFAULT_SOURCE/file2.c"
            echo "[INFO] Created sample directory '$DEFAULT_SOURCE'."
        else
            echo "[ERROR] Directory '$source_dir' does not exist!"
            log_event "ERROR: Failed backup. Missing source '$source_dir'."
            return 1
        fi
    fi

    local timestamp=$(date '+%Y%m%d_%H%M%S')
    local backup_file="$BACKUP_DIR/backup_$timestamp.tar.gz"

    echo "Compressing '$source_dir' into '$backup_file'..."
    tar -czf "$backup_file" "$source_dir" 2>/dev/null

    if [ $? -eq 0 ]; then
        echo "[SUCCESS] Backup created successfully: $backup_file"
        log_event "BACKUP: Created archive '$backup_file' from '$source_dir'."
    else
        echo "[ERROR] Backup failed during compression!"
        log_event "ERROR: Failed to compress '$source_dir'."
    fi
}

# 2. RESTORE BACKUP
restore_backup() {
    echo ""
    echo "--- Available Backup Archives ---"
    local files=("$BACKUP_DIR"/*.tar.gz)

    if [ ! -e "${files[0]}" ]; then
        echo "[INFO] No backup archives found to restore."
        return 1
    fi

    ls -1 "$BACKUP_DIR"/*.tar.gz
    echo ""
    read -p "Enter full path or filename of backup to restore: " target_file

    # Prepend BACKUP_DIR if user entered just filename
    if [ ! -f "$target_file" ] && [ -f "$BACKUP_DIR/$target_file" ]; then
        target_file="$BACKUP_DIR/$target_file"
    fi

    if [ ! -f "$target_file" ]; then
        echo "[ERROR] Selected backup file '$target_file' not found!"
        log_event "ERROR: Restore failed. Missing archive '$target_file'."
        return 1
    fi

    mkdir -p "$RESTORE_DIR"
    echo "Restoring '$target_file' into '$RESTORE_DIR'..."
    tar -xzf "$target_file" -C "$RESTORE_DIR" 2>/dev/null

    if [ $? -eq 0 ]; then
        echo "[SUCCESS] Files restored successfully into '$RESTORE_DIR'."
        log_event "RESTORE: Restored archive '$target_file' to '$RESTORE_DIR'."
    else
        echo "[ERROR] Restore operation failed!"
        log_event "ERROR: Failed to extract '$target_file'."
    fi
}

# 3. VIEW BACKUP HISTORY
view_backups() {
    echo ""
    echo "=== Backup History in '$BACKUP_DIR' ==="
    if [ -z "$(ls -A "$BACKUP_DIR" 2>/dev/null)" ]; then
        echo "No backup archives found."
    else
        ls -lh "$BACKUP_DIR"/*.tar.gz 2>/dev/null
    fi
}

# 4. DELETE AN EXISTING BACKUP
delete_backup() {
    view_backups
    echo ""
    read -p "Enter filename of backup to delete: " del_file

    if [ ! -f "$del_file" ] && [ -f "$BACKUP_DIR/$del_file" ]; then
        del_file="$BACKUP_DIR/$del_file"
    fi

    if [ -f "$del_file" ]; then
        rm -f "$del_file"
        echo "[SUCCESS] Deleted backup file '$del_file'."
        log_event "DELETE: Removed backup archive '$del_file'."
    else
        echo "[ERROR] Backup file '$del_file' not found!"
    fi
}

# 5. VIEW OR CLEAR ACTIVITY LOG
manage_log() {
    echo ""
    echo "--- Activity Log Options ---"
    echo "1. View Activity Log"
    echo "2. Clear Activity Log"
    read -p "Choose option [1-2]: " log_choice

    if [ "$log_choice" = "1" ]; then
        if [ -f "$LOG_FILE" ]; then
            echo ""
            echo "=== ACTIVITY LOG ==="
            cat "$LOG_FILE"
        else
            echo "[INFO] Log file is currently empty."
        fi
    elif [ "$log_choice" = "2" ]; then
        > "$LOG_FILE"
        echo "[SUCCESS] Activity log cleared."
        log_event "LOG: Activity log cleared by user."
    else
        echo "[ERROR] Invalid option."
    fi
}

# MAIN INTERACTIVE MENU LOOP
while true; do
    echo ""
    echo "=========================================="
    echo " LINUX FILE BACKUP & RECOVERY MANAGER     "
    echo "=========================================="
    echo "1. Create a backup"
    echo "2. Restore a backup"
    echo "3. View backup history"
    echo "4. Delete an existing backup"
    echo "5. View or clear activity log"
    echo "0. Exit program"
    echo "=========================================="
    read -p "Enter choice [0-5]: " choice

    case $choice in
        1) create_backup ;;
        2) restore_backup ;;
        3) view_backups ;;
        4) delete_backup ;;
        5) manage_log ;;
        0) echo "Exiting Backup Manager. Goodbye!"; exit 0 ;;
        *) echo "[ERROR] Invalid selection! Please enter a number between 0 and 5." ;;
    esac
done
