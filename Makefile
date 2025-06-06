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

CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c17 -O3 -flto
GTK_CFLAGS := $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS := $(shell pkg-config --libs gtk+-3.0)

PREFIX ?= /usr/local
BIN := pif
GTK_BIN := pif-gtk
SRC_DIR := src
OBJ_DIR := build
CLI_SRC := $(SRC_DIR)/pif.c
GTK_SRC := $(SRC_DIR)/pif-gtk.c
CLI_OBJ := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CLI_SRC))
GTK_OBJ := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(GTK_SRC))
CLI_DEP := $(CLI_OBJ:.o=.d)
GTK_DEP := $(GTK_OBJ:.o=.d)

all: $(BIN) $(GTK_BIN)

$(BIN): $(CLI_OBJ)
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

clean:
	@rm -rf $(OBJ_DIR) $(BIN) $(GTK_BIN)

install: all
	@echo "Installing $(BIN), $(GTK_BIN) and service files..."
	@mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	@install -m 755 $(BIN) "$(DESTDIR)$(PREFIX)/bin/$(BIN)"
	@install -m 755 $(GTK_BIN) "$(DESTDIR)$(PREFIX)/bin/$(GTK_BIN)"
	@install -m 755 install-pif-notify.sh "$(DESTDIR)$(PREFIX)/bin/install-pif-notify.sh"
	@mkdir -p "$(DESTDIR)/etc/systemd/system"
	@install -m 644 pif-notify.service "$(DESTDIR)/etc/systemd/system/pif-notify.service"
	@echo "Installation complete."

uninstall:
	@echo "Uninstalling $(BIN), $(GTK_BIN) and service files..."
	@rm -f "$(DESTDIR)$(PREFIX)/bin/$(BIN)"
	@rm -f "$(DESTDIR)$(PREFIX)/bin/$(GTK_BIN)"
	@rm -f "$(DESTDIR)$(PREFIX)/bin/install-pif-notify.sh"
	@rm -f "$(DESTDIR)/etc/systemd/system/pif-notify.service"
	@echo "Uninstallation complete."

.PHONY: all clean install uninstall
