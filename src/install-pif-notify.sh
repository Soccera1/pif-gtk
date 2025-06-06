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

# Enable the service for the current user
systemctl enable pif-notify@$SUDO_USER

# Start the service
systemctl start pif-notify@$SUDO_USER

echo "Service enabled and started for user $SUDO_USER"
echo "To check status: sudo systemctl status pif-notify@$SUDO_USER"