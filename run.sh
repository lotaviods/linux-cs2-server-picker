#!/bin/bash

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

DAEMON_EXEC="$SCRIPT_DIR/CS2ServerPickerDaemon"
GUI_EXEC="$SCRIPT_DIR/CS2ServerPicker"
SOCKET_PATH="/tmp/CS2ServerPickerDaemon"

# Check if daemon is already running
if [ ! -e "$SOCKET_PATH" ]; then
    echo "Starting daemon..."
    pkexec "$DAEMON_EXEC" &
    # Wait for socket to appear
    timeout=10
    while [ ! -e "$SOCKET_PATH" ] && [ $timeout -gt 0 ]; do
        sleep 1
        timeout=$((timeout - 1))
    done
    if [ ! -e "$SOCKET_PATH" ]; then
        echo "Failed to start daemon"
        exit 1
    fi
    echo "Daemon started."
else
    echo "Daemon already running."
fi

echo "Starting GUI..."
"$GUI_EXEC"
