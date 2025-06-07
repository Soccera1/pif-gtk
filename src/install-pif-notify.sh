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

if [ -z "$ORIGINAL_USER" ]; then
    echo "Error: Could not determine original user from PKEXEC_UID."
    exit 1
fi

# Reload systemd user units for the original user
systemctl --machine="$ORIGINAL_USER"@.host --user daemon-reload

# Enable and start the service and timer for the original user
systemctl --machine="$ORIGINAL_USER"@.host --user enable --now pif-notify.service
systemctl --machine="$ORIGINAL_USER"@.host --user enable --now pif-notify.timer

echo "PIF Song Rotation Service has been installed and enabled."
echo "The service will run daily to show you which songs to practice."