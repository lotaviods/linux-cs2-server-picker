#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

DAEMON_EXEC="$SCRIPT_DIR/CS2ServerPickerDaemon"
GUI_EXEC="$SCRIPT_DIR/CS2ServerPicker"
SOCKET_PATH="/tmp/CS2ServerPickerDaemon"

# Ensure Ctrl+C will stop the daemon started via pkexec
cleanup() {
    echo "Stopping daemon..."
    if [ -n "$PKEXEC_PID" ]; then
        kill -TERM "$PKEXEC_PID" 2>/dev/null || true
        sleep 1
    fi
    if command -v pgrep >/dev/null 2>&1; then
        pids=$(pgrep -f "$DAEMON_EXEC" || true)
        if [ -n "$pids" ]; then
            kill -TERM $pids 2>/dev/null || true
        fi
    fi
    if [ -e "$SOCKET_PATH" ]; then
        rm -f "$SOCKET_PATH" 2>/dev/null || true
    fi
    exit 130
}

trap cleanup INT TERM

if [ ! -e "$SOCKET_PATH" ]; then
        echo "Starting daemon..."

        # Start the daemon with pkexec in background and record its PID
        pkexec "$DAEMON_EXEC" &
        PKEXEC_PID=$!

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
fi

echo "Starting GUI..."
"$GUI_EXEC"
