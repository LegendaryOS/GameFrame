#!/bin/bash

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "This script must be run with sudo for namespace isolation."
    exit 1
fi

# Check if command is provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <command> [args...]"
    exit 1
fi

# Set up cgroups for resource management
CGROUP_NAME="gameframe_$$"
mkdir -p /sys/fs/cgroup/cpu/$CGROUP_NAME
mkdir -p /sys/fs/cgroup/memory/$CGROUP_NAME
echo $$ > /sys/fs/cgroup/cpu/$CGROUP_NAME/tasks
echo $$ > /sys/fs/cgroup/memory/$CGROUP_NAME/tasks
echo 100000 > /sys/fs/cgroup/cpu/$CGROUP_NAME/cpu.cfs_quota_us
echo 4G > /sys/fs/cgroup/memory/$CGROUP_NAME/memory.limit_in_bytes

# Thermal monitoring (requires lm-sensors)
if command -v sensors >/dev/null 2>&1; then
    TEMP=$(sensors | grep -E 'Core 0|Package id 0' | awk '{print $3}' | cut -d '+' -f2 | cut -d '.' -f1)
    export GAMEFRAME_TEMP=$TEMP
else
    export GAMEFRAME_TEMP=70
fi

# Optimize namespace isolation
unshare -m -u -n -p -f --mount-proc -- /usr/local/bin/gameframe_launcher "$@"
