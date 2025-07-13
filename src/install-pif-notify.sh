#!/bin/bash
#
# This file is part of pif.
#
# pif is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# pif is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with pif.  If not, see <https://www.gnu.org/licenses/>.

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root"
    exit 1
fi

# Get the original user's username (who initiated pkexec)
# PKEXEC_UID is set by pkexec and holds the UID of the original user.
if [ -z "$PKEXEC_UID" ]; then
    echo "Error: PKEXEC_UID not set. This script must be run via pkexec (e.g., from a graphical application)."
    exit 1
fi

ORIGINAL_USER=$(getent passwd "$PKEXEC_UID" | cut -d: -f1)
ORIGINAL_USER_HOME=$(getent passwd "$PKEXEC_UID" | cut -d: -f6)

if [ -z "$ORIGINAL_USER" ] || [ -z "$ORIGINAL_USER_HOME" ]; then
    echo "Error: Could not determine original user or home directory from PKEXEC_UID."
    exit 1
fi

# Detect init system
if [ "$(ps -p 1 -o comm=)" = "systemd" ]; then
    echo "Systemd detected. Installing systemd user service and timer."

    # Reload systemd user units for the original user
    systemctl --machine="$ORIGINAL_USER"@.host --user daemon-reload

    # Enable and start the service and timer for the original user
    systemctl --machine="$ORIGINAL_USER"@.host --user enable --now pif-notify.service
    systemctl --machine="$ORIGINAL_USER"@.host --user enable --now pif-notify.timer

    echo "PIF Song Rotation Service has been installed and enabled for user $ORIGINAL_USER."
    echo "The service will run daily to show you which songs to practice."
else
    echo "Non-systemd init system detected. Setting up a cron job."

    if ! pgrep -x "cron" > /dev/null && ! pgrep -x "crond" > /dev/null; then
        echo "Warning: cron daemon is not running. The daily notification might not work."
        echo "Please make sure cron is installed and enabled."
    fi

    # The command from the .service file is:
    # /bin/sh -c "MESSAGE=$(/usr/local/bin/pif); /usr/bin/notify-send 'PIF Rotation' "$MESSAGE""
    # For cron, we need to handle environment variables for notify-send.
    CRON_COMMAND="export DISPLAY=:0; export XAUTHORITY=$ORIGINAL_USER_HOME/.Xauthority; MESSAGE=\$(/usr/local/bin/pif); /usr/bin/notify-send 'PIF Rotation' \"\$MESSAGE\""
    
    # Create the cron job file in /etc/cron.d
    echo "@daily $ORIGINAL_USER $CRON_COMMAND" > /etc/cron.d/pif-notify
    chmod 0644 /etc/cron.d/pif-notify

    echo "PIF Song Rotation cron job has been installed for user $ORIGINAL_USER."
    echo "The job will run daily to show you which songs to practice."
fi
