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

# Compiler and flags
CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c17 -O3 -flto
GTK_CFLAGS := $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS := $(shell pkg-config --libs gtk+-3.0)

# Project structure
SRC_DIR := src
OBJ_DIR := build

# Binaries
CLI_BIN := pif
GTK_BIN := pif-gtk

# Source and object files
CLI_SRC := $(SRC_DIR)/pif.c
GTK_SRC := $(SRC_DIR)/pif-gtk.c
CLI_OBJ := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CLI_SRC))
GTK_OBJ := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(GTK_SRC))

# Dependency files
CLI_DEP := $(CLI_OBJ:.o=.d)
GTK_DEP := $(GTK_OBJ:.o=.d)

# Installation paths
PREFIX ?= /usr/local
BIN_DIR := $(DESTDIR)$(PREFIX)/bin
SYSTEMD_USER_DIR := $(DESTDIR)/usr/lib/systemd/user
APPLICATIONS_DIR := $(DESTDIR)/usr/share/applications
PIF_GTK_SHARE_DIR := $(DESTDIR)/usr/share/pif-gtk

.PHONY: all clean install uninstall

all: $(CLI_BIN) $(GTK_BIN)

# Build rules
$(CLI_BIN): $(CLI_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(GTK_BIN): $(GTK_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(GTK_LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -MMD -MP -c -o $@ $<

-include $(CLI_DEP)
-include $(GTK_DEP)

$(OBJ_DIR):
	@mkdir -p $@

# Housekeeping
clean:
	@echo "Cleaning up..."
	@rm -rf $(OBJ_DIR) $(CLI_BIN) $(GTK_BIN)

# Installation
install: all
	@echo "Installing $(CLI_BIN), $(GTK_BIN) and service files..."
	@mkdir -p "$(BIN_DIR)"
	@install -m 755 $(CLI_BIN) "$(BIN_DIR)/$(CLI_BIN)"
	@install -m 755 $(GTK_BIN) "$(BIN_DIR)/$(GTK_BIN)"
	@install -m 755 $(SRC_DIR)/install-pif-notify.sh "$(BIN_DIR)/install-pif-notify"
	@mkdir -p "$(SYSTEMD_USER_DIR)"
	@install -m 644 $(SRC_DIR)/pif-notify.service "$(SYSTEMD_USER_DIR)/pif-notify.service"
	@install -m 644 $(SRC_DIR)/pif-notify.timer "$(SYSTEMD_USER_DIR)/pif-notify.timer"
	@mkdir -p "$(APPLICATIONS_DIR)"
	@install -m 644 $(SRC_DIR)/pif-gtk.desktop "$(APPLICATIONS_DIR)/pif-gtk.desktop"
	@mkdir -p "$(PIF_GTK_SHARE_DIR)"
	@install -m 644 logo.png "$(PIF_GTK_SHARE_DIR)/logo.png"
	@echo "Installation complete."

# Uninstallation
uninstall:
	@echo "Uninstalling $(CLI_BIN), $(GTK_BIN) and service files..."
	@rm -f "$(BIN_DIR)/$(CLI_BIN)"
	@rm -f "$(BIN_DIR)/$(GTK_BIN)"
	@rm -f "$(BIN_DIR)/install-pif-notify"
	@rm -f "$(SYSTEMD_USER_DIR)/pif-notify.service"
	@rm -f "$(SYSTEMD_USER_DIR)/pif-notify.timer"
	@rm -f "$(APPLICATIONS_DIR)/pif-gtk.desktop"
	@rm -rf "$(PIF_GTK_SHARE_DIR)"
	@echo "Uninstallation complete."