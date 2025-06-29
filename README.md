# pif

A simple song management tool with both CLI and GTK interfaces.

## 🚀 Features

- Add songs to your list
- Remove songs from your list
- View all songs
- Simple and intuitive interface (GTK version)
- Command-line interface for automation (CLI version)

## 🛠️ Building

### Prerequisites

- C compiler (GCC recommended)
- GTK+ 3.0 development files (for GTK interface)
- pkg-config
- libnotify-bin (for desktop notifications)
- polkitd (for privilege escalation via pkexec)

On Debian/Ubuntu systems, you can install these dependencies with:

```bash
sudo apt-get install build-essential libgtk-3-dev pkg-config libnotify-bin polkitd
```

### Compilation

To build both the CLI (`pif`) and GTK (`pif-gtk`) versions, run:

```bash
make
```

> **Note:** `pif-gtk` depends on `pif`, so you must keep the `pif` executable.

## 📦 Installation

To install both versions (typically to `/usr/local/bin/`), run:

```bash
sudo make install
```

## 🖥️ Usage

### CLI Version

To start the command-line interface, run:

```bash
pif
```

### GTK Version

Launch `pif-gtk` from your desktop's application launcher to start the graphical interface.

## 🖼️ Screenshot

![pif-gtk screenshot](https://github.com/user-attachments/assets/bc2bc5dd-75f1-4868-9f3d-675407968827)

## 📄 License

This project is licensed under the GNU Affero General Public License v3.0. See the [LICENSE](LICENSE) file for details.
